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
#include "tomtom/interface/interface.hpp"
#include "tomtom/defines.hpp"

using namespace tomtom::connection;

namespace tomtom
{
    /**
     * @brief Represents a TomTom watch device and provides access to its data.
     */
    class Watch
    {
    public:
        std::shared_ptr<DeviceConnection> connection;

    public:
        /**
         * @brief Constructs a new Watch instance.
         * @param connection Unique pointer to an established DeviceConnection.
         */
        explicit Watch(std::shared_ptr<DeviceConnection> connection);

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

        void printTime()
        {
            using namespace tomtom::interface;

            // Create packet to request watch time
            GetWatchTimeTxPayload payload;
            Packet<GetWatchTimeTxPayload> packet(payload);

            // Send packet
            int bytes_written = connection->write(reinterpret_cast<const uint8_t *>(&packet), packet.size(), 1000);
            if (bytes_written != static_cast<int>(packet.size()))
            {
                throw std::runtime_error("Failed to write GetWatchTime packet");
            }

            // Read response
            Packet<GetWatchTimeRxPayload> response_packet;
            int bytes_read = connection->read(reinterpret_cast<uint8_t *>(&response_packet), response_packet.size(), 1000);
            if (bytes_read != static_cast<int>(response_packet.size()))
            {
                throw std::runtime_error("Failed to read GetWatchTime response");
            }

            // Extract time from response
            uint32_t watch_time = response_packet.payload.time;

            // Convert to time_t and print
            std::time_t t = static_cast<std::time_t>(TT_BIGENDIAN(watch_time));
            std::cout << "Watch Time: " << std::asctime(std::gmtime(&t));
        }
    };

}