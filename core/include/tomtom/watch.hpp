#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <ctime>
#include <map>
#include <functional>

#include <iostream>

#include "tomtom/connection/connection.hpp"
#include "tomtom/interface/codec/packet_handler.hpp"
#include "tomtom/defines.hpp"

namespace tomtom
{
    /**
     * @brief Represents a TomTom watch device and provides access to its data.
     */
    class Watch
    {
    public:
        std::shared_ptr<connection::DeviceConnection> connection;

    private:
        std::unique_ptr<interface::codec::PacketHandler> packet_handler_;

    public:
        /**
         * @brief Constructs a new Watch instance.
         * @param connection Unique pointer to an established DeviceConnection.
         */
        explicit Watch(std::shared_ptr<connection::DeviceConnection> connection);

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

        /**
         * @brief Retrieves the current time from the watch.
         * @return std::time_t The watch time.
         */
        std::time_t getTime();
    };

}