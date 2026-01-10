#pragma once

#include <memory>
#include <string>

#include "tomtom/protocol/runtime/packet_handler.hpp"
#include "tomtom/protocol/definition/protocol.hpp"

namespace tomtom::services
{
    /**
     * @brief Service for watch control operations.
     *
     * Provides methods to control the watch such as formatting,
     * resetting the device, and resetting the GPS processor.
     */
    class WatchControlService
    {
    public:
        explicit WatchControlService(std::shared_ptr<protocol::runtime::PacketHandler> packet_handler);

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

    private:
        std::shared_ptr<protocol::runtime::PacketHandler> packet_handler_;
    };
}
