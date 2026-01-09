#pragma once

#include <spdlog/spdlog.h>

#include "packet_header.hpp"
#include "payload_traits.hpp"

#pragma pack(push, 1)

namespace tomtom::protocol::definition
{
    /**
     * @brief Smart Packet structure that auto-fills headers based on PacketPayload traits.
     *
     * @tparam PacketPayload The type of the payload (must have a PayloadTraits specialization)
     */
    template <typename PacketPayload>
    struct Packet
    {
        using header_type = PacketHeader;
        using payload_type = PacketPayload;

        PacketHeader header;
        PacketPayload payload;

        /**
         * @brief Default constructor.
         * Automatically populates the header fields (Type, Direction, Length)
         * based on the PacketPayload's traits.
         */
        Packet()
        {
            using Traits = PayloadTraits<PacketPayload>;
            constexpr bool empty_payload = std::is_empty_v<PacketPayload>;

            header.direction = Traits::direction;
            header.type = Traits::type;
            header.counter = 0; // Expected to be set by the connection manager if needed
            header.length =
                sizeof(header.counter) +
                sizeof(header.type) +
                (empty_payload ? 0 : sizeof(PacketPayload));
        }

        /**
         * @brief Constructor with payload data.
         */
        explicit Packet(const PacketPayload &p) : Packet()
        {
            payload = p;
        }

        /**
         * @brief Get the total packet size in bytes
         */
        constexpr size_t size() const
        {
            return sizeof(PacketHeader) + sizeof(PacketPayload);
        }

        /**
         * @brief Print packet information for debugging.
         */
        void printInfo() const
        {
            spdlog::info("Packet Info:");
            spdlog::info("  Direction: {}", static_cast<uint8_t>(header.direction));
            spdlog::info("  Type: {}", static_cast<uint8_t>(header.type));
            spdlog::info("  Counter: {}", static_cast<uint8_t>(header.counter));
            spdlog::info("  Length: {}", static_cast<uint8_t>(header.length));
            spdlog::info("  Total Size: {} bytes", size());
        }
    };

}

#pragma pack(pop)
