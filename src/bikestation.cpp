#include "bikestation.h"

BikeStation::BikeStation(int _capacity) : capacity(_capacity) {}

BikeStation::~BikeStation() {
    ending();
}

void BikeStation::putBike(Bike* _bike){
    // TODO: implement this method


    // enter the monitor
    mutex.lock();

    // waiting for a spot to be free
    while (nbBikes() >= capacity) {
        notFull.wait(&mutex);
    }

    // we add the bike at the right type of bike
    bikes.push_back(_bike);

    // we notify one thread that at least one bike is available
    notEmpty.notifyOne();

    // we get out of the monitor
    mutex.unlock();
}

// todo : can be called by getBike, only with mutex, dangerous ?
bool BikeStation::hasBikeType(size_t _bikeType) {
    if (bikes.empty()) {
        return false;
    }

    for (size_t i = 0 ; i < bikes.size() ; i++) {
        if (bikes[i]->bikeType == _bikeType) {
            return true;
        }
    }
    return false;
}

Bike* BikeStation::getBike(size_t _bikeType) {
    // TODO: implement this method

    // enter the monitor
    mutex.lock();

    // while there is no right bike type
    while (!hasBikeType(_bikeType)) {
        notEmpty.wait(&mutex);
    }

    Bike* result = nullptr;

    for (size_t i = 0 ; i < bikes.size() ; i++) {
        if (bikes[i]->bikeType == _bikeType) {
            result = bikes[i];
            bikes[i] = bikes.back();
            bikes.pop_back();
            break;
        }
    }

    if (result != nullptr) {
        notFull.notifyOne();
    }

    mutex.unlock();
    return result;
    
}

std::vector<Bike*> BikeStation::addBikes(std::vector<Bike*> _bikesToAdd) {
    std::vector<Bike*> result; // Can be removed, it's just to avoid a compiler warning
    // TODO: implement this method
    return result;
}

std::vector<Bike*> BikeStation::getBikes(size_t _nbBikes) {
    std::vector<Bike*> result; // Can be removed, it's just to avoid a compiler warning
    // TODO: implement this method
    return result;
}

size_t BikeStation::countBikesOfType(size_t type) const {
    // TODO: implement this method
    return 0;
}

size_t BikeStation::nbBikes() {
    // TODO: implement this method
    return 0;
}

size_t BikeStation::nbSlots() {
    return capacity;
}

void BikeStation::ending() {
   // TODO: implement this method
}
