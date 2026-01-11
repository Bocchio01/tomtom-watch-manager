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

// Services
#include "tomtom/services/files/files.hpp"
#include "tomtom/services/watch/watch_service.hpp"
#include "tomtom/services/preferences/preferences.hpp"
#include "tomtom/services/gps_quickfix/gps_quickfix.hpp"
#include "tomtom/services/activity/activity.hpp"

namespace tomtom
{
    /**
     * @brief Represents a TomTom watch device and provides access to its data.
     */
    class Watch
    {
    private:
        std::shared_ptr<transport::DeviceConnection> connection_;
        std::shared_ptr<protocol::runtime::PacketHandler> packet_handler_;

        std::shared_ptr<services::files::FileService> file_service_;
        std::shared_ptr<services::watch::WatchService> watch_service_;
        std::shared_ptr<services::activity::ActivityService> activity_service_;
        std::shared_ptr<services::preferences::PreferencesService> preferences_service_;
        std::shared_ptr<services::gps_quickfix::GpsQuickFixService> gps_quickfix_service_;

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
        uint16_t getVendorId() const { return static_cast<uint16_t>(connection_->deviceInfo().vendor_id); }
        uint16_t getProductId() const { return static_cast<uint16_t>(connection_->deviceInfo().product_id); }
        std::string_view getProductName() const { return connection_->deviceInfo().product_name; }
        std::string_view getManufacturer() const { return connection_->deviceInfo().manufacturer; }
        std::string_view getSerialNumber() const { return connection_->deviceInfo().serial_number; }

        /**
         * @brief Access low-level file operations
         * @return Reference to FileService
         */
        services::files::FileService &files() { return *file_service_; }

        /**
         * @brief Access watch control and info operations
         * @return Reference to WatchService
         */
        services::watch::WatchService &watch() { return *watch_service_; }

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
         * @brief Access GPS QuickFix update operations
         * @return Reference to GpsQuickFixService
         */
        services::gps_quickfix::GpsQuickFixService &gpsQuickFix() { return *gps_quickfix_service_; }
    };

}