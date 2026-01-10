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
#include "tomtom/defines.hpp"

// Domain-specific file services
#include "tomtom/services/file_service.hpp"
#include "tomtom/services/watch_info_service.hpp"
#include "tomtom/services/watch_control_service.hpp"
#include "tomtom/services/activity/activity.hpp"
#include "tomtom/services/preferences/preferences.hpp"
#include "tomtom/services/tracking/tracking.hpp"
#include "tomtom/services/routes/route.hpp"
#include "tomtom/services/manifest/manifest_service.hpp"
#include "tomtom/services/gps_quickfix/gps_quickfix_service.hpp"

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
        std::shared_ptr<services::FileService> file_service_;
        std::unique_ptr<services::WatchInfoService> info_service_;
        std::shared_ptr<services::WatchControlService> control_service_;

        // Domain-specific services
        std::unique_ptr<services::activity::ActivityService> activity_service_;
        std::unique_ptr<services::preferences::PreferencesService> preferences_service_;
        std::unique_ptr<services::tracking::TrackingService> tracking_service_;
        std::unique_ptr<services::routes::RouteService> route_service_;
        std::unique_ptr<services::manifest::ManifestService> manifest_service_;
        std::unique_ptr<services::gps_quickfix::GpsQuickFixService> gps_quickfix_service_;

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
        services::FileService &files() { return *file_service_; }

        /**
         * @brief Get the WatchInfoService for information queries.
         * @return Reference to the WatchInfoService object.
         */
        services::WatchInfoService &info() { return *info_service_; }

        /**
         * @brief Get the WatchControlService for control operations.
         * @return Reference to the WatchControlService object.
         */
        services::WatchControlService &control() { return *control_service_; }

        // Domain-specific service accessors
        /**
         * @brief Access activity-related operations (workouts, GPS data, etc.)
         * @return Reference to ActivityService
         */
        services::activity::ActivityService &activities() { return *activity_service_; }

        /**
         * @brief Access watch preferences and settings
         * @return Reference to PreferencesService
         */
        services::preferences::PreferencesService &preferences() { return *preferences_service_; }

        /**
         * @brief Access daily activity tracking data (steps, calories)
         * @return Reference to TrackingService
         */
        services::tracking::TrackingService &tracking() { return *tracking_service_; }

        /**
         * @brief Access route/track management
         * @return Reference to RouteService
         */
        services::routes::RouteService &routes() { return *route_service_; }

        /**
         * @brief Access file manifest and storage information
         * @return Reference to ManifestService
         */
        services::manifest::ManifestService &manifest() { return *manifest_service_; }

        /**
         * @brief Access GPS QuickFix update operations
         * @return Reference to GpsQuickFixService
         */
        services::gps_quickfix::GpsQuickFixService &gpsQuickFix() { return *gps_quickfix_service_; }
    };

}