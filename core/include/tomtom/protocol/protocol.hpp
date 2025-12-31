#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include <ctime>
#include "tomtom/transport/usb_connection.hpp"
#include "tomtom/defines.hpp"

namespace tomtom
{

    // Forward declaration
    enum class WatchError;

    /**
     * @brief Protocol Layer: Handles packet framing, sequence counters, and atomic commands.
     * Corresponds to the packet handling logic in Java's UsbInterface.
     */
    class WatchProtocol
    {
    public:
        explicit WatchProtocol(DeviceConnection &connection, uint32_t productId);

        // --- Low-Level File Operations (Primitives) ---

        WatchError openFile(uint32_t fileId, bool writeMode);
        WatchError closeFile(uint32_t fileId);
        WatchError deleteFile(uint32_t fileId);
        WatchError getFileSize(uint32_t fileId, uint32_t &size);

        // Reads a single chunk of data
        WatchError readFileChunk(uint32_t fileId, uint32_t length, std::vector<uint8_t> &data);

        // Writes a single chunk of data
        WatchError writeFileChunk(uint32_t fileId, const std::vector<uint8_t> &data);

        // --- File Enumeration ---
        struct FileEntry
        {
            uint32_t fileId;
            uint32_t size;
        };

        WatchError findFirstFile(FileEntry &entry, bool &endOfList);
        WatchError findNextFile(FileEntry &entry, bool &endOfList);
        WatchError findClose();

        // --- System Commands ---
        WatchError getFirmwareVersion(std::string &version);
        WatchError getBleVersion(std::string &version);
        WatchError getCurrentTime(std::time_t &time);

        // Helper to determine max payload size based on model (Multisport vs Spark)
        size_t getReadChunkSize() const;
        size_t getWriteChunkSize() const;

    private:
        DeviceConnection &connection_;
        uint32_t productId_;
        uint8_t messageCounter_;

        // The generic transaction method: Send Request -> Receive Response -> Validate
        WatchError sendTransaction(
            uint8_t msgId,
            const std::vector<uint8_t> &txPayload,
            std::vector<uint8_t> &rxPayload,
            uint8_t expectedRxMsgId);
    };
}