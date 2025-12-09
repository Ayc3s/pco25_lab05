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

    // wait our tourn AND a bike is available
    while (true) {
        // count bikes of requested type while holding the mutex
        size_t available = 0;
        for (Bike* b : bikes) {
            if (b->bikeType == _bikeType) ++available;
        }

        if (ticket == getNext[_bikeType] && available > 0) {
            break;
        }

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

    Bike* bike = nullptr;
    for (size_t i = 0; i < bikes.size(); ++i) {
        if (bikes[i]->bikeType == _bikeType) {
            bike = bikes[i];
            bikes.erase(bikes.begin() + i);
            break;
        }
    }

    getNext[_bikeType]++;
    isntFull.notifyAll();

    // notify the next waiting type of bike
    switch (_bikeType) {
        case 0: hasVTT.notifyAll(); break;
        case 1: hasRoad.notifyAll(); break;
        case 2: hasGravel.notifyAll(); break;
    }

    mutex.unlock();
    return bike;
}

std::vector<Bike*> BikeStation::addBikes(std::vector<Bike*> _bikesToAdd) {
    // Do not lock here: callers (Van) may hold the mutex. This function is safe
    // to call both with and without external locking; it avoids invalid indexing.
    if (ended) {
        return _bikesToAdd;
    }
    // Count add bike per type
    size_t addedByType[Bike::nbBikeTypes] = {0, 0, 0};

    // Add as many as fit, taking from the back of the provided vector.
    while (! _bikesToAdd.empty() && nbBikes() < nbSlots()) {
        Bike* b = _bikesToAdd.back();
        _bikesToAdd.pop_back();
        bikes.push_back(b);
        addedByType[b->bikeType]++;
    }

    for (size_t t = 0; t < Bike::nbBikeTypes; ++t) {
        if (addedByType[t] > 0) {
            switch (t) {
                case 0: hasVTT.notifyAll(); break;
                case 1: hasRoad.notifyAll(); break;
                case 2: hasGravel.notifyAll(); break;
            }
        }
    }

    if (!_bikesToAdd.empty() || bikes.size() < capacity) {
        isntFull.notifyAll();
    }

    return _bikesToAdd;
}

std::vector<Bike*> BikeStation::getBikes(size_t _nbBikes) {
    // Do not lock here: callers (Van) may hold the mutex.
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