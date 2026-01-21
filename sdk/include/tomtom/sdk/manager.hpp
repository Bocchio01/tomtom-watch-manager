#pragma once

#include <memory>
#include <vector>
#include <string>

#include <tomtom/core/transport/device_info.hpp>
#include <tomtom/core/watch.hpp>

#include "tomtom/sdk/services/activity_service.hpp"
#include "tomtom/sdk/services/preferences_service.hpp"
#include "tomtom/sdk/services/gps_service.hpp"
#include "tomtom/sdk/services/device_service.hpp"
#include "tomtom/sdk/store/local_store.hpp"

namespace tomtom::sdk
{
    /**
     * @brief Main SDK facade for TomTom Watch Manager
     *
     * This is the primary entry point for the SDK. It provides:
     * - Watch discovery and connection
     * - Access to all high-level services
     * - Simplified API for common operations
     *
     * Example usage:
     * ```cpp
     * Manager manager;
     * auto watch = manager.connectToWatch();
     * if (watch) {
     *     auto activities = manager.activity()->listActivities();
     *     auto activity = manager.activity()->downloadActivity(activities[0]);
     *     manager.exportActivity(*activity, "gpx", "output.gpx");
     * }
     * ```
     */
    class Manager
    {
    public:
        Manager();
        ~Manager();

        // ====================================================================
        // Watch Discovery and Connection
        // ====================================================================

        /**
         * @brief Detect all connected TomTom watches
         *
         * Scans USB devices for connected watches without establishing
         * full connections.
         *
         * @return Vector of detected watch information
         */
        std::vector<core::transport::DeviceInfo> detectWatches();

        /**
         * @brief Connect to the first available watch
         *
         * @return Shared pointer to Watch, or nullptr if none found
         */
        std::shared_ptr<core::Watch> connectToWatch();

        /**
         * @brief Connect to a specific watch by index
         *
         * @param index Index from detectWatches() results (0-based)
         * @return Shared pointer to Watch, or nullptr if invalid index
         */
        std::shared_ptr<core::Watch> connectToWatch(size_t index);

        /**
         * @brief Connect to a specific watch by serial number
         *
         * @param serial Serial number of the watch
         * @return Shared pointer to Watch, or nullptr if not found
         */
        std::shared_ptr<core::Watch> connectToWatch(const std::string &serial);

        /**
         * @brief Get current watch count
         */
        size_t getWatchCount() const;

        /**
         * @brief Check if any watches are connected
         */
        bool hasWatches() const;

        /**
         * @brief Get currently connected watch
         */
        std::shared_ptr<core::Watch> getConnectedWatch() const { return connectedWatch_; }

        /**
         * @brief Disconnect from current watch
         */
        void disconnect();

        // ====================================================================
        // Service Access
        // ====================================================================

        /**
         * @brief Access device service (info, time, battery, etc.)
         * @throws std::runtime_error if no watch is connected
         */
        services::DeviceService *device();

        /**
         * @brief Access activity service (download, list, delete activities)
         * @throws std::runtime_error if no watch is connected
         */
        services::ActivityService *activity();

        /**
         * @brief Access GPS service (QuickFix updates)
         * @throws std::runtime_error if no watch is connected
         */
        services::GpsService *gps();

        /**
         * @brief Access preferences service (settings, user profile)
         * @throws std::runtime_error if no watch is connected
         */
        services::PreferencesService *preferences();

        /**
         * @brief Access the local storage system
         */
        store::LocalStore &store();

        // /**
        //  * @brief Access race service (manage races)
        //  * @throws std::runtime_error if no watch is connected
        //  */
        // services::RaceService *race();

        // /**
        //  * @brief Access route service (manage routes)
        //  * @throws std::runtime_error if no watch is connected
        //  */
        // services::RouteService *route();

        // /**
        //  * @brief Access tracking service (daily stats, HR, sleep)
        //  * @throws std::runtime_error if no watch is connected
        //  */
        // services::TrackingService *tracking();

        // /**
        //  * @brief Access workout service (manage workouts)
        //  * @throws std::runtime_error if no watch is connected
        //  */
        // services::WorkoutService *workout();

        // /**
        //  * @brief Access sync service (full sync orchestration)
        //  * @throws std::runtime_error if no watch is connected
        //  */
        // services::SyncService *sync();

        // ====================================================================
        // Convenience Methods
        // ====================================================================

        /**
         * @brief Perform a quick sync (GPS + time)
         * @throws std::runtime_error if no watch is connected
         */
        void quickSync();

        /**
         * @brief Perform a full sync (download activities, update GPS, sync time)
         * @param downloadPath Directory to save downloaded files
         * @param progress Optional progress callback
         * @throws std::runtime_error if no watch is connected
         */
        void fullSync(
            const std::string &downloadPath,
            std::function<void(const std::string &stage, float progress)> progressCallback = nullptr);

    private:
        std::vector<core::transport::DeviceInfo> cachedDevices_;
        std::shared_ptr<core::Watch> connectedWatch_;

        // Services (created on-demand)
        std::unique_ptr<services::DeviceService> deviceService_;
        std::unique_ptr<services::ActivityService> activityService_;
        std::unique_ptr<services::GpsService> gpsService_;
        std::shared_ptr<services::PreferencesService> preferencesService_;
        // std::unique_ptr<services::RaceService> raceService_;
        // std::unique_ptr<services::RouteService> routeService_;
        // std::unique_ptr<services::TrackingService> trackingService_;
        // std::unique_ptr<services::WorkoutService> workoutService_;
        // std::unique_ptr<services::SyncService> syncService_;

        // Store
        std::shared_ptr<store::LocalStore> localStore_;

        void refreshDeviceCache();
    };

}
