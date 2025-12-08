#include "bikestation.h"

BikeStation::BikeStation(int _capacity) : capacity(_capacity) {
    bikes = std::vector<Bike*>();
    bikes.reserve(_capacity);
}

BikeStation::~BikeStation() {
    ending();
}

void BikeStation::putBike(Bike* _bike){
    unsigned int ticket;
    mutex.lock();
    if (ended) {
        mutex.unlock();
        return;
    }
    ticket = putTicket++;
    while (nbBikes() >= nbSlots() || ticket != putNext) {
        if (ended) {
            mutex.unlock();
            return;
        }
        isntFull.wait(&this->mutex);
    }
    bikes.push_back(_bike);
    putNext++;
    if (_bike->bikeType == 0) {
        hasVTT.notifyAll();
    } else if (_bike->bikeType == 1) {
        hasRoad.notifyAll();
    } else if (_bike->bikeType == 2) {
        hasGravel.notifyAll();
    }
    mutex.unlock();
}

Bike* BikeStation::getBike(size_t _bikeType) {
    mutex.lock();

    if (ended) {
        mutex.unlock();
        return nullptr;
    }

    size_t ticket = getTickets[_bikeType]++;

    // Attendre que ce soit notre tour et qu'un vélo soit disponible
    while (ticket != getNext[_bikeType] || countBikesOfType(_bikeType) == 0) {
        if (ended) {
            mutex.unlock();
            return nullptr;
        }

        switch (_bikeType) {
            case 0: hasVTT.wait(&mutex); break;
            case 1: hasRoad.wait(&mutex); break;
            case 2: hasGravel.wait(&mutex); break;
            default:
                mutex.unlock();
                return nullptr;
        }
    }

    // prendre le vélo
    Bike* bike = nullptr;
    putNext++;
    for (size_t i = 0; i < bikes.size(); ++i) {
        if (bikes[i]->bikeType == _bikeType) {
            bike = bikes[i];
            bikes.erase(bikes.begin() + i);
            break;
        }
    }

    getNext[_bikeType]++;
    isntFull.notifyAll();  // Une place s'est libérée

    switch (_bikeType) {
        case 0: hasVTT.notifyAll(); break;
        case 1: hasRoad.notifyAll(); break;
        case 2: hasGravel.notifyAll(); break;
    }

    mutex.unlock();
    return bike;
}

std::vector<Bike*> BikeStation::addBikes(std::vector<Bike*> _bikesToAdd) {

    if (ended) {
        return _bikesToAdd;
    }

    size_t freeSlots = 0;
    if (nbSlots() > nbBikes()) freeSlots = nbSlots() - nbBikes();

    size_t toAdd = std::min(freeSlots, _bikesToAdd.size());
    for (size_t i = 0; i < toAdd; ++i) {
        Bike* b = _bikesToAdd.back();
        _bikesToAdd.pop_back();
        bikes.push_back(b);
        switch (b->bikeType) {
            case 0: hasVTT.notifyAll(); break;
            case 1: hasRoad.notifyAll(); break;
            case 2: hasGravel.notifyAll(); break;
        }
    }

    return _bikesToAdd;
}

std::vector<Bike*> BikeStation::getBikes(size_t _nbBikes) {
    std::vector<Bike*> result;
    if (ended) {
        return result;
    }
    for (size_t i = 0; i < _nbBikes && !bikes.empty(); ++i) {
        result.push_back(bikes.back());
        bikes.pop_back();
    }
    return result;
}

size_t BikeStation::countBikesOfType(size_t type) const {
    size_t result = 0;
    for (Bike* bike: bikes) {
        if (bike->bikeType == type) {
            result++;
        }
    }
    return result;
}

size_t BikeStation::nbBikes() {
    return bikes.size();
}

size_t BikeStation::nbSlots() {
    return capacity;
}

void BikeStation::ending() {
    mutex.lock();
    ended = true;

    hasVTT.notifyAll();
    hasRoad.notifyAll();
    hasGravel.notifyAll();
    isntFull.notifyAll();

    mutex.unlock();
}
