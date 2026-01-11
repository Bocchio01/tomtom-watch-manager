#include <stdexcept>

#include "tomtom/core/utils/endianness.hpp"
#include "tomtom/core/commands/kernel_operations.hpp"

namespace tomtom::core::commands
{
    KernelOperations::KernelOperations(std::shared_ptr<runtime::PacketHandler> packet_handler)
        : packet_handler_(std::move(packet_handler))
    {
        if (!packet_handler_)
        {
            throw std::invalid_argument("PacketHandler cannot be null");
        }
    }

    void KernelOperations::formatWatch()
    {
        spdlog::warn("Formatting watch - this will erase all user data!");

        definitions::FormatWatchTx request;
        auto response = packet_handler_->transaction<definitions::FormatWatchTx, definitions::FormatWatchRx>(request);

        if (response.packet.payload.error != definitions::ProtocolError::SUCCESS)
        {
            throw std::runtime_error("Failed to format watch (Error: " +
                                     std::to_string(static_cast<uint32_t>(response.packet.payload.error)) + ")");
        }

        spdlog::info("Watch formatted successfully");
    }

    void KernelOperations::resetDevice()
    {
        spdlog::info("Sending device reset command...");

        definitions::ResetDeviceTx request;
        // Note: Device typically does not send a response as it immediately resets
        packet_handler_->send(request);

        spdlog::info("Reset command sent - watch will reboot");
    }

    std::string KernelOperations::resetGpsProcessor()
    {
        spdlog::info("Resetting GPS processor...");

        definitions::ResetGpsTx request;
        auto response = packet_handler_->transaction<definitions::ResetGpsTx, definitions::ResetGpsRx>(request);

        std::string message(
            reinterpret_cast<const char *>(response.raw_payload_bytes.data()),
            response.raw_payload_bytes.size());

        spdlog::info("GPS processor reset complete: {}", message);
        return message;
    }

    std::time_t KernelOperations::getTime()
    {
        spdlog::debug("Requesting time from watch...");

        definitions::GetWatchTimeTx request;
        auto response = packet_handler_->transaction<definitions::GetWatchTimeTx, definitions::GetWatchTimeRx>(request);

        uint32_t raw_time = response.packet.payload.time;
        std::time_t time = static_cast<std::time_t>(tomtom::core::utils::hostToBigEndian(raw_time));

        std::tm *tm_ptr = std::gmtime(&time);
        char time_buffer[26];
        if (tm_ptr)
        {
#ifdef _WIN32
            asctime_s(time_buffer, sizeof(time_buffer), tm_ptr);
#else
            asctime_r(tm_ptr, time_buffer);
#endif
            spdlog::debug("Watch time: {}", time_buffer);
        }
        return time;
    }

    std::string KernelOperations::getFirmwareVersion()
    {
        spdlog::debug("Requesting firmware version from watch...");

        definitions::GetFirmwareVersionTx request;
        auto response = packet_handler_->transaction<definitions::GetFirmwareVersionTx, definitions::GetFirmwareVersionRx>(request);

        std::string version(
            reinterpret_cast<const char *>(response.raw_payload_bytes.data()),
            response.raw_payload_bytes.size());

        spdlog::debug("Firmware version: {}", version);
        return version;
    }

    std::string KernelOperations::getBleVersion()
    {
        spdlog::debug("Requesting BLE version from watch...");

        definitions::GetBleVersionTx request;
        auto response = packet_handler_->transaction<definitions::GetBleVersionTx, definitions::GetBleVersionRx>(request);

        std::string version(
            reinterpret_cast<const char *>(response.raw_payload_bytes.data()),
            response.raw_payload_bytes.size());

        spdlog::debug("BLE version: {}", version);
        return version;
    }

    uint32_t KernelOperations::getProductId()
    {
        spdlog::debug("Requesting product ID from watch...");

        definitions::GetProductIdTx request;
        auto response = packet_handler_->transaction<definitions::GetProductIdTx, definitions::GetProductIdRx>(request);

        uint32_t product_id = tomtom::core::utils::hostToBigEndian(response.packet.payload.product_id);

        spdlog::debug("Product ID: 0x{:08X}", product_id);
        return product_id;
    }
}
