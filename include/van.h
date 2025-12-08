#ifndef VAN_H
#define VAN_H

#include <vector>
#include <array>
#include "config.h"
#include "bikestation.h"
#include "bikinginterface.h"
/**
 * @brief Simulates the van that rebalances bikes between sites and the depot.
 *
 * The van regularly:
 *  - loads bikes at the depot,
 *  - drives to each site to remove surplus bikes or drop missing ones,
 *  - returns to the depot with remaining bikes.
 */
class Van
{
public:

    /**
     * @brief Constructs a van with a given identifier.
     *
     * The van starts at the depot site.
     *
     * @param _id Identifier of the van (for logging and UI).
     */
    Van(unsigned int _id);

    /**
     * @brief Main loop of the van.
     *
     * Repeatedly:
     *  - loads bikes at the depot,
     *  - visits sites to balance bike counts,
     *  - returns to the depot.
     * This function is usually run in its own thread and never returns.
     */
    void run();

    /**
     * @brief Sets the user interface used to display van actions.
     *
     * @param _binkingInterface Pointer to the interface implementation.
     */
    static void setInterface(BikingInterface* _binkingInterface);

    /**
     * @brief Sets the array of bike stations used by the van.
     *
     * @param _stations Array of pointers to all stations (sites + depot).
     */
    static void setStations(const std::array<BikeStation*, NB_SITES_TOTAL>& _stations);

private:
    /**
     * @brief Writes a message about the van to the user interface console.
     *
     * @param msg Message to log.
     */
    void log(const QString& msg) const;

    /**
     * @brief Simulates driving the van from the current site to a destination site.
     *
     * Notifies the user interface and updates @ref currentSite.
     *
     * @param _dest Destination site index.
     */
    void driveTo(unsigned int _dest);

    /**
     * @brief Loads bikes from the depot into the van.
     *
     * Clears the current cargo, drives to the depot if necessary,
     * and takes a limited number of bikes from the depot station.
     */
    void loadAtDepot();

    /**
     * @brief Balances the number of bikes at a given site.
     *
     * If the site has more bikes than the target, the van takes some bikes.
     * If the site has fewer bikes than the target, the van drops bikes from its cargo.
     *
     * @param _s Index of the site to balance.
     */
    void balanceSite(unsigned int _s);

    /**
     * @brief Returns to the depot and drops all remaining bikes.
     *
     * Any bikes still in the cargo are added back to the depot station.
     */
    void returnToDepot();

    /**
     * @brief Takes a bike of a given type from the van cargo.
     *
     * Searches the cargo for a bike of @p type, removes it if found,
     * and returns it.
     *
     * @param type Desired bike type index.
     * @return Pointer to the bike if found, nullptr otherwise.
     */
    Bike* takeBikeFromCargo(size_t type);

    size_t countBikesOfType(size_t type);

    /**
     * @brief Identifier of the van.
     */
    unsigned int id;

    /**
     * @brief Site where the van is currently located.
     *
     * Initialized to @ref DEPOT_ID.
     */
    unsigned int currentSite;

    /**
     * @brief Collection of bikes currently loaded in the van.
     */
    std::vector<Bike*> cargo;

    /**
     * @brief User interface shared by all vans (may be null).
     */
    static BikingInterface* binkingInterface;

    /**
     * @brief Shared array of bike stations for all sites and the depot.
     */
    static std::array<BikeStation*, NB_SITES_TOTAL> stations;
};

#endif // VAN_H
