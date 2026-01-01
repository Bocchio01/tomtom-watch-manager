#pragma once

#include "../packet_types.hpp"
#include "../payload_traits.hpp"
#include "common.hpp"

#pragma pack(push, 1)

namespace tomtom::protocol
{

    // ============================================================================
    // Find First File
    // ============================================================================

    /**
     * @brief Initialize file enumeration
     *
     * @details
     * Starts a file listing operation. The watch responds with the first file.
     * Subsequent files are retrieved using FindNextFile.
     */
    struct FindFirstFileTxPayload
    {
        uint32_t reserved_1{0};
        uint32_t reserved_2{0};
    };

    template <>
    struct PayloadTraits<FindFirstFileTxPayload>
    {
        static constexpr MessageType type = MessageType::FIND_FIRST_FILE;
        static constexpr PacketDirection direction = PacketDirection::TX;
    };

    struct FindFirstFileRxPayload : FindResponse
    {
    };

    template <>
    struct PayloadTraits<FindFirstFileRxPayload>
    {
        static constexpr MessageType type = MessageType::FIND_FIRST_FILE;
        static constexpr PacketDirection direction = PacketDirection::RX;
    };

    // ============================================================================
    // Find Next File
    // ============================================================================

    /**
     * @brief Continue file enumeration
     *
     * @details
     * Retrieves the next file after a FindFirst or previous FindNext.
     * The end_of_list field in the response indicates when enumeration is complete.
     */
    struct FindNextFileTxPayload
    {
        // Empty payload
    };

    template <>
    struct PayloadTraits<FindNextFileTxPayload>
    {
        static constexpr MessageType type = MessageType::FIND_NEXT_FILE;
        static constexpr PacketDirection direction = PacketDirection::TX;
    };

    struct FindNextFileRxPayload : FindResponse
    {
    };

    template <>
    struct PayloadTraits<FindNextFileRxPayload>
    {
        static constexpr MessageType type = MessageType::FIND_NEXT_FILE;
        static constexpr PacketDirection direction = PacketDirection::RX;
    };

}

#pragma pack(pop)
