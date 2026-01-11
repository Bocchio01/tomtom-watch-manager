#pragma once

#include <memory>
#include <vector>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <spdlog/spdlog.h>

#include "tomtom/core/transport/connection.hpp"
#include "tomtom/core/protocol/definitions/definitions.hpp"
#include "tomtom/core/protocol/runtime/packet_validation.hpp"
#include "tomtom/core/protocol/runtime/transaction_exceptions.hpp"

namespace tomtom::core::protocol::runtime
{
    /**
     * @brief Simple view over a contiguous range of bytes (C++17 compatible).
     */
    struct ByteView
    {
        const uint8_t *data_ptr;
        size_t data_size;

        const uint8_t *begin() const { return data_ptr; }
        const uint8_t *end() const { return data_ptr + data_size; }
        size_t size() const { return data_size; }
        bool empty() const { return data_size == 0; }
    };

    /**
     * @brief Holds a parsed packet along with the raw payload bytes.
     *
     * This is necessary for variable-length payloads where the structured
     * payload type only contains fixed fields (metadata) and the actual
     * data follows in the raw bytes.
     */
    template <typename PacketType>
    struct PacketResponse
    {
        PacketType packet;
        std::vector<uint8_t> raw_payload_bytes;

        /**
         * @brief Get a view over the variable data portion.
         *
         * @tparam PayloadType The structured payload type.
         * @return View over bytes following the structured payload.
         */
        template <typename PayloadType = typename PacketType::payload_type>
        ByteView variable_data() const
        {
            if (raw_payload_bytes.size() < sizeof(PayloadType))
            {
                return ByteView{nullptr, 0};
            }
            return ByteView{
                raw_payload_bytes.data() + sizeof(PayloadType),
                raw_payload_bytes.size() - sizeof(PayloadType)};
        }
    };

    /**
     * @brief Handles serialization and transport of packets over a connection.
     */
    class PacketHandler
    {
    public:
        explicit PacketHandler(std::shared_ptr<transport::DeviceConnection> connection) : connection_(std::move(connection))
        {
            if (!connection_)
            {
                throw std::invalid_argument("PacketHandler requires a valid connection");
            }
        }

        ~PacketHandler() = default;

        /**
         * @brief Sends a packet to the device, optionally appending variable-length data.
         *
         * @tparam TxPacket Type of packet to send (must be Packet<T>).
         * @param packet The packet instance.
         * @param extra_data Optional raw bytes to append (e.g., for file writing).
         */
        template <typename TxPacket>
        void send(const TxPacket &packet, const std::vector<uint8_t> &extra_data = {})
        {
            TxPacket tx_copy = packet;
            tx_copy.header.counter = current_counter_++;

            // Adjust length field to include the extra data
            // Header length = (Counter + Type + StructuredPayload) + ExtraData
            tx_copy.header.length += static_cast<uint8_t>(extra_data.size());

            std::vector<uint8_t> buffer;
            buffer.reserve(tx_copy.size() + extra_data.size());

            // Serialize structured packet
            const uint8_t *pkg_ptr = reinterpret_cast<const uint8_t *>(&tx_copy);
            buffer.insert(buffer.end(), pkg_ptr, pkg_ptr + tx_copy.size());

            // Append extra data
            if (!extra_data.empty())
            {
                buffer.insert(buffer.end(), extra_data.begin(), extra_data.end());
            }

            int result = connection_->write(buffer.data(), buffer.size(), 2000); // 2 second timeout
            if (result < 0)
            {
                throw ConnectionError("Failed to write to device (IO Error)");
            }
        }

        /**
         * @brief Receives a specific packet type from the device.
         *
         * @tparam RxPacket The expected return packet type (Packet<T>).
         * @return PacketResponse containing the deserialized packet and raw payload bytes.
         */
        template <typename RxPacket>
        PacketResponse<RxPacket> receive()
        {
            PacketResponse<RxPacket> response;

            // Read the header first
            std::vector<uint8_t> header_bytes(sizeof(definitions::PacketHeader));
            connection_->read(header_bytes.data(), header_bytes.size(), 2000);
            printBytesAsHex(header_bytes.data(), header_bytes.size());

            // Read the payload based on length in header
            std::vector<uint8_t> payload_bytes(header_bytes[1] - 2); // Subtract counter(1) + type(1)
            connection_->read(payload_bytes.data(), payload_bytes.size(), 2000);
            printBytesAsHex(payload_bytes.data(), payload_bytes.size());

            // Construct the packet header
            std::memcpy(&response.packet.header, header_bytes.data(), header_bytes.size());

            // Copy structured payload (only up to sizeof(payload))
            size_t copy_size = std::min(payload_bytes.size(), sizeof(response.packet.payload));
            std::memcpy(&response.packet.payload, payload_bytes.data(), copy_size);

            // Store raw payload bytes for variable-length access
            response.raw_payload_bytes = std::move(payload_bytes);

            // Clear any remaining data in the connection buffer
            connection_->read(nullptr, 0, 0);

            return response;
        }

        /**
         * @brief Performs a synchronous Request-Response transaction.
         *
         * @tparam TxPacket Type of the command packet.
         * @tparam RxPacket Type of the expected response packet.
         * @param tx The command packet to send.
         * @param extra_data Optional raw bytes to append to the request.
         * @return PacketResponse<RxPacket> The response with raw payload bytes.
         */
        template <typename TxPacket, typename RxPacket>
        PacketResponse<RxPacket> transaction(const TxPacket &tx, const std::vector<uint8_t> &extra_data = {})
        {
            send(tx, extra_data);
            auto rx = receive<RxPacket>();

            // Validate that the response corresponds to the request if needed
            using PayloadType = decltype(rx.packet.payload);
            using Traits = definitions::PayloadTraits<PayloadType>;

            if (rx.packet.header.type != Traits::type)
            {
                throw UnexpectedPacketError(Traits::type, rx.packet.header.type);
            }

            return rx;
        }

    private:
        std::shared_ptr<transport::DeviceConnection> connection_;
        uint8_t current_counter_ = 0;

        void printBytesAsHex(const uint8_t *data, size_t size)
        {
            std::ostringstream oss;
            oss << std::hex << std::uppercase << std::setfill('0');
            for (size_t i = 0; i < size; ++i)
            {
                oss << std::setw(2) << static_cast<int>(data[i]);
                if (i + 1 < size)
                    oss << ' ';
            }
            spdlog::debug("Data ({} bytes): {}", size, oss.str());
        }
    };
}