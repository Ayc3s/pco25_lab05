#include "person.h"
#include "bike.h"
#include <random>

BikingInterface* Person::binkingInterface = nullptr;
std::array<BikeStation*, NB_SITES_TOTAL> Person::stations{};


Person::Person(unsigned int _id) : id(_id), homeSite(0), currentSite(0) {
    static thread_local std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, Bike::nbBikeTypes - 1);
    preferredType = dist(rng);

    if (binkingInterface) {
        log(QString("Person %1, préfère type %2")
                .arg(id).arg(preferredType));
    }
}

void Person::setStations(const std::array<BikeStation*, NB_SITES_TOTAL>& _stations){
    Person::stations = _stations;
}

void Person::setInterface(BikingInterface* _binkingInterface) {
    binkingInterface = _binkingInterface;
}


void Person::run() {
    // TODO: implement this method
}

Bike* Person::takeBikeFromSite(unsigned int _site) {
    Bike * bike = nullptr;
    bike = stations[_site]->getBike(preferredType);

    if (binkingInterface) {
        binkingInterface->setBikes(_site, stations[_site]->nbBikes());
    }

    return bike;
}

void Person::depositBikeAtSite(unsigned int _site, Bike* _bike) {
    stations[_site]->putBike(_bike);

    if (binkingInterface) {
        binkingInterface->setBikes(_site, stations[_site]->nbBikes());
    }
}

void Person::bikeTo(unsigned int _dest, Bike* _bike) {
    unsigned int t = bikeTravelTime();
    if (binkingInterface) {
        binkingInterface->travel(id, currentSite, _dest, t);
    }
    currentSite = _dest;
}

void Person::walkTo(unsigned int _dest) {
    unsigned int t = walkTravelTime();
    if (binkingInterface) {
        binkingInterface->walk(id, currentSite, _dest, t);
    }
    currentSite = _dest;
}

unsigned int Person::chooseOtherSite(unsigned int _from) const {
    return randomSiteExcept(NBSITES, _from);
}

unsigned int Person::bikeTravelTime() const {
    return randomTravelTimeMs() + 1000;
}

unsigned int Person::walkTravelTime() const {
    return randomTravelTimeMs() + 2000;
}

void Person::log(const QString& msg) const {
    if (binkingInterface) {
        binkingInterface->consoleAppendText(id, msg);
    }
}

