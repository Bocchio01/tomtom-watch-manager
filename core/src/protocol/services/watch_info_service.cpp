#include "tomtom/protocol/services/watch_info_service.hpp"

#include <spdlog/spdlog.h>
#include <stdexcept>

#include "tomtom/defines.hpp"

namespace tomtom::protocol::services
{
    WatchInfoService::WatchInfoService(std::shared_ptr<runtime::PacketHandler> packet_handler)
        : packet_handler_(std::move(packet_handler))
    {
        if (!packet_handler_)
        {
            throw std::invalid_argument("PacketHandler cannot be null");
        }
    }

    std::time_t WatchInfoService::getTime()
    {
        spdlog::debug("Requesting time from watch...");

        definition::GetWatchTimeTx request;
        auto response = packet_handler_->transaction<definition::GetWatchTimeTx, definition::GetWatchTimeRx>(request);

        uint32_t raw_time = response.packet.payload.time;
        std::time_t time = static_cast<std::time_t>(TT_BIGENDIAN(raw_time));

        spdlog::debug("Watch time: {}", std::asctime(std::gmtime(&time)));
        return time;
    }

    std::string WatchInfoService::getFirmwareVersion()
    {
        spdlog::debug("Requesting firmware version from watch...");

        definition::GetFirmwareVersionTx request;
        auto response = packet_handler_->transaction<definition::GetFirmwareVersionTx, definition::GetFirmwareVersionRx>(request);

        std::string version(
            reinterpret_cast<const char *>(response.raw_payload_bytes.data()),
            response.raw_payload_bytes.size());

        spdlog::debug("Firmware version: {}", version);
        return version;
    }

    std::string WatchInfoService::getBleVersion()
    {
        spdlog::debug("Requesting BLE version from watch...");

        definition::GetBleVersionTx request;
        auto response = packet_handler_->transaction<definition::GetBleVersionTx, definition::GetBleVersionRx>(request);

        std::string version(
            reinterpret_cast<const char *>(response.raw_payload_bytes.data()),
            response.raw_payload_bytes.size());

        spdlog::debug("BLE version: {}", version);
        return version;
    }

    uint32_t WatchInfoService::getProductId()
    {
        spdlog::debug("Requesting product ID from watch...");

        definition::GetProductIdTx request;
        auto response = packet_handler_->transaction<definition::GetProductIdTx, definition::GetProductIdRx>(request);

        uint32_t product_id = TT_BIGENDIAN(response.packet.payload.product_id);

        spdlog::debug("Product ID: 0x{:08X}", product_id);
        return product_id;
    }
}
