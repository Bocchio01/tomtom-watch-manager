#pragma once

#include <memory>
#include <vector>
#include <cstring>
#include <spdlog/spdlog.h>

#include "tomtom/connection/connection.hpp"
#include "tomtom/interface/protocol/packet.hpp"
#include "tomtom/interface/codec/packet_validation.hpp"
#include "tomtom/interface/codec/transaction_exceptions.hpp"

namespace tomtom::interface::codec
{
    /**
     * @brief Handles serialization and transport of packets over a connection.
     */
    class PacketHandler
    {
    public:
        explicit PacketHandler(std::shared_ptr<connection::DeviceConnection> connection);
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

            auto direction = static_cast<protocol::PacketDirection>(prefix[0]);
            uint8_t remaining_len = prefix[1];

            // 2. Basic Validation
            if (direction != protocol::PacketDirection::RX)
            {
                throw MalformedPacketError("Received packet with non-RX direction");
            }

            // 3. Read the rest of the packet based on Length field
            // The length field includes Counter + Type + Payload
            std::vector<uint8_t> buffer(2 + remaining_len);
            buffer[0] = prefix[0];
            buffer[1] = prefix[1];

            if (remaining_len > 0)
            {
                readBytes(buffer.data() + 2, remaining_len);
            }

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
            using Traits = protocol::PayloadTraits<PayloadType>;

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
            spdlog::debug("Starting transaction: Sending {} and expecting {}",
                          typeid(TxPacket).name(), typeid(RxPacket).name());
            tx.printInfo();
            send(tx);
            spdlog::debug("Packet sent, waiting for response...");
            return receive<RxPacket>();
        }

    private:
        std::shared_ptr<connection::DeviceConnection> connection_;
        uint8_t current_counter_ = 0;

        // Helper methods implemented in .cpp
        void writeBytes(const uint8_t *data, size_t size);
        void readBytes(uint8_t *buffer, size_t size);
    };
}