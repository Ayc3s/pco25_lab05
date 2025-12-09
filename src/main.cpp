/******************************************************************************
  \file main.cpp
  \author Yann Thoma
  \date 05.05.2011

  Ce fichier propose un squelette pour l'application de gestion des vélos.
  Il est évident qu'il doit être grandement modifié pour respecter la donnée,
  mais vous y trouvez des exemples d'appels de fonctions de l'interface.
  ****************************************************************************/

#include <QApplication>
#include "bikinginterface.h"
#include <cstdlib>
#include <vector>

#include "person.h"
#include "van.h"
#include "bikestation.h"
#include "config.h"

#include <pcosynchro/pcothread.h>

std::array<BikeStation*, NB_SITES_TOTAL>* globalStations = nullptr;
std::vector<std::unique_ptr<PcoThread>>* globalThreads = nullptr;

// Should stop all threads and release waiting ones
void stopSimulation() {
    if (globalStations) {
        for (size_t i = 0; i < NB_SITES_TOTAL; ++i) {
            if ((*globalStations)[i]) {
                (*globalStations)[i]->ending();
            }
        }
    }

    if (globalThreads) {
        for (auto& thread : *globalThreads) {
            if (thread) {
                thread->requestStop();
            }
        }
    }
}


int main(int argc, char* argv[]) {
    // Checking constants
    if (BORNES < 4) {
        throw std::runtime_error("Each station should have at least 4 slots");
    }

    if (NBSITES < 2) {
        throw std::runtime_error("There should be at least two sites");
    }

    if (NB_BIKES < NBSITES * (BORNES - 2) + 3) {
        throw std::runtime_error("Not enough bikes to initialize the stations and the depot"); 
    }

    QApplication a(argc, argv);
    std::vector<std::unique_ptr<PcoThread>> threads;
    std::array<BikeStation*, NB_SITES_TOTAL> bikeStations;

    // Init of GUI
    BikingInterface::initialize(NBPEOPLE, NBSITES);
    auto* binkingInterface = new BikingInterface();

    // Create bikes stations with BORNES slots
    for (size_t s = 0; s < NBSITES; ++s) {
        bikeStations[s] = new BikeStation(BORNES);
    }

    // Create depot with NB_BIKES slots
    bikeStations[DEPOT_ID] = new BikeStation(NB_BIKES);

    // Create all bikes
    std::vector<Bike*> allBikes;
    allBikes.reserve(NB_BIKES);
    for (size_t i = 0; i < NB_BIKES; ++i) {
        auto* bike = new Bike;
        bike->bikeType = i % Bike::nbBikeTypes;
        allBikes.push_back(bike);
    }

    // Distribute bikes to stations
    size_t idx = 0;
    for (size_t s = 0; s < NBSITES; ++s) {
        std::vector<Bike*> chunk;
        for (size_t k = 0; k < BORNES - 2; ++k) {
            chunk.push_back(allBikes[idx++]);
        }

        bikeStations[s]->addBikes(chunk);
        binkingInterface->setInitBikes(s, chunk.size());
    }

    // Remaining bikes go to depot
    std::vector<Bike*> depotBikes;
    for (; idx < allBikes.size(); ++idx) {
        depotBikes.push_back(allBikes[idx]);
    }
    bikeStations[DEPOT_ID]->addBikes(depotBikes);
    binkingInterface->setInitBikes(DEPOT_ID, depotBikes.size());

    // Setting up pointer for interfaces
    Person::setInterface(binkingInterface);
    Van::setInterface(binkingInterface);

    // Setting up pointer for stations
    Person::setStations(bikeStations);
    Van::setStations(bikeStations);

    globalStations = &bikeStations;
    globalThreads = &threads;

    // Starting people and van threads
    for(size_t i = 0; i <= NBPEOPLE; ++i){
        if(i == 0) {
            threads.emplace_back(std::make_unique<PcoThread>(&Van::run, new Van(i)));
            continue;
        }

        threads.emplace_back(std::make_unique<PcoThread>(&Person::run, new Person(i)));
        binkingInterface->setInitPerson(0, i);
    }

    int ret = a.exec();

    for (auto& thread : threads) {
        thread->join();
    }

    return ret;
}

