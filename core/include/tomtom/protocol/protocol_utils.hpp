#pragma once

#include <algorithm>
#include <cstring>
#include <stdexcept>

#include "protocol_types.hpp"

namespace tomtom::protocol::utils
{

    /**
     * @brief Write 32-bit integer in little-endian format
     */
    inline void writeUInt32LE(std::vector<uint8_t> &buffer, uint32_t value)
    {
        buffer.push_back(static_cast<uint8_t>(value & 0xFF));
        buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
    }

    /**
     * @brief Read 32-bit integer in little-endian format
     */
    inline uint32_t readUInt32LE(const uint8_t *data)
    {
        return static_cast<uint32_t>(data[0]) |
               (static_cast<uint32_t>(data[1]) << 8) |
               (static_cast<uint32_t>(data[2]) << 16) |
               (static_cast<uint32_t>(data[3]) << 24);
    }

    /**
     * @brief Read 32-bit integer from vector at offset
     */
    inline uint32_t readUInt32LE(const std::vector<uint8_t> &buffer, size_t offset)
    {
        if (offset + 4 > buffer.size())
        {
            throw std::out_of_range("Buffer too small for uint32");
        }
        return readUInt32LE(buffer.data() + offset);
    }

    /**
     * @brief Build payload for FIND_FIRST_FILE (matches TXFindFirstFilePacket)
     * Contains two uint32_t unknowns (both 0)
     */
    inline std::vector<uint8_t> buildFindFirstPayload()
    {
        std::vector<uint8_t> payload(8, 0); // 2 x uint32_t = 8 bytes
        return payload;
    }

    /**
     * @brief Build payload for file operations - TXFileOperationPacket
     * Just the file_id (4 bytes)
     */
    inline std::vector<uint8_t> buildFilePayload(uint32_t file_id)
    {
        std::vector<uint8_t> payload;
        writeUInt32LE(payload, file_id);
        return payload;
    }

    /**
     * @brief Build payload for read file data request - TXReadFileDataPacket
     * Format: [file_id:4][length:4]
     */
    inline std::vector<uint8_t> buildReadPayload(uint32_t file_id, uint32_t length)
    {
        std::vector<uint8_t> payload;
        writeUInt32LE(payload, file_id);
        writeUInt32LE(payload, length);
        return payload;
    }

    /**
     * @brief Build payload for write file data - TXWriteFileDataPacket
     * Format: [file_id:4][data:246]
     * NOTE: data must be exactly 246 bytes or less (will be padded to 246)
     */
    inline std::vector<uint8_t> buildWritePayload(uint32_t file_id, const std::vector<uint8_t> &data)
    {
        if (data.size() > MAX_WRITE_DATA_SIZE)
        {
            throw std::runtime_error("Write data exceeds maximum chunk size (246 bytes)");
        }

        std::vector<uint8_t> payload;
        writeUInt32LE(payload, file_id);
        payload.insert(payload.end(), data.begin(), data.end());

        // Pad to 246 bytes total data size if needed
        if (payload.size() - 4 < MAX_WRITE_DATA_SIZE)
        {
            payload.resize(4 + MAX_WRITE_DATA_SIZE, 0);
        }

        return payload;
    }

    /**
     * @brief Parse RXFileOperationPacket response
     * Format: [_unk1:4][id:4][_unk2:8][error:4] = 20 bytes
     */
    inline FileInfo parseFileOperationResponse(const std::vector<uint8_t> &payload)
    {
        if (payload.size() < 20)
        {
            throw std::runtime_error("Response payload too small for file operation");
        }

        FileInfo info;
        // Skip _unk1 (offset 0-3)
        info.file_id = readUInt32LE(payload, 4);
        // Skip _unk2 (offset 8-15)
        info.error = static_cast<ErrorCode>(readUInt32LE(payload, 16));
        info.size = 0;
        info.end_of_list = false;

        return info;
    }

    /**
     * @brief Parse RXGetFileSizePacket response
     * Format: [_unk1:4][id:4][_unk2:4][file_size:4][_unk3:4] = 20 bytes
     */
    inline FileInfo parseFileSizeResponse(const std::vector<uint8_t> &payload)
    {
        if (payload.size() < 20)
        {
            throw std::runtime_error("Response payload too small for file size");
        }

        FileInfo info;
        // Skip _unk1 (offset 0-3)
        info.file_id = readUInt32LE(payload, 4);
        // Skip _unk2 (offset 8-11)
        info.size = readUInt32LE(payload, 12);
        // Skip _unk3 (offset 16-19)
        info.error = ErrorCode::SUCCESS;
        info.end_of_list = false;

        return info;
    }

    /**
     * @brief Parse RXFindFilePacket response (FIND_FIRST_FILE / FIND_NEXT_FILE)
     * Format: [_unk1:4][id:4][_unk2:4][file_size:4][end_of_list:4] = 20 bytes
     */
    inline FileInfo parseFindFileResponse(const std::vector<uint8_t> &payload)
    {
        if (payload.size() < 20)
        {
            throw std::runtime_error("Response payload too small for find file");
        }

        FileInfo info;
        // Skip _unk1 (offset 0-3)
        info.file_id = readUInt32LE(payload, 4);
        // Skip _unk2 (offset 8-11)
        info.size = readUInt32LE(payload, 12);
        uint32_t end_flag = readUInt32LE(payload, 16);
        info.end_of_list = (end_flag != 0);
        info.error = ErrorCode::SUCCESS;

        return info;
    }

    /**
     * @brief Parse RXReadFileDataPacket response
     * Format: [id:4][data_length:4][data:242]
     * Returns only the actual data (without id and data_length)
     */
    inline std::vector<uint8_t> parseReadFileDataResponse(const std::vector<uint8_t> &payload)
    {
        if (payload.size() < 8)
        {
            throw std::runtime_error("Response payload too small for read file data");
        }

        uint32_t file_id = readUInt32LE(payload, 0);
        uint32_t data_length = readUInt32LE(payload, 4);

        (void)file_id; // Not used but available if needed

        if (data_length > MAX_READ_DATA_SIZE)
        {
            throw std::runtime_error("Data length exceeds maximum (242 bytes)");
        }

        if (payload.size() < 8 + data_length)
        {
            throw std::runtime_error("Payload too small for declared data length");
        }

        return std::vector<uint8_t>(
            payload.begin() + 8,
            payload.begin() + 8 + data_length);
    }

    /**
     * @brief Parse RXGetCurrentTimePacket response
     * Format: [utc_time:4][_unk:16] = 20 bytes
     */
    inline uint32_t parseCurrentTimeResponse(const std::vector<uint8_t> &payload)
    {
        if (payload.size() < 4)
        {
            throw std::runtime_error("Response payload too small for current time");
        }

        return readUInt32LE(payload, 0);
    }

    /**
     * @brief Parse RXGetFirmwareVersionPacket response
     * Format: [version:60] - null-terminated string
     */
    inline std::string parseFirmwareVersionResponse(const std::vector<uint8_t> &payload)
    {
        if (payload.empty())
        {
            return "";
        }

        // Find null terminator or use full 60 bytes
        size_t length = std::min<size_t>(60, payload.size());
        auto null_pos = std::find(payload.begin(), payload.begin() + length, '\0');
        if (null_pos != payload.begin() + length)
        {
            length = std::distance(payload.begin(), null_pos);
        }

        return std::string(reinterpret_cast<const char *>(payload.data()), length);
    }

    /**
     * @brief Parse RXGetProductIDPacket response
     * Format: [product_id:4]
     */
    inline uint32_t parseProductIdResponse(const std::vector<uint8_t> &payload)
    {
        if (payload.size() < 4)
        {
            throw std::runtime_error("Response payload too small for product ID");
        }

        return readUInt32LE(payload, 0);
    }

    /**
     * @brief Parse RXGetBLEVersionPacket response
     * Format: [ble_version:4]
     */
    inline uint32_t parseBleVersionResponse(const std::vector<uint8_t> &payload)
    {
        if (payload.size() < 4)
        {
            throw std::runtime_error("Response payload too small for BLE version");
        }

        return readUInt32LE(payload, 0);
    }

    /**
     * @brief Parse RXRebootWatchPacket response (GPS reset)
     * Format: [message:60] - null-terminated string
     */
    inline std::string parseRebootMessageResponse(const std::vector<uint8_t> &payload)
    {
        return parseFirmwareVersionResponse(payload); // Same format
    }

    /**
     * @brief Parse RXFormatWatchPacket response
     * Format: [_unk1:16][error:4] = 20 bytes
     */
    inline ErrorCode parseFormatWatchResponse(const std::vector<uint8_t> &payload)
    {
        if (payload.size() < 20)
        {
            throw std::runtime_error("Response payload too small for format watch");
        }

        return static_cast<ErrorCode>(readUInt32LE(payload, 16));
    }

}
