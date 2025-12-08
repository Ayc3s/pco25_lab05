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

    // lock depot because BikeStation methods don't lock internally in this repo
    BikeStation* depot = stations[DEPOT_ID];
    depot->mutex.lock();

    size_t D = depot->nbBikes();
    // a = current number of bikes in van
    size_t a = cargo.size();
    // load up to min(2, D) but don't exceed van capacity
    size_t want = std::min((size_t)2, D);
    size_t canLoad = (VAN_CAPACITY > a) ? (VAN_CAPACITY - a) : 0;
    size_t toLoad = std::min(want, canLoad);
    if (toLoad > 0) {
        std::vector<Bike*> bikes = depot->getBikes(toLoad);
        cargo.insert(cargo.end(), bikes.begin(), bikes.end());
        a = cargo.size();
    }

    if (binkingInterface) {
        binkingInterface->setBikes(DEPOT_ID, depot->nbBikes());
    }

    depot->mutex.unlock();
}

void Van::balanceSite(unsigned int _site)
{
    BikeStation* site = stations[_site];
    // lock the site while we inspect and operate (station methods assume external locking)
    site->mutex.lock();

    size_t B = site->nbSlots();
    size_t Vi = site->nbBikes();
    size_t a = cargo.size();

    if (Vi > B - 2) {
        // take surplus
        size_t surplus = Vi - (B - 2);
        size_t freeSpace = (VAN_CAPACITY > a) ? (VAN_CAPACITY - a) : 0;
        size_t c = std::min(surplus, freeSpace);
        if (c > 0) {
            std::vector<Bike*> taken = site->getBikes(c);
            cargo.insert(cargo.end(), taken.begin(), taken.end());
            a = cargo.size();
        }
    } else if (Vi < B - 2) {
        // need to deposit bikes
        size_t missing = (B - 2) - Vi;
        size_t c = std::min(missing, a); // number we can actually deposit
        size_t cdeposes = 0;
        std::vector<Bike*> bikesToDeposit;

        // First, for each type missing on the site, try to deposit one of that type
        for (size_t t = 0; t < Bike::nbBikeTypes && cdeposes < c; ++t) {
            if (site->countBikesOfType(t) == 0) {
                Bike* b = takeBikeFromCargo(t);
                if (b != nullptr) {
                    bikesToDeposit.push_back(b);
                    ++cdeposes;
                }
            }
        }

        // If still need to deposit, deposit arbitrary bikes from cargo
        while (cdeposes < c && !cargo.empty()) {
            bikesToDeposit.push_back(cargo.back());
            cargo.pop_back();
            ++cdeposes;
        }

        if (!bikesToDeposit.empty()) {
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