#pragma once

#include <memory>
#include <string>
#include <cstdint>
#include <ctime>

#include "tomtom/protocol/runtime/packet_handler.hpp"
#include "tomtom/protocol/definition/protocol.hpp"

namespace tomtom::services
{
    /**
     * @brief Service for retrieving watch information.
     *
     * Provides methods to query various watch properties like time,
     * firmware version, BLE version, and product ID.
     */
    class WatchInfoService
    {
    public:
        explicit WatchInfoService(std::shared_ptr<protocol::runtime::PacketHandler> packet_handler);

        /**
         * @brief Get the current time from the watch.
         * @return The watch time as Unix timestamp.
         * @throws std::runtime_error if the operation fails.
         */
        std::time_t getTime();

        /**
         * @brief Get the firmware version string.
         * @return Firmware version as a string.
         * @throws std::runtime_error if the operation fails.
         */
        std::string getFirmwareVersion();

        /**
         * @brief Get the BLE firmware version string.
         * @return BLE version as a string.
         * @throws std::runtime_error if the operation fails.
         */
        std::string getBleVersion();

        /**
         * @brief Get the product ID.
         * @return Product ID value.
         * @throws std::runtime_error if the operation fails.
         */
        uint32_t getProductId();

    private:
        std::shared_ptr<protocol::runtime::PacketHandler> packet_handler_;
    };
}
