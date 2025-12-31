#include "tomtom/protocol.hpp"
#include "tomtom/watch.hpp" // For WatchError enum
#include <spdlog/spdlog.h>
#include <cstring>
#include <algorithm>

namespace tomtom
{

    WatchProtocol::WatchProtocol(DeviceConnection &conn, uint32_t pid)
        : connection_(conn), productId_(pid), messageCounter_(0) {}

    size_t WatchProtocol::getReadChunkSize() const
    {
        // Logic from UsbConnection.java getFileReadChunkSize
        if (productId_ == TOMTOM_MULTISPORT_PRODUCT_ID)
            return 50;
        return 242; // Spark/Runner/Adventurer
    }

    size_t WatchProtocol::getWriteChunkSize() const
    {
        // Logic from UsbConnection.java getFileWriteChunkSize
        if (productId_ == TOMTOM_MULTISPORT_PRODUCT_ID)
            return 54;
        return 246; // Spark/Runner/Adventurer
    }

    WatchError WatchProtocol::sendTransaction(uint8_t msgId, const std::vector<uint8_t> &txPayload, std::vector<uint8_t> &rxPayload, uint8_t expectedRxMsgId)
    {
        if (!connection_.isOpen())
            return WatchError::UnableToOpenDevice;

        // 1. Construct Packet (Framing)
        // Header: [0x09] [Len+2] [Cnt] [MsgID] [Payload...]
        uint8_t txLength = static_cast<uint8_t>(txPayload.size());
        std::vector<uint8_t> packet(256, 0);

        packet[0] = 0x09;
        packet[1] = txLength + 2;
        packet[2] = messageCounter_; // Sequence number
        packet[3] = msgId;

        if (!txPayload.empty())
        {
            std::memcpy(packet.data() + 4, txPayload.data(), txLength);
        }

        // Determine HID packet size based on device type
        size_t packetSize = 256;
        if (productId_ == TOMTOM_MULTISPORT_PRODUCT_ID)
        {
            packetSize = txLength + 4;
        }

        // 2. Send
        if (connection_.write(packet.data(), packetSize, 5000) < 0)
        {
            return WatchError::UnableToSendPacket;
        }

        // Increment counter after send
        uint8_t sentCounter = messageCounter_++;

        // 3. Receive
        std::vector<uint8_t> rxBuffer(256, 0);
        int bytesRead = connection_.read(rxBuffer.data(), 256, 5000);

        if (bytesRead < 0)
            return WatchError::UnableToReceivePacket;

        // 4. Validate Response
        // Header: [0x01] [Len+2] [Cnt] [MsgID]
        if (rxBuffer[0] != 0x01)
            return WatchError::InvalidResponse;
        if (rxBuffer[2] != sentCounter)
            return WatchError::OutOfSyncResponse;
        if (rxBuffer[3] != expectedRxMsgId)
            return WatchError::UnexpectedResponse;

        // Extract Payload
        int payloadLen = (rxBuffer[1] & 0xFF) - 2;
        if (payloadLen > 0)
        {
            rxPayload.assign(rxBuffer.begin() + 4, rxBuffer.begin() + 4 + payloadLen);
        }
        else
        {
            rxPayload.clear();
        }

        return WatchError::NoError;
    }

    // --- Command Implementations ---

    WatchError WatchProtocol::openFile(uint32_t fileId, bool writeMode)
    {
        std::vector<uint8_t> tx(4);
        // Big Endian encoding for fileId
        tx[0] = (fileId >> 24) & 0xFF;
        tx[1] = (fileId >> 16) & 0xFF;
        tx[2] = (fileId >> 8) & 0xFF;
        tx[3] = (fileId) & 0xFF;

        std::vector<uint8_t> rx;
        uint8_t cmd = writeMode ? MSG_OPEN_FILE_WRITE : MSG_OPEN_FILE_READ;

        WatchError err = sendTransaction(cmd, tx, rx, cmd);
        if (err != WatchError::NoError)
            return err;

        // Check internal error code in response (offset 12 in payload, so index 16 total)
        // Payload: [FileID(4)] ... [Error(4)]
        if (rx.size() >= 16)
        {
            uint32_t error = (rx[12] << 24) | (rx[13] << 16) | (rx[14] << 8) | rx[15];
            if (error != 0)
                return WatchError::InvalidParameter; // File open error
        }
        return WatchError::NoError;
    }

    WatchError WatchProtocol::readFileChunk(uint32_t fileId, uint32_t length, std::vector<uint8_t> &data)
    {
        std::vector<uint8_t> tx(8);
        // FileID
        tx[0] = (fileId >> 24) & 0xFF;
        tx[1] = (fileId >> 16) & 0xFF;
        tx[2] = (fileId >> 8) & 0xFF;
        tx[3] = (fileId) & 0xFF;
        // Length
        tx[4] = (length >> 24) & 0xFF;
        tx[5] = (length >> 16) & 0xFF;
        tx[6] = (length >> 8) & 0xFF;
        tx[7] = (length) & 0xFF;

        std::vector<uint8_t> rx;
        WatchError err = sendTransaction(MSG_READ_FILE_DATA_REQUEST, tx, rx, MSG_READ_FILE_DATA_RESPONSE);
        if (err != WatchError::NoError)
            return err;

        // Payload: [FileID(4)] [BytesRead(4)] [Data...]
        if (rx.size() < 8)
            return WatchError::IncorrectResponseLength;

        uint32_t bytesRead = (rx[4] << 24) | (rx[5] << 16) | (rx[6] << 8) | rx[7];
        if (bytesRead != length)
            return WatchError::ParseError;

        data.assign(rx.begin() + 8, rx.end());
        return WatchError::NoError;
    }

    WatchError WatchProtocol::getFileSize(uint32_t fileId, uint32_t &size)
    {
        std::vector<uint8_t> tx(4);
        tx[0] = (fileId >> 24) & 0xFF;
        tx[1] = (fileId >> 16) & 0xFF;
        tx[2] = (fileId >> 8) & 0xFF;
        tx[3] = (fileId) & 0xFF;

        std::vector<uint8_t> rx;
        WatchError err = sendTransaction(MSG_GET_FILE_SIZE, tx, rx, MSG_GET_FILE_SIZE);
        if (err != WatchError::NoError)
            return err;

        // Payload: [FileID(4)] ... [Size(4)] [Error(4)]
        if (rx.size() < 12)
            return WatchError::IncorrectResponseLength;

        size = (rx[8] << 24) | (rx[9] << 16) | (rx[10] << 8) | rx[11];
        return WatchError::NoError;
    }

    WatchError WatchProtocol::closeFile(uint32_t fileId)
    {
        std::vector<uint8_t> tx(4);
        tx[0] = (fileId >> 24) & 0xFF;
        tx[1] = (fileId >> 16) & 0xFF;
        tx[2] = (fileId >> 8) & 0xFF;
        tx[3] = (fileId) & 0xFF;

        std::vector<uint8_t> rx;
        return sendTransaction(MSG_CLOSE_FILE, tx, rx, MSG_CLOSE_FILE);
    }

    // ... Implementations for findFirst/Next, writeChunk, etc. follow similar pattern ...
}