#pragma once
#include "packet_types.hpp"

#pragma pack(push, 1)

namespace tomtom::interface::protocol
{

    /**
     * @brief Common packet header for all messages
     *
     * Contains metadata about the packet such as direction, length, counter, and message type.
     * Length specifies the number of bytes following the length field (i.e., Counter + MsgType + Payload).
     * Each field is 1 byte (uint8_t).
     * Counter is a rolling counter incremented with each packet and should be echoed back in responses.
     */
    struct PacketHeader
    {
        PacketDirection direction;
        uint8_t length;
        uint8_t counter;
        MessageType type;
    };

    static_assert(sizeof(PacketHeader) == 4);

}

#pragma pack(pop)
