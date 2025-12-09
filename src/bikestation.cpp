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
    //check if ended
    if (ended) {
        mutex.unlock();
        return;
    }

    ticket = putTicket++; // get our ticket and increment for next
    while (nbBikes() >= nbSlots() || ticket != putNext) { // be sure that it's our turn and there is space
        if (ended) {
            mutex.unlock();
            return;
        }
        isntFull.wait(&this->mutex); // wait until there is space
    }
    bikes.push_back(_bike);// add bike
    putNext++; // pass the turn to the next person
    if (_bike->bikeType == 0) { // notify waiters
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

    if (ended) { //check if ended
        mutex.unlock();
        return nullptr;
    }

    size_t ticket = getTickets[_bikeType]++; // get our ticket and increment for next

    // wait our tourn AND a bike is available
    while (ticket != getNext[_bikeType] || countBikesOfType(_bikeType) == 0) {

        if (ended) {
            mutex.unlock();
            return nullptr;
        }

        switch (_bikeType) { // wait on the right condition
            case 0: hasVTT.wait(&mutex); break;
            case 1: hasRoad.wait(&mutex); break;
            case 2: hasGravel.wait(&mutex); break;
            default:
                mutex.unlock();
                return nullptr;
        }
    }

    Bike* bike = nullptr; // find and remove bike of the right type
    for (size_t i = 0; i < bikes.size(); ++i) {
        if (bikes[i]->bikeType == _bikeType) {
            bike = bikes[i];
            bikes.erase(bikes.begin() + i);
            break;
        }
    }

    getNext[_bikeType]++; // pass the turn to the next person
    isntFull.notifyAll(); // notify that people waiting for space can put their bike

    // notify the next waiting type of bike in case multiple bikes of same type were added
    switch (_bikeType) {
        case 0: hasVTT.notifyAll(); break;
        case 1: hasRoad.notifyAll(); break;
        case 2: hasGravel.notifyAll(); break;
    }

    mutex.unlock();
    return bike;
}

std::vector<Bike*> BikeStation::addBikes(std::vector<Bike*> _bikesToAdd) {
    // no mutex since Van should already have it

    if (ended) { //check if ended
        return _bikesToAdd;
    }
    // Count add bike per type for awake
    size_t addedByType[Bike::nbBikeTypes] = {0, 0, 0};

    // Add as many as fit, taking from the back of the provided vector.
    while (!_bikesToAdd.empty() && nbBikes() < nbSlots()) {
        Bike* b = _bikesToAdd.back();
        _bikesToAdd.pop_back();
        bikes.push_back(b);
        addedByType[b->bikeType]++;
    }

    // Notify waiters for each type added
    for (size_t t = 0; t < Bike::nbBikeTypes; ++t) {
        if (addedByType[t] > 0) {
            switch (t) {
                case 0: hasVTT.notifyAll(); break;
                case 1: hasRoad.notifyAll(); break;
                case 2: hasGravel.notifyAll(); break;
            }
        }
    }

    return _bikesToAdd;
}

std::vector<Bike*> BikeStation::getBikes(size_t _nbBikes) {
    // no mutex since Van should already have it
    std::vector<Bike*> result; // bikes to return
    if (ended) { //check if ended
        return result;
    }
    for (size_t i = 0; i < _nbBikes && !bikes.empty(); ++i) { // get as many as requested
        result.push_back(bikes.back());
        bikes.pop_back();
    }
    return result;
}

size_t BikeStation::countBikesOfType(size_t type) const {
    // no mutex since Van or Person should already have it
    size_t result = 0;
    for (Bike* bike: bikes) {
        if (bike->bikeType == type) {
            result++;
        }
    }
    return result;
}

size_t BikeStation::nbBikes() {
    // no mutex since Van or Person should already have it
    return bikes.size();
}

size_t BikeStation::nbSlots() {
    // no mutex since Van or Person should already have it
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