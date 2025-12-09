#ifndef BIKESTATION_H
#define BIKESTATION_H

#include <vector>
#include <deque>
#include <array>
#include "bike.h"
#include <pcosynchro/pcoconditionvariable.h>
#include <pcosynchro/pcomutex.h>

/**
 * @brief Thread-safe bike station storing bikes by type with a limited capacity.
 *
 * Bikes are stored. Multiple threads can safely
 * put and get bikes using internal synchronization.
 */
class BikeStation
{
public:
    /**
     * @brief Mutex that secure the critical section
     */
    PcoMutex mutex;

    /**
     * @brief Default constructor (deleted or undefined in your code base).
     *
     * Declared but not defined here; if used, it must be implemented elsewhere.
     */
    BikeStation();

    /**
     * @brief Constructs a bike station with the given capacity.
     *
     * @param _capacity Maximum number of bikes that can be stored at this station.
     */
    BikeStation(int _capacity);

    /**
     * @brief Destructor.
     *
     * Calls ending() to wake up all waiting threads and signal termination.
     */
    ~BikeStation();

    /**
     * @brief Inserts a bike into the station.
     *
     * If the station is full, the calling thread blocks until a slot becomes
     * available or the station is marked as ending.
     *
     * @param _bike Pointer to the bike to put into the station. Must not be null.
     */
    void putBike(Bike *_bike);

    /**
     * @brief Retrieves one bike of the requested type from the station.
     *
     * If no bike of the requested type is available, the calling thread waits
     * until one is put or until the station is ending.
     *
     * @param _bikeType Requested bike type index (0..Bike::nbBikeTypes-1).
     * @return Pointer to the retrieved bike, or nullptr if the station is ending.
     */
    Bike* getBike(size_t _bikeType);

    /**
     * @brief Adds several bikes to the station at once.
     *
     * This function tries to insert as many bikes from @_bikesToAdd as possible
     * given the remaining capacity. Bikes that do not fit are returned.
     *
     * @param _bikesToAdd Vector of bike pointers to insert.
     * @return Vector containing the bikes that could not be inserted.
     */
    std::vector<Bike*> addBikes(std::vector<Bike*> _bikesToAdd);

    /**
     * @brief Retrieves up to a given number of bikes from the station.
     *
     * Bikes are taken in order of type and FIFO within each type until either
     * @_nbBikes bikes have been collected or the station runs out of bikes.
     *
     * @param _nbBikes Maximum number of bikes to retrieve.
     * @return Vector containing the bikes actually retrieved (may be fewer).
     */
    std::vector<Bike*> getBikes(size_t _nbBikes);

    /**
     * @brief Counts the bikes of a specific type currently stored.
     *
     * @param type Bike type index (0..Bike::nbBikeTypes-1).
     * @return Number of bikes of the given type in the station.
     */
    size_t countBikesOfType(size_t type) const;

    /**
     * @brief Returns the total number of bikes currently stored.
     *
     * @return Current number of bikes in the station.
     */
    size_t nbBikes();

    /**
     * @brief Returns the maximum number of bikes the station can contain.
     *
     * @return Station capacity in number of bikes.
     */
    size_t nbSlots();

    /**
     * @brief Signals that the station is ending and wakes up all waiting threads.
     *
     * Sets the @ref shouldEnd flag to true and notifies all condition variables
     * so that blocked threads can exit gracefully.
     */
    void ending();

private:

    /**
     * @brief condition of hasVTT - notify when there is a VTT available
     */
    PcoConditionVariable hasVTT;

    /**
     * @brief condition of hasRoad - notify when there is a Road available
     */
    PcoConditionVariable hasRoad;
    /**
    * @brief condition of hasRoad - notify when there is a hasGravel available
    */
    PcoConditionVariable hasGravel;

    /**
    * @brief condition if bikeStation is not full.
    */
    PcoConditionVariable isntFull;

    /** TICKET SYSTEM ------
    * @brief tab for each type of bike. Each index is the ticket system for the type of bike
    * Exemple: getTickets[0] -> we get the next size_t of the ticket system.
    */
    size_t getTickets[Bike::nbBikeTypes] = {0,0,0};

    /**
    * @brief get the next ticket
    */
    size_t getNext[Bike::nbBikeTypes] = {0,0,0};

    /**
    * @brief it is the the turn of a person, 'use' his ticket and put his bike
    */
    unsigned int putTicket = 0;

    /**
    * @brief get the next person turn.
    */
    unsigned int putNext = 0;
    /**
     * @brief Maximum number of bikes that can be stored in this station.
     */
    const size_t capacity;

    /**
     * @brief Bike storage of each bikestation
     */
    std::vector<Bike*> bikes;

    /**
     * @brief when emergency stop, check this condition to return the right way
     */
    bool ended = false;
};

#endif // BIKESTATION_H
