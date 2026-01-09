#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <ctime>
#include <map>
#include <functional>

#include <iostream>

#include "tomtom/transport/connection.hpp"
#include "tomtom/protocol/definition/protocol.hpp"
#include "tomtom/protocol/runtime/packet_handler.hpp"
#include "tomtom/protocol/services/file_service.hpp"
#include "tomtom/protocol/services/watch_info_service.hpp"
#include "tomtom/protocol/services/watch_control_service.hpp"
#include "tomtom/defines.hpp"

// Domain-specific file services
#include "tomtom/files/activity/activity.hpp"
#include "tomtom/files/preferences/preferences.hpp"
#include "tomtom/files/tracking/tracking.hpp"
#include "tomtom/files/routes/route.hpp"
#include "tomtom/files/manifest/manifest_service.hpp"

namespace tomtom
{
    /**
     * @brief Represents a TomTom watch device and provides access to its data.
     */
    class Watch
    {
    public:
        std::shared_ptr<transport::DeviceConnection> connection;

    private:
        std::shared_ptr<protocol::runtime::PacketHandler> packet_handler_;
        std::shared_ptr<protocol::services::FileService> file_service_;
        std::unique_ptr<protocol::services::WatchInfoService> info_service_;
        std::unique_ptr<protocol::services::WatchControlService> control_service_;

        // Domain-specific services
        std::unique_ptr<files::activity::ActivityService> activity_service_;
        std::unique_ptr<files::preferences::PreferencesService> preferences_service_;
        std::unique_ptr<files::tracking::TrackingService> tracking_service_;
        std::unique_ptr<files::routes::RouteService> route_service_;
        std::unique_ptr<files::manifest::ManifestService> manifest_service_;

    public:
        /**
         * @brief Constructs a new Watch instance.
         * @param connection Unique pointer to an established DeviceConnection.
         */
        explicit Watch(std::shared_ptr<transport::DeviceConnection> connection);

        /**
         * @brief Closes the connection to the watch and releases resources.
         */
        ~Watch();

        // Non-copyable and movable
        Watch(const Watch &) = delete;
        Watch &operator=(const Watch &) = delete;
        Watch(Watch &&) noexcept;
        Watch &operator=(Watch &&) noexcept;

        // Getters for watch properties
        uint16_t getVendorId() const { return static_cast<uint16_t>(connection->deviceInfo().vendor_id); }
        uint16_t getProductId() const { return static_cast<uint16_t>(connection->deviceInfo().product_id); }
        std::string_view getProductName() const { return connection->deviceInfo().product_name; }
        std::string_view getManufacturer() const { return connection->deviceInfo().manufacturer; }
        std::string_view getSerialNumber() const { return connection->deviceInfo().serial_number; }

        // Service accessors
        /**
         * @brief Get the FileService for file operations.
         * @return Reference to the FileService object.
         */
        protocol::services::FileService &files() { return *file_service_; }

        /**
         * @brief Get the WatchInfoService for information queries.
         * @return Reference to the WatchInfoService object.
         */
        protocol::services::WatchInfoService &info() { return *info_service_; }

        /**
         * @brief Get the WatchControlService for control operations.
         * @return Reference to the WatchControlService object.
         */
        protocol::services::WatchControlService &control() { return *control_service_; }

        // Domain-specific service accessors
        /**
         * @brief Access activity-related operations (workouts, GPS data, etc.)
         * @return Reference to ActivityService
         */
        files::activity::ActivityService &activities() { return *activity_service_; }

        /**
         * @brief Access watch preferences and settings
         * @return Reference to PreferencesService
         */
        files::preferences::PreferencesService &preferences() { return *preferences_service_; }

        /**
         * @brief Access daily activity tracking data (steps, calories)
         * @return Reference to TrackingService
         */
        files::tracking::TrackingService &tracking() { return *tracking_service_; }

        /**
         * @brief Access route/track management
         * @return Reference to RouteService
         */
        files::routes::RouteService &routes() { return *route_service_; }

        /**
         * @brief Access file manifest and storage information
         * @return Reference to ManifestService
         */
        files::manifest::ManifestService &manifest() { return *manifest_service_; }
    };

}