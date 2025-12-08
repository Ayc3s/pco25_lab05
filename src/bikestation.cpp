#include "bikestation.h"

BikeStation::BikeStation(int _capacity) : capacity(_capacity) {
    bikes = std::vector<Bike*>();
}

BikeStation::~BikeStation() {
    ending();
}

void BikeStation::putBike(Bike* _bike){
    mutex.lock();
    bikes.push_back(_bike);
    if (_bike->bikeType == 0) {
        hasVTT.notifyOne();
    } else if (_bike->bikeType == 1) {
        hasRoad.notifyOne();
    } else if (_bike->bikeType == 2) {
        hasGravel.notifyOne();
    }
    mutex.unlock();
}

Bike* BikeStation::getBike(size_t _bikeType) {
    mutex.lock();
    while (true) {
        if (countBikesOfType(_bikeType)) {
            for (size_t i = 0; i < bikes.size(); ++i) {
                if (bikes[i]->bikeType == _bikeType) {
                    Bike* bike = bikes[i];
                    bikes.erase(bikes.begin() + i);
                    mutex.unlock();
                    return bike;
                }
            }
        }
        switch (_bikeType) {
            case 0:
                hasVTT.wait(&this->mutex);
                break;
            case 1:
                hasRoad.wait(&this->mutex);
                break;
            case 2:
                hasGravel.wait(&this->mutex);
                break;
            default:
                mutex.unlock();
                return nullptr;
        }
    }
}

std::vector<Bike*> BikeStation::addBikes(std::vector<Bike*> _bikesToAdd) {
    std::vector<Bike*> result;
    for (Bike* bike: _bikesToAdd) {
        putBike(bike);
    }
    return result;//todo jsp quoi retourner
}

std::vector<Bike*> BikeStation::getBikes(size_t _nbBikes) {
    std::vector<Bike*> result;
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
    ended = true;
   // TODO: implement this method
}
