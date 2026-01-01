#pragma once

#include <memory>
#include <vector>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <spdlog/spdlog.h>

#include "tomtom/transport/connection.hpp"
#include "tomtom/protocol/definition/packet.hpp"
#include "tomtom/protocol/runtime/packet_validation.hpp"
#include "tomtom/protocol/runtime/transaction_exceptions.hpp"

namespace tomtom::protocol::runtime
{
    /**
     * @brief Handles serialization and transport of packets over a connection.
     */
    class PacketHandler
    {
    public:
        explicit PacketHandler(std::shared_ptr<transport::DeviceConnection> connection);
        ~PacketHandler() = default;

        /**
         * @brief Sends a packet to the device.
         *
         * @tparam TxPacket Type of packet to send (must be Packet<T>).
         * @param packet The packet instance.
         */
        template <typename TxPacket>
        void send(const TxPacket &packet)
        {
            // Serialize
            const uint8_t *data = reinterpret_cast<const uint8_t *>(&packet);
            size_t size = packet.size();

            writeBytes(data, size);
        }

        /**
         * @brief Receives a specific packet type from the device.
         *
         * @tparam RxPacket The expected return packet type (Packet<T>).
         * @return The deserialized packet.
         */
        template <typename RxPacket>
        RxPacket receive()
        {
            // 1. Read the first 2 bytes (Direction + Length)
            // Header structure: [Direction(1)][Length(1)][Counter(1)][Type(1)]

            uint8_t prefix[2];
            readBytes(prefix, 2);

            auto direction = static_cast<definition::PacketDirection>(prefix[0]);
            uint8_t remaining_len = prefix[1];

            // 2. Basic Validation
            if (direction != definition::PacketDirection::RX)
            {
                throw MalformedPacketError("Received packet with non-RX direction");
            }

            // 3. Read the rest of the packet based on Length field
            // The length field includes Counter + Type + Payload
            std::vector<uint8_t> buffer(2 + remaining_len);
            buffer[0] = prefix[0];
            buffer[1] = prefix[1];

            spdlog::debug("Receiving packet: direction=0x{:02X}, length={}", static_cast<uint8_t>(direction), remaining_len);

            if (remaining_len > 0)
            {
                readBytes(buffer.data() + 2, remaining_len);
            }

            // Format bytes as hex string (space separated)
            std::ostringstream oss;
            oss << std::hex << std::uppercase << std::setfill('0');
            for (size_t i = 0; i < buffer.size(); ++i)
            {
                oss << std::setw(2) << static_cast<int>(buffer[i]);
                if (i + 1 < buffer.size())
                    oss << ' ';
            }
            spdlog::debug("Received {} bytes: {}", buffer.size(), oss.str());

            // 4. Cast to Packet Structure (Zero-Copy-ish)
            // Safety check: verify buffer size matches expected struct size
            // For variable length packets, RxPacket::size() might differ from buffer.size()
            // This implementation assumes Fixed Size responses for simple commands.
            if (buffer.size() < sizeof(RxPacket))
            {
                // This might happen if the device sends less data than the struct defines
                spdlog::warn("Received packet size {} is smaller than struct size {}",
                             buffer.size(), sizeof(RxPacket));
            }

            RxPacket packet;
            // Use memcpy to avoid strict aliasing violations or alignment issues
            std::memcpy(&packet, buffer.data(), std::min(buffer.size(), sizeof(RxPacket)));

            // 5. Type Validation
            // Get traits from the payload type of the requested RxPacket
            using PayloadType = decltype(packet.payload);
            using Traits = definition::PayloadTraits<PayloadType>;

            if (packet.header.type != Traits::type)
            {
                throw UnexpectedPacketError(Traits::type, packet.header.type);
            }

            return packet;
        }

        /**
         * @brief Performs a synchronous Request-Response transaction.
         *
         * @tparam TxPacket Type of the command packet.
         * @tparam RxPacket Type of the expected response packet.
         * @param tx The command packet to send.
         * @return RxPacket The response received.
         */
        template <typename TxPacket, typename RxPacket>
        RxPacket transaction(const TxPacket &tx)
        {
            send(tx);
            return receive<RxPacket>();
        }

    private:
        std::shared_ptr<transport::DeviceConnection> connection_;
        uint8_t current_counter_ = 0;

        // Helper methods implemented in .cpp
        void writeBytes(const uint8_t *data, size_t size);
        void readBytes(uint8_t *buffer, size_t size);
    };
}