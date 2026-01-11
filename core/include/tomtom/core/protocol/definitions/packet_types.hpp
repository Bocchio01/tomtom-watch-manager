#pragma once

#include <cstdint>
#include <array>

namespace tomtom::core::protocol::definitions
{
    /**
     * @brief Direction of packets communication
     */
    enum class PacketDirection : uint8_t
    {
        TX = 0x09, // To device
        RX = 0x01, // From device
        UNKNOWN = 0xFF
    };

    /**
     * @brief Message types supported by the TomTom watch protocol
     */
    enum class MessageType : uint8_t
    {
        // File Operations
        OPEN_FILE_WRITE = 0x02,
        DELETE_FILE = 0x03,
        WRITE_FILE_DATA = 0x04,
        GET_FILE_SIZE = 0x05,
        OPEN_FILE_READ = 0x06,
        READ_FILE_DATA_REQUEST = 0x07,
        READ_FILE_DATA_RESPONSE = 0x09,
        // FIND_CLOSE = 0x0A,
        CLOSE_FILE = 0x0C,

        // Watch Control
        FORMAT_WATCH = 0x0E,
        RESET_DEVICE = 0x10,
        RESET_GPS_PROCESSOR = 0x1D,

        // File Discovery
        FIND_FIRST_FILE = 0x11,
        FIND_NEXT_FILE = 0x12,

        // Watch Information
        GET_WATCH_TIME = 0x14,
        GET_PRODUCT_ID = 0x20,
        GET_FIRMWARE_VERSION = 0x21,
        GET_BLE_VERSION = 0x28,

        // Unknown/Reserved
        UNKNOWN_1F = 0x1F,
        UNKNOWN_22 = 0x22,
        UNKNOWN_23 = 0x23,
        UNKNOWN = 0xFF
    };

    /**
     * @brief Protocol error codes
     * @note Expand this enum as more error codes are discovered
     */
    enum class ProtocolError : uint32_t
    {
        SUCCESS = 0x00000000,
        UNKNOWN_ERROR = 0x00000001,
        FILE_NOT_FOUND = 0x00000002,
        ACCESS_DENIED = 0x00000003,
        INVALID_HANDLE = 0x00000004
    };

}