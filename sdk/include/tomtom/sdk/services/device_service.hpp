#pragma once

#include <memory>
#include <string>
#include <tomtom/core/watch.hpp>

namespace tomtom::sdk::services
{
    /**
     * @brief Basic device statistics
     *
     * Contains information about the watch hardware and status.
     * Note: Some fields may be estimated or unavailable depending on the watch model.
     */
    struct DeviceStats
    {
        std::string serial_number; ///< Device serial number
        std::string product_name;  ///< Product name (e.g., "TomTom Runner Cardio")
        int battery_level;         ///< Battery level 0-100 (estimated if not available)
        // Add storage stats here later if needed
    };

    /**
     * @brief Service for device-level operations
     *
     * Handles operations that affect the watch itself rather than data:
     * - Time synchronization
     * - Device naming
     * - Battery and storage information
     *
     * Example usage:
     * ```cpp
     * auto device = manager.device();
     * device->syncTime();  // Sync watch time with PC
     * device->setWatchName("My Watch");
     * auto stats = device->getStats();
     * std::cout << "Battery: " << stats.battery_level << "%\n";
     * ```
     */
    class DeviceService
    {
    public:
        /**
         * @brief Constructor
         * @param watch Connected watch instance
         */
        explicit DeviceService(std::shared_ptr<core::Watch> watch);

        /**
         * @brief Synchronize watch time with system local time
         *
         * Sets the watch's internal clock to match the current system time.
         * This is critical for accurate activity timestamps. Should be called:
         * - After connecting to the watch
         * - After traveling across time zones
         * - If activities show incorrect timestamps
         *
         * @throws std::runtime_error if time sync fails
         */
        void syncTime();

        /**
         * @brief Set the display name of the watch
         *
         * Sets a friendly name for the watch (may be displayed in Bluetooth settings
         * or on the watch itself, depending on the model).
         *
         * @param name Display name (typically 1-32 characters)
         * @throws std::runtime_error if operation fails
         */
        void setWatchName(const std::string &name);

        /**
         * @brief Get basic device info (partially mocked/estimated for now)
         *
         * Returns device statistics. Note that some fields may be estimated
         * or unavailable depending on the watch model and firmware.
         *
         * @return Device statistics structure
         */
        DeviceStats getStats();

    private:
        std::shared_ptr<core::Watch> watch_;
    };
}
