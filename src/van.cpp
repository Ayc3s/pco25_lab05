#include "van.h"

#include <pcosynchro/pcothread.h>

BikingInterface* Van::binkingInterface = nullptr;
std::array<BikeStation*, NB_SITES_TOTAL> Van::stations{};

Van::Van(unsigned int _id)
    : id(_id),
    currentSite(DEPOT_ID)
{}

void Van::run() {
    while (!PcoThread::thisThread()->stopRequested()) {
        loadAtDepot();
        for (unsigned int s = 0; s < NBSITES; ++s) {
            driveTo(s);
            balanceSite(s);
        }
        returnToDepot();
    }
    log("Van s'arrête proprement");
}

void Van::setInterface(BikingInterface* _binkingInterface){
    binkingInterface = _binkingInterface;
}

void Van::setStations(const std::array<BikeStation*, NB_SITES_TOTAL>& _stations) {
    stations = _stations;
}

void Van::log(const QString& msg) const {
    if (binkingInterface) {
        binkingInterface->consoleAppendText(0, msg);
    }
}

void Van::driveTo(unsigned int _dest) {
    if (currentSite == _dest)
        return;

    unsigned int travelTime = randomTravelTimeMs();
    if (binkingInterface) {
        binkingInterface->vanTravel(currentSite, _dest, travelTime);
    }

    currentSite = _dest;
}

void Van::loadAtDepot() {
    driveTo(DEPOT_ID);

    stations[DEPOT_ID]->mutex.lock();
    if (cargo.size() < VAN_CAPACITY - 1 && stations[DEPOT_ID]->nbBikes() >= 2) {
        size_t toLoad = std::min((size_t)2, stations[DEPOT_ID]->nbBikes());
        std::vector<Bike*> bikes = stations[DEPOT_ID]->getBikes(std::min ((VAN_CAPACITY - 1 - cargo.size()), toLoad));
        cargo.insert(cargo.end(), bikes.begin(), bikes.end());
    }
    if (binkingInterface) {
        binkingInterface->setBikes(DEPOT_ID, stations[DEPOT_ID]->nbBikes());
    }
    stations[DEPOT_ID]->mutex.unlock();
}



void Van::balanceSite(unsigned int _site)
{
    stations[_site]->mutex.lock();

    size_t targetCapacity = stations[_site]->nbSlots() - 2;
    size_t nbBikesOnSite = stations[_site]->nbBikes();

    if (nbBikesOnSite > targetCapacity) {
        size_t espaceDispo = VAN_CAPACITY - cargo.size();
        size_t velosAEnlever = std::min(nbBikesOnSite - targetCapacity, espaceDispo);

        if (velosAEnlever > 0) {
            std::vector<Bike*> bikesToTake = stations[_site]->getBikes(velosAEnlever);
            cargo.insert(cargo.end(), bikesToTake.begin(), bikesToTake.end());
        }
    } else if (nbBikesOnSite < targetCapacity) {
        size_t toDrop = std::min(targetCapacity - nbBikesOnSite, cargo.size());

        if (toDrop > 0) {
            std::vector<Bike*> bikesToAdd;
            size_t dropped = 0;

            for (size_t type = 0; type < Bike::nbBikeTypes; ++type) {
                if (dropped >= toDrop) break;

                if (stations[_site]->countBikesOfType(type) == 0) {
                    Bike* bike = takeBikeFromCargo(type);
                    if (bike != nullptr) {
                        bikesToAdd.push_back(bike);
                        dropped++;
                    }
                }
            }
            while (dropped < toDrop && !cargo.empty()) {
                bikesToAdd.push_back(cargo.back());
                cargo.pop_back();
                dropped++;
            }
            
            stations[_site]->addBikes(bikesToAdd);
        }
    }

    if (binkingInterface) {
        binkingInterface->setBikes(DEPOT_ID, stations[DEPOT_ID]->nbBikes()); // Keep somewhere for GUI
    }
    stations[_site]->mutex.unlock();
}

void Van::returnToDepot() {
    driveTo(DEPOT_ID);

    stations[DEPOT_ID]->mutex.lock();
    stations[DEPOT_ID]->addBikes(cargo);
    cargo.clear();

    if (binkingInterface) {
        binkingInterface->setBikes(DEPOT_ID, stations[DEPOT_ID]->nbBikes());
    }

    stations[DEPOT_ID]->mutex.unlock();
}

Bike* Van::takeBikeFromCargo(size_t type) {
    for (size_t i = 0; i < cargo.size(); ++i) {
        if (cargo[i]->bikeType == type) {
            Bike* bike = cargo[i];
            cargo[i] = cargo.back();
            cargo.pop_back();
            return bike;
        }
    }
    return nullptr;
}
size_t Van::countBikesOfType(size_t type) {
    size_t result = 0;
    for (Bike* bike: cargo) {
        if (bike->bikeType == type) {
            result++;
        }
    }
    return result;
}