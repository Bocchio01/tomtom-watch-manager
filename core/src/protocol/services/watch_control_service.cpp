#include "tomtom/protocol/services/watch_control_service.hpp"

#include <spdlog/spdlog.h>
#include <stdexcept>

#include "tomtom/defines.hpp"

namespace tomtom::protocol::services
{
    WatchControlService::WatchControlService(std::shared_ptr<runtime::PacketHandler> packet_handler)
        : packet_handler_(std::move(packet_handler))
    {
        if (!packet_handler_)
        {
            throw std::invalid_argument("PacketHandler cannot be null");
        }
    }

    void WatchControlService::formatWatch()
    {
        spdlog::warn("Formatting watch - this will erase all user data!");

        definition::FormatWatchTx request;
        auto response = packet_handler_->transaction<definition::FormatWatchTx, definition::FormatWatchRx>(request);

        if (response.packet.payload.error != definition::ProtocolError::SUCCESS)
        {
            throw std::runtime_error("Failed to format watch (Error: " +
                                     std::to_string(static_cast<uint32_t>(response.packet.payload.error)) + ")");
        }

        spdlog::info("Watch formatted successfully");
    }

    void WatchControlService::resetDevice()
    {
        spdlog::info("Sending device reset command...");

        definition::ResetDeviceTx request;
        // Note: Device typically does not send a response as it immediately resets
        packet_handler_->send(request);

        spdlog::info("Reset command sent - watch will reboot");
    }

    std::string WatchControlService::resetGpsProcessor()
    {
        spdlog::info("Resetting GPS processor...");

        definition::ResetGpsTx request;
        auto response = packet_handler_->transaction<definition::ResetGpsTx, definition::ResetGpsRx>(request);

        std::string message(
            reinterpret_cast<const char *>(response.raw_payload_bytes.data()),
            response.raw_payload_bytes.size());

        spdlog::info("GPS processor reset complete: {}", message);
        return message;
    }
}
