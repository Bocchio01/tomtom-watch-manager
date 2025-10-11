#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <memory>

#include "tomtom/connection/usb_connection.hpp"

namespace tomtom
{

    /**
     * @brief Represents a TomTom watch device and provides access to its data.
     */
    class Watch
    {

    private:
        // Are these properties needed here or can be fetched on demand from connection?
        uint32_t product_id = 0;
        uint32_t firmware_version = 0;
        uint32_t ble_version = 0;
        std::string serial_number = "";

    public:
        std::unique_ptr<USBConnection> connection;

    public:
        /**
         * @brief Constructs a new Watch instance.
         * @param connection Unique pointer to an established USBConnection.
         */
        explicit Watch(std::unique_ptr<USBConnection> connection);

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
        uint32_t getProductId() const { return product_id; }
        uint32_t getFirmwareVersion() const { return firmware_version; }
        std::string getSerialNumber() const { return serial_number; }
    };

}