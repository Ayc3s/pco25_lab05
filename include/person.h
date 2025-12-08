#ifndef PERSON_H
#define PERSON_H

#include <array>
#include "config.h"
#include "bikestation.h"
#include "bikinginterface.h"

/**
 * @brief Simulates an person using the bike-sharing system.
 *
 * A Person repeatedly performs a loop:
 *  - takes a bike from the current site
 *  - rides to another site and drops the bike
 *  - walks to yet another site
 *  - takes a bike to go back home
 */
class Person
{
public:



    /**
     * @brief Constructs an person with a given identifier.
     *
     * The constructor randomly chooses a preferred bike type and initializes
     * the home and current site (here both start at 0).
     *
     * @param _id Unique identifier for this person.
     */
    Person(unsigned int _id);

    /**
     * @brief Main loop of the person.
     *
     * This function usually runs in its own thread and never returns.
     * It continuously simulates bike and walk trips between sites.
     */
    void run();

    /**
     * @brief Sets the user interface used to display actions and movements.
     *
     * @param _binkingInterface Pointer to the interface implementation.
     */
    static void setInterface(BikingInterface* _binkingInterface);

    /**
     * @brief Sets the array of bike stations used by all people.
     *
     * @param _stations Array of pointers to all stations (sites + depot).
     */
    static void setStations(const std::array<BikeStation*, NB_SITES_TOTAL>& _stations);

private:
    /**
     * @brief Chooses a random site different from the given one.
     *
     * @param _from Origin site index.
     * @return Index of a different site.
     */
    unsigned int chooseOtherSite(unsigned int _from) const;

    /**
     * @brief Computes a random travel time for a bike trip.
     *
     * @return Travel time in milliseconds.
     */
    unsigned int bikeTravelTime() const;

    /**
     * @brief Computes a random travel time for a walk.
     *
     * Typically longer than a bike trip.
     *
     * @return Travel time in milliseconds.
     */
    unsigned int walkTravelTime() const;

    /**
     * @brief Takes a bike of the preferred type from the given site.
     *
     * Updates the user interface with the new bike count at the site.
     *
     * @param _site Index of the site from which to take the bike.
     * @return Pointer to the taken bike (never null in normal operation).
     */
    Bike* takeBikeFromSite(unsigned int _site);

    /**
     * @brief Deposits a bike at the given site.
     *
     * Updates the user interface with the new bike count at the site.
     *
     * @param _site Index of the site where the bike is deposited.
     * @param _bike Pointer to the bike being deposited.
     */
    void depositBikeAtSite(unsigned int _site, Bike* _bike);

    /**
     * @brief Simulates riding a bike from the current site to a destination.
     *
     * Notifies the user interface of the trip and updates @ref currentSite.
     *
     * @param _dest Destination site index.
     * @param _bike Pointer to the bike used for this trip.
     */
    void bikeTo(unsigned int _dest, Bike* _bike);

    /**
     * @brief Simulates walking from the current site to a destination.
     *
     * Notifies the user interface of the walk and updates @ref currentSite.
     *
     * @param _dest Destination site index.
     */
    void walkTo(unsigned int _dest);

    /**
     * @brief Writes a message to the user interface console if available.
     *
     * @param _msg Message to log.
     */
    void log(const QString& _msg) const;

    /**
     * @brief Unique identifier of the person.
     */
    unsigned int id;

    /**
     * @brief Preferred bike type for this person.
     *
     * The person will always try to take bikes of this type.
     */
    size_t preferredType;

    /**
     * @brief Home site of the person.
     */
    unsigned int homeSite;

    /**
     * @brief Site where the person is currently located.
     */
    unsigned int currentSite;

    /**
     * @brief User interface shared by all people (may be null).
     */
    static BikingInterface* binkingInterface;

    /**
     * @brief Shared array of bike stations for all sites and the depot.
     */
    static std::array<BikeStation*, NB_SITES_TOTAL> stations;
    
};

#endif // PERSON_H
