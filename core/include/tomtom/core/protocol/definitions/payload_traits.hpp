#pragma once

#include "packet_types.hpp"

namespace tomtom::core::protocol::definitions
{
    /**
     * @brief Traits struct to bind metadata to payload types.
     * Specializations must define:
     * static constexpr MessageType type;
     * static constexpr PacketDirection direction;
     */
    template <typename PacketPayload>
    struct PayloadTraits
    {
        static constexpr MessageType type = MessageType::UNKNOWN;
        static constexpr PacketDirection direction = PacketDirection::UNKNOWN;
    };
}
