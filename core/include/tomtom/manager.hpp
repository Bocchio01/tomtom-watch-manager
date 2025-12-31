#pragma once

#include <memory>
#include <vector>
#include <optional>
#include "tomtom/watch.hpp"
#include "tomtom/connection/device_info.hpp"

namespace tomtom
{
    /**
     * @brief Manager class for handling TomTom watch connections and operations.
     *
     * This class provides high-level functionality for discovering, connecting to,
     * and managing TomTom watches.
     */
    class Manager
    {
    public:
        Manager();
        ~Manager();

        /**
         * @brief Detects all connected TomTom watches.
         *
         * Scans USB devices and returns information about all connected TomTom watches
         * without establishing full connections.
         *
         * @return A vector of WatchInfo structures containing basic information about each watch.
         */
        std::vector<WatchInfo> detectWatches();

        /**
         * @brief Connects to the first available TomTom watch.
         *
         * Attempts to connect to the first detected TomTom watch. If multiple watches
         * are connected, use connectToWatch(index) to specify which one.
         *
         * @return A shared pointer to the connected Watch, or nullptr if no watch is found.
         */
        std::shared_ptr<Watch> connectToWatch();

        /**
         * @brief Connects to a specific TomTom watch by index.
         *
         * @param index The index of the watch to connect to (0-based, from detectWatches results).
         * @return A shared pointer to the connected Watch, or nullptr if index is invalid.
         */
        std::shared_ptr<Watch> connectToWatch(size_t index);

        /**
         * @brief Connects to a specific TomTom watch by serial number.
         *
         * @param serial The serial number of the watch to connect to.
         * @return A shared pointer to the connected Watch, or nullptr if not found.
         */
        std::shared_ptr<Watch> connectToWatch(const std::string &serial);

        /**
         * @brief Enumerates all connected TomTom watches with full initialization.
         *
         * Creates Watch instances for all connected devices. This establishes
         * full connections and initializes each watch.
         *
         * @return A vector of shared pointers to Watch instances.
         */
        static std::vector<std::shared_ptr<Watch>> enumerate();

        /**
         * @brief Gets the number of connected watches.
         *
         * @return The number of TomTom watches currently connected.
         */
        size_t getWatchCount() const;

        /**
         * @brief Checks if any watches are connected.
         *
         * @return True if at least one watch is connected, false otherwise.
         */
        bool hasWatches() const;

    private:
        std::vector<DeviceInfo> cachedDevices_;

        void refreshDeviceCache();
    };

} // namespace tomtom