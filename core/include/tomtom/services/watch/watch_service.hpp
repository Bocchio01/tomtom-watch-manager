#pragma once

#include <memory>
#include <string>

#include "tomtom/protocol/runtime/packet_handler.hpp"
#include "tomtom/protocol/definition/protocol.hpp"

namespace tomtom::services::watch
{
    /**
     * @brief Service for watch operations.
     *
     * Provides methods to control the watch (format, reset)
     * and retrieve basic information (time, firmware version, product ID).
     */
    class WatchService
    {
    public:
        explicit WatchService(std::shared_ptr<protocol::runtime::PacketHandler> packet_handler);

        // ===================================================================
        // Control Operations
        // ===================================================================

        /**
         * @brief Format the watch storage.
         * @warning This operation erases all user data on the watch!
         * @throws std::runtime_error if the operation fails.
         */
        void formatWatch();

        /**
         * @brief Reset/reboot the watch device.
         * @warning The watch will restart and connection will be lost.
         * @throws std::runtime_error if the operation fails.
         */
        void resetDevice();

        /**
         * @brief Reset the GPS processor.
         * @return GPS reset confirmation message (if available).
         * @throws std::runtime_error if the operation fails.
         */
        std::string resetGpsProcessor();

        // ==================================================================
        // Info Retrieval Operations
        // ==================================================================

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
