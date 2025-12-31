#pragma once

#include <memory>
#include <optional>
#include <chrono>

#include "tomtom/connection/connection.hpp"
#include "protocol_packet.hpp"

namespace tomtom::protocol
{

    /**
     * @brief Protocol handler - manages message exchange with the device
     *
     * This class handles:
     * - Message counter management
     * - Packet size detection based on device
     * - Request/response correlation
     * - Timeout handling
     * - Error detection
     */
    class ProtocolHandler
    {
    public:
        explicit ProtocolHandler(std::shared_ptr<DeviceConnection> connection)
            : connection_(std::move(connection)), counter_(0), packet_size_(256) // Default, will be set based on device
              ,
              default_timeout_ms_(5000)
        {
            if (!connection_)
            {
                throw std::invalid_argument("Connection cannot be null");
            }

            // Determine packet size based on device
            const auto &info = connection_->deviceInfo();
            packet_size_ = determinePacketSize(info);

            // Calculate max chunk sizes based on packet structures
            if (packet_size_ == 64)
            {
                max_write_chunk_ = MAX_PAYLOAD_SIZE_64 - 4; // -4 for file_id
                max_read_chunk_ = MAX_PAYLOAD_SIZE_64 - 8;  // -8 for id + data_length
            }
            else
            {
                max_write_chunk_ = MAX_WRITE_DATA_SIZE; // 246 bytes from struct
                max_read_chunk_ = MAX_READ_DATA_SIZE;   // 242 bytes from struct
            }
        }

        /**
         * @brief Send a request and receive response
         * @param msg_type Message type to send
         * @param payload Optional payload data
         * @param timeout_ms Timeout in milliseconds (0 = use default)
         * @return Response payload (empty if no payload)
         */
        std::vector<uint8_t> sendRequest(
            MessageType msg_type,
            const std::vector<uint8_t> &payload = {},
            int timeout_ms = 0)
        {
            if (!connection_->isOpen())
            {
                throw std::runtime_error("Connection not open");
            }

            if (timeout_ms == 0)
            {
                timeout_ms = default_timeout_ms_;
            }

            // Build request packet
            uint8_t current_counter = counter_++;
            auto request = Packet::buildRequest(msg_type, current_counter, payload, packet_size_);

            // Send request
            int bytes_written = connection_->write(request.data(), request.size(), timeout_ms);
            if (bytes_written != static_cast<int>(request.size()))
            {
                throw std::runtime_error("Failed to write complete packet");
            }

            // Receive response (unless it's a reset command - no response expected)
            if (msg_type == MessageType::RESET_DEVICE)
            {
                return {};
            }

            std::vector<uint8_t> response(packet_size_);
            int bytes_read = connection_->read(response.data(), response.size(), timeout_ms);

            if (bytes_read <= 0)
            {
                throw std::runtime_error("Failed to read response or timeout");
            }

            response.resize(bytes_read);

            // Parse and validate response
            PacketHeader response_header = Packet::parseHeader(response);

            if (!Packet::validateResponse(response_header, current_counter, msg_type))
            {
                throw std::runtime_error(
                    "Response validation failed: expected counter " +
                    std::to_string(current_counter) + " and type " +
                    messageTypeToString(msg_type));
            }

            // Extract and return payload
            return Packet::extractPayload(response);
        }

        /**
         * @brief Send request without expecting payload in response (but validate)
         */
        void sendCommand(MessageType msg_type, const std::vector<uint8_t> &payload = {}, int timeout_ms = 0)
        {
            (void)sendRequest(msg_type, payload, timeout_ms);
        }

        size_t maxWriteChunkSize() const { return max_write_chunk_; }
        size_t maxReadChunkSize() const { return max_read_chunk_; }
        size_t packetSize() const { return packet_size_; }

        void setDefaultTimeout(int timeout_ms) { default_timeout_ms_ = timeout_ms; }
        int defaultTimeout() const { return default_timeout_ms_; }

    private:
        std::shared_ptr<DeviceConnection> connection_;
        uint8_t counter_;
        size_t packet_size_;
        size_t max_write_chunk_;
        size_t max_read_chunk_;
        int default_timeout_ms_;

        /**
         * @brief Determine packet size based on device info
         */
        size_t determinePacketSize(const DeviceInfo &info)
        {
            // Multisport uses 64-byte packets, all others use 256
            if (info.product_id == 0x7474)
            { // Multisport
                return 64;
            }
            return 256;
        }
    };

}
