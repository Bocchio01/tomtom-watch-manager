#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <ctime>

#include "tomtom/connection/connection.hpp"

using namespace tomtom::connection;

namespace tomtom
{
    enum class WatchError
    {
        NoError,
        UnableToSendPacket,
        UnableToReceivePacket,
        InvalidResponse,
        IncorrectResponseLength,
        OutOfSyncResponse,
        UnexpectedResponse,
        NoMatchingWatch,
        NotAWatch,
        UnableToOpenDevice,
        FileOpen,
        FileNotOpen,
        NoMoreFiles,
        VerifyError,
        ParseError,
        NoData,
        InvalidParameter,
    };

    struct WatchInfo
    {
        uint32_t product_id;
        uint32_t firmware_version;
        uint32_t ble_version;
        std::string serial_number;
        std::string manufacturer;
        std::string product_name;
    };

    /**
     * @brief Represents a TomTom watch device and provides access to its data.
     */
    class Watch
    {

    public:
        WatchInfo info;
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
        uint32_t getProductId() const { return info.product_id; }
        std::string getSerialNumber() const { return info.serial_number; }
        std::string getManufacturer() const { return info.manufacturer; }
        std::string getProductName() const { return info.product_name; }
    };

}