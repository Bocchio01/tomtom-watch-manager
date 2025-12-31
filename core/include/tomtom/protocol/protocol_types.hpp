#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace tomtom::protocol
{

    // Protocol constants
    constexpr uint8_t START_BYTE_TX = 0x09;
    constexpr uint8_t START_BYTE_RX = 0x01;
    constexpr size_t HEADER_SIZE = 4;
    constexpr size_t MAX_PAYLOAD_SIZE_64 = 60;   // 64 - 4 bytes header
    constexpr size_t MAX_PAYLOAD_SIZE_256 = 252; // 256 - 4 bytes header
    constexpr size_t MAX_WRITE_DATA_SIZE = 246;  // From TXWriteFileDataPacket
    constexpr size_t MAX_READ_DATA_SIZE = 242;   // From RXReadFileDataPacket

    /**
     * @brief Message types as defined in the TomTom protocol
     */
    enum class MessageType : uint8_t
    {
        OPEN_FILE_WRITE = 0x02,
        DELETE_FILE = 0x03,
        WRITE_FILE_DATA = 0x04,
        GET_FILE_SIZE = 0x05,
        OPEN_FILE_READ = 0x06,
        READ_FILE_DATA_REQUEST = 0x07,
        READ_FILE_DATA_RESPONSE = 0x09,
        FIND_CLOSE = 0x0A,
        CLOSE_FILE = 0x0C,
        UNKNOWN_0D = 0x0D,
        FORMAT_WATCH = 0x0E,
        RESET_DEVICE = 0x10,
        FIND_FIRST_FILE = 0x11,
        FIND_NEXT_FILE = 0x12,
        GET_CURRENT_TIME = 0x14,
        UNKNOWN_1A = 0x1A,
        RESET_GPS_PROCESSOR = 0x1D,
        UNKNOWN_1F = 0x1F,
        GET_PRODUCT_ID = 0x20,
        GET_FIRMWARE_VERSION = 0x21,
        UNKNOWN_22 = 0x22,
        UNKNOWN_23 = 0x23,
        GET_BLE_VERSION = 0x28
    };

    /**
     * @brief Protocol error codes
     */
    enum class ErrorCode : uint32_t
    {
        SUCCESS = 0,
        FILE_NOT_FOUND = 1,
        FILE_OPEN_ERROR = 2,
        READ_ERROR = 3,
        WRITE_ERROR = 4,
        INVALID_OPERATION = 5,
        TIMEOUT = 6,
        PROTOCOL_ERROR = 7
    };

    /**
     * @brief Packet header structure (matches C struct exactly)
     */
    struct PacketHeader
    {
        uint8_t direction; // 0x09 = transmit, 0x01 = receive
        uint8_t length;    // of remaining packet (excluding direction and length)
        uint8_t counter;
        uint8_t message_id;

        PacketHeader()
            : direction(0), length(0), counter(0), message_id(0) {}

        PacketHeader(uint8_t dir, uint8_t len, uint8_t cnt, uint8_t msg)
            : direction(dir), length(len), counter(cnt), message_id(msg) {}
    };

    /**
     * @brief File information structure (parsed from RX packets)
     */
    struct FileInfo
    {
        uint32_t file_id;
        uint32_t size;
        ErrorCode error;
        bool end_of_list;

        FileInfo() : file_id(0), size(0), error(ErrorCode::SUCCESS), end_of_list(false) {}
    };

    /**
     * @brief Convert MessageType to string for debugging
     */
    inline std::string messageTypeToString(MessageType type)
    {
        switch (type)
        {
        case MessageType::OPEN_FILE_WRITE:
            return "OPEN_FILE_WRITE";
        case MessageType::DELETE_FILE:
            return "DELETE_FILE";
        case MessageType::WRITE_FILE_DATA:
            return "WRITE_FILE_DATA";
        case MessageType::GET_FILE_SIZE:
            return "GET_FILE_SIZE";
        case MessageType::OPEN_FILE_READ:
            return "OPEN_FILE_READ";
        case MessageType::READ_FILE_DATA_REQUEST:
            return "READ_FILE_DATA_REQUEST";
        case MessageType::READ_FILE_DATA_RESPONSE:
            return "READ_FILE_DATA_RESPONSE";
        case MessageType::FIND_CLOSE:
            return "FIND_CLOSE";
        case MessageType::CLOSE_FILE:
            return "CLOSE_FILE";
        case MessageType::FORMAT_WATCH:
            return "FORMAT_WATCH";
        case MessageType::RESET_DEVICE:
            return "RESET_DEVICE";
        case MessageType::FIND_FIRST_FILE:
            return "FIND_FIRST_FILE";
        case MessageType::FIND_NEXT_FILE:
            return "FIND_NEXT_FILE";
        case MessageType::GET_CURRENT_TIME:
            return "GET_CURRENT_TIME";
        case MessageType::RESET_GPS_PROCESSOR:
            return "RESET_GPS_PROCESSOR";
        case MessageType::GET_PRODUCT_ID:
            return "GET_PRODUCT_ID";
        case MessageType::GET_FIRMWARE_VERSION:
            return "GET_FIRMWARE_VERSION";
        case MessageType::GET_BLE_VERSION:
            return "GET_BLE_VERSION";
        default:
            return "UNKNOWN";
        }
    }

}
