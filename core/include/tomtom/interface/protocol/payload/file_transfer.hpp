#pragma once

#include "../packet_types.hpp"
#include "../payload_traits.hpp"

#pragma pack(push, 1)

namespace tomtom::protocol
{

    // ============================================================================
    // Read File Data
    // ============================================================================

    /**
     * @brief Request to read data from an open file
     *
     * @details
     * Requests reading 'length' bytes from the file identified by file_id.
     * The watch responds with ReadFileDataRxPayload containing the data.
     */
    struct ReadFileDataTxPayload
    {
        FileId file_id;
        uint32_t length;
    };

    template <>
    struct PayloadTraits<ReadFileDataTxPayload>
    {
        static constexpr MessageType type = MessageType::READ_FILE_DATA_REQUEST;
        static constexpr PacketDirection direction = PacketDirection::TX;
    };

    /**
     * @brief Response containing file data
     *
     * @details
     * Variable-length payload. The actual data follows this structure.
     * Use read_length to determine how many data bytes follow.
     */
    struct ReadFileDataRxPayload
    {
        FileId file_id;
        uint32_t read_length;
        // uint8_t data[];
    };

    template <>
    struct PayloadTraits<ReadFileDataRxPayload>
    {
        static constexpr MessageType type = MessageType::READ_FILE_DATA_RESPONSE;
        static constexpr PacketDirection direction = PacketDirection::RX;
    };

    // ============================================================================
    // Write File Data
    // ============================================================================

    /**
     * @brief Write data to an open file
     *
     * @details
     * Variable-length payload. Data to write follows the file_id.
     * The total write size is determined by the packet length.
     */
    struct WriteFileDataTxPayload
    {
        FileId file_id;
        // uint8_t data[];
    };

    template <>
    struct PayloadTraits<WriteFileDataTxPayload>
    {
        static constexpr MessageType type = MessageType::WRITE_FILE_DATA;
        static constexpr PacketDirection direction = PacketDirection::TX;
    };

    /**
     * @brief Acknowledgment of write operation
     */
    struct WriteFileDataRxPayload
    {
        uint32_t reserved_1;
        FileId file_id;
        uint32_t reserved_2;
        uint32_t reserved_3;
        uint32_t reserved_4;
    };

    template <>
    struct PayloadTraits<WriteFileDataRxPayload>
    {
        static constexpr MessageType type = MessageType::WRITE_FILE_DATA;
        static constexpr PacketDirection direction = PacketDirection::RX;
    };

}

#pragma pack(pop)
