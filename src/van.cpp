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

        PcoThread::usleep(2000000);// simulating the driver taking a break
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

    BikeStation* depot = stations[DEPOT_ID];
    depot->mutex.lock();

    size_t bikeInDepot = depot->nbBikes();
    size_t bikesInCargo = cargo.size();
    size_t maxBikesToLoad = std::min((size_t)2, bikeInDepot);
    size_t canLoad = (VAN_CAPACITY > bikesInCargo) ? (VAN_CAPACITY - bikesInCargo) : 0; // van should be empty but added for safety
    size_t toLoad = std::min(maxBikesToLoad, canLoad);

    if (toLoad > 0) {
        std::vector<Bike*> bikes = depot->getBikes(toLoad);
        cargo.insert(cargo.end(), bikes.begin(), bikes.end());
    }

    if (binkingInterface) {
        binkingInterface->setBikes(DEPOT_ID, depot->nbBikes());
    }

    depot->mutex.unlock();
}

void Van::balanceSite(unsigned int _site)
{
    BikeStation* site = stations[_site];
    // lock the station to make sure that state doesn't change while balancing
    site->mutex.lock();

    size_t slotsInSite = site->nbSlots();
    size_t bikesInSite = site->nbBikes();
    size_t bikesInCargo = cargo.size();

    if (bikesInSite > slotsInSite - 2) { // too many bikes
        size_t maxBikesToLoad = bikesInSite - (slotsInSite - 2);
        size_t canLoad = (VAN_CAPACITY > bikesInCargo) ? (VAN_CAPACITY - bikesInCargo) : 0;
        size_t toLoad = std::min(maxBikesToLoad, canLoad);
        if (toLoad > 0) {
            std::vector<Bike*> taken = site->getBikes(toLoad);
            cargo.insert(cargo.end(), taken.begin(), taken.end());
        }
    } else if (bikesInSite < slotsInSite - 2) { // not enought bikes
        size_t maxBikesToPut = (slotsInSite - 2) - bikesInSite;
        size_t canPut = std::min(maxBikesToPut, bikesInCargo);
        size_t deposed = 0;
        std::vector<Bike*> bikesToDeposit; // bikes to deposit at the station

       for (size_t t = 0; t < Bike::nbBikeTypes && deposed < canPut; ++t) { // deposit missing types first
            if (site->countBikesOfType(t) == 0) {
                Bike* b = takeBikeFromCargo(t);
                if (b != nullptr) {
                    bikesToDeposit.push_back(b);
                    ++deposed;
                }
            }
        }

        // deposit any type if needed
        while (deposed < canPut && !cargo.empty()) {
            bikesToDeposit.push_back(cargo.back());
            cargo.pop_back();
            ++deposed;
        }

        if (!bikesToDeposit.empty()) { // deposit bikes at the station
            site->addBikes(bikesToDeposit);
        }

    }


    if (binkingInterface) {
        // update GUI for this site
        binkingInterface->setBikes(_site, site->nbBikes());
    }

    site->mutex.unlock();
}

void Van::returnToDepot() {
    driveTo(DEPOT_ID);

    BikeStation* depot = stations[DEPOT_ID];
    depot->mutex.lock();

    if (!cargo.empty()) {
        depot->addBikes(cargo);
        cargo.clear();
    }

    if (binkingInterface) {
        binkingInterface->setBikes(DEPOT_ID, depot->nbBikes());
    }

    depot->mutex.unlock();
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