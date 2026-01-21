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
        definitions::FormatWatchTx request;
        auto response = packet_handler_->transaction<definitions::FormatWatchTx, definitions::FormatWatchRx>(request);

        if (response.packet.payload.error != definitions::ProtocolError::SUCCESS)
        {
            throw std::runtime_error("Failed to format watch (Error: " +
                                     std::to_string(static_cast<uint32_t>(response.packet.payload.error)) + ")");
        }
    }

    void KernelOperations::resetDevice()
    {
        definitions::ResetDeviceTx request;
        // Note: Device typically does not send a response as it immediately resets
        packet_handler_->send(request);
    }

    std::string KernelOperations::resetGpsProcessor()
    {
        definitions::ResetGpsTx request;
        auto response = packet_handler_->transaction<definitions::ResetGpsTx, definitions::ResetGpsRx>(request);

        std::string message(
            reinterpret_cast<const char *>(response.raw_payload_bytes.data()),
            response.raw_payload_bytes.size());

        return message;
    }

    std::time_t KernelOperations::getTime()
    {
        definitions::GetWatchTimeTx request;
        auto response = packet_handler_->transaction<definitions::GetWatchTimeTx, definitions::GetWatchTimeRx>(request);

        uint32_t raw_time = response.packet.payload.time;
        std::time_t time = static_cast<std::time_t>(tomtom::core::utils::hostToBigEndian(raw_time));

        return time;
    }

    std::string KernelOperations::getFirmwareVersion()
    {
        definitions::GetFirmwareVersionTx request;
        auto response = packet_handler_->transaction<definitions::GetFirmwareVersionTx, definitions::GetFirmwareVersionRx>(request);

        std::string version(
            reinterpret_cast<const char *>(response.raw_payload_bytes.data()),
            response.raw_payload_bytes.size());

        return version;
    }

    std::string KernelOperations::getBleVersion()
    {
        definitions::GetBleVersionTx request;
        auto response = packet_handler_->transaction<definitions::GetBleVersionTx, definitions::GetBleVersionRx>(request);

        std::string version(
            reinterpret_cast<const char *>(response.raw_payload_bytes.data()),
            response.raw_payload_bytes.size());

        return version;
    }

    uint32_t KernelOperations::getProductId()
    {
        definitions::GetProductIdTx request;
        auto response = packet_handler_->transaction<definitions::GetProductIdTx, definitions::GetProductIdRx>(request);

        uint32_t product_id = tomtom::core::utils::hostToBigEndian(response.packet.payload.product_id);

        return product_id;
    }
}
