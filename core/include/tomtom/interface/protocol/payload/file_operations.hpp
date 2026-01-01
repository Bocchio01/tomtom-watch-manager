#pragma once

#include "../packet_types.hpp"
#include "../payload_traits.hpp"
#include "common.hpp"

#pragma pack(push, 1)

namespace tomtom::protocol
{

    // ============================================================================
    // Open File for Reading
    // ============================================================================

    /**
     * @brief Request to open a file for reading
     */
    struct OpenFileReadTxPayload
    {
        FileId file_id;
    };

    template <>
    struct PayloadTraits<OpenFileReadTxPayload>
    {
        static constexpr MessageType type = MessageType::OPEN_FILE_READ;
        static constexpr PacketDirection direction = PacketDirection::TX;
    };

    struct OpenFileReadRxPayload : FileOperationResponse
    {
    };

    template <>
    struct PayloadTraits<OpenFileReadRxPayload>
    {
        static constexpr MessageType type = MessageType::OPEN_FILE_READ;
        static constexpr PacketDirection direction = PacketDirection::RX;
    };

    // ============================================================================
    // Open File for Writing
    // ============================================================================

    /**
     * @brief Request to open a file for writing
     */
    struct OpenFileWriteTxPayload
    {
        FileId file_id;
    };

    template <>
    struct PayloadTraits<OpenFileWriteTxPayload>
    {
        static constexpr MessageType type = MessageType::OPEN_FILE_WRITE;
        static constexpr PacketDirection direction = PacketDirection::TX;
    };

    struct OpenFileWriteRxPayload : FileOperationResponse
    {
    };

    template <>
    struct PayloadTraits<OpenFileWriteRxPayload>
    {
        static constexpr MessageType type = MessageType::OPEN_FILE_WRITE;
        static constexpr PacketDirection direction = PacketDirection::RX;
    };

    // ============================================================================
    // Close File
    // ============================================================================

    /**
     * @brief Request to close an open file
     */
    struct CloseFileTxPayload
    {
        FileId file_id;
    };

    template <>
    struct PayloadTraits<CloseFileTxPayload>
    {
        static constexpr MessageType type = MessageType::CLOSE_FILE;
        static constexpr PacketDirection direction = PacketDirection::TX;
    };

    struct CloseFileRxPayload : FileOperationResponse
    {
    };

    template <>
    struct PayloadTraits<CloseFileRxPayload>
    {
        static constexpr MessageType type = MessageType::CLOSE_FILE;
        static constexpr PacketDirection direction = PacketDirection::RX;
    };

    // ============================================================================
    // Delete File
    // ============================================================================

    /**
     * @brief Request to delete a file
     */
    struct DeleteFileTxPayload
    {
        FileId file_id;
    };

    template <>
    struct PayloadTraits<DeleteFileTxPayload>
    {
        static constexpr MessageType type = MessageType::DELETE_FILE;
        static constexpr PacketDirection direction = PacketDirection::TX;
    };

    struct DeleteFileRxPayload : FileOperationResponse
    {
    };

    template <>
    struct PayloadTraits<DeleteFileRxPayload>
    {
        static constexpr MessageType type = MessageType::DELETE_FILE;
        static constexpr PacketDirection direction = PacketDirection::RX;
    };

    // ============================================================================
    // Get File Size
    // ============================================================================

    /**
     * @brief Request file size information
     */
    struct GetFileSizeTxPayload
    {
        FileId file_id;
    };

    template <>
    struct PayloadTraits<GetFileSizeTxPayload>
    {
        static constexpr MessageType type = MessageType::GET_FILE_SIZE;
        static constexpr PacketDirection direction = PacketDirection::TX;
    };

    struct GetFileSizeRxPayload : FileSizeResponse
    {
    };

    template <>
    struct PayloadTraits<GetFileSizeRxPayload>
    {
        static constexpr MessageType type = MessageType::GET_FILE_SIZE;
        static constexpr PacketDirection direction = PacketDirection::RX;
    };

}

#pragma pack(pop)
