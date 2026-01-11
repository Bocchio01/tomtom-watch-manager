#include <stdexcept>
#include <spdlog/spdlog.h>

#include "tomtom/defines.hpp"
#include "tomtom/services/watch/watch_service.hpp"

namespace tomtom::services::watch
{
    WatchService::WatchService(std::shared_ptr<protocol::runtime::PacketHandler> packet_handler)
        : packet_handler_(std::move(packet_handler))
    {
        if (!packet_handler_)
        {
            throw std::invalid_argument("PacketHandler cannot be null");
        }
    }

    void WatchService::formatWatch()
    {
        spdlog::warn("Formatting watch - this will erase all user data!");

        protocol::definition::FormatWatchTx request;
        auto response = packet_handler_->transaction<protocol::definition::FormatWatchTx, protocol::definition::FormatWatchRx>(request);

        if (response.packet.payload.error != protocol::definition::ProtocolError::SUCCESS)
        {
            throw std::runtime_error("Failed to format watch (Error: " +
                                     std::to_string(static_cast<uint32_t>(response.packet.payload.error)) + ")");
        }

        spdlog::info("Watch formatted successfully");
    }

    void WatchService::resetDevice()
    {
        spdlog::info("Sending device reset command...");

        protocol::definition::ResetDeviceTx request;
        // Note: Device typically does not send a response as it immediately resets
        packet_handler_->send(request);

        spdlog::info("Reset command sent - watch will reboot");
    }

    std::string WatchService::resetGpsProcessor()
    {
        spdlog::info("Resetting GPS processor...");

        protocol::definition::ResetGpsTx request;
        auto response = packet_handler_->transaction<protocol::definition::ResetGpsTx, protocol::definition::ResetGpsRx>(request);

        std::string message(
            reinterpret_cast<const char *>(response.raw_payload_bytes.data()),
            response.raw_payload_bytes.size());

        spdlog::info("GPS processor reset complete: {}", message);
        return message;
    }

    std::time_t WatchService::getTime()
    {
        spdlog::debug("Requesting time from watch...");

        protocol::definition::GetWatchTimeTx request;
        auto response = packet_handler_->transaction<protocol::definition::GetWatchTimeTx, protocol::definition::GetWatchTimeRx>(request);

        uint32_t raw_time = response.packet.payload.time;
        std::time_t time = static_cast<std::time_t>(TT_BIGENDIAN(raw_time));

        spdlog::debug("Watch time: {}", std::asctime(std::gmtime(&time)));
        return time;
    }

    std::string WatchService::getFirmwareVersion()
    {
        spdlog::debug("Requesting firmware version from watch...");

        protocol::definition::GetFirmwareVersionTx request;
        auto response = packet_handler_->transaction<protocol::definition::GetFirmwareVersionTx, protocol::definition::GetFirmwareVersionRx>(request);

        std::string version(
            reinterpret_cast<const char *>(response.raw_payload_bytes.data()),
            response.raw_payload_bytes.size());

        spdlog::debug("Firmware version: {}", version);
        return version;
    }

    std::string WatchService::getBleVersion()
    {
        spdlog::debug("Requesting BLE version from watch...");

        protocol::definition::GetBleVersionTx request;
        auto response = packet_handler_->transaction<protocol::definition::GetBleVersionTx, protocol::definition::GetBleVersionRx>(request);

        std::string version(
            reinterpret_cast<const char *>(response.raw_payload_bytes.data()),
            response.raw_payload_bytes.size());

        spdlog::debug("BLE version: {}", version);
        return version;
    }

    uint32_t WatchService::getProductId()
    {
        spdlog::debug("Requesting product ID from watch...");

        protocol::definition::GetProductIdTx request;
        auto response = packet_handler_->transaction<protocol::definition::GetProductIdTx, protocol::definition::GetProductIdRx>(request);

        uint32_t product_id = TT_BIGENDIAN(response.packet.payload.product_id);

        spdlog::debug("Product ID: 0x{:08X}", product_id);
        return product_id;
    }
}
