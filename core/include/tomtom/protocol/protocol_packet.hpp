#pragma once

#include <cstring>
#include <stdexcept>

#include "protocol_types.hpp"

namespace tomtom::protocol
{

    /**
     * @brief Packet builder and parser
     *
     * Handles the low-level packet format according to C struct definitions:
     * [direction][length][counter][message_id][payload...][padding]
     */
    class Packet
    {
    public:
        /**
         * @brief Construct a TX packet
         * @param msg_type Message type
         * @param counter Message counter
         * @param payload Payload data (optional)
         * @param packet_size Total packet size (64 or 256)
         */
        static std::vector<uint8_t> buildRequest(
            MessageType msg_type,
            uint8_t counter,
            const std::vector<uint8_t> &payload = {},
            size_t packet_size = 256)
        {
            if (payload.size() > packet_size - HEADER_SIZE)
            {
                throw std::runtime_error("Payload too large for packet size");
            }

            std::vector<uint8_t> packet(packet_size, 0);

            // Build header (matches PacketHeader struct)
            packet[0] = START_BYTE_TX;                            // direction
            packet[1] = static_cast<uint8_t>(2 + payload.size()); // length = counter + message_id + payload
            packet[2] = counter;
            packet[3] = static_cast<uint8_t>(msg_type); // message_id

            // Copy payload
            if (!payload.empty())
            {
                std::memcpy(packet.data() + HEADER_SIZE, payload.data(), payload.size());
            }

            // Remaining bytes are already zeroed (padding)
            return packet;
        }

        /**
         * @brief Parse a RX packet header
         */
        static PacketHeader parseHeader(const std::vector<uint8_t> &packet)
        {
            if (packet.size() < HEADER_SIZE)
            {
                throw std::runtime_error("Packet too small");
            }

            PacketHeader header;
            header.direction = packet[0];
            header.length = packet[1];
            header.counter = packet[2];
            header.message_id = packet[3];

            if (header.direction != START_BYTE_RX)
            {
                throw std::runtime_error("Invalid direction byte in response");
            }

            return header;
        }

        /**
         * @brief Extract payload from packet
         * Returns raw payload starting after header (4 bytes)
         */
        static std::vector<uint8_t> extractPayload(const std::vector<uint8_t> &packet)
        {
            PacketHeader header = parseHeader(packet);

            // Payload length = total length - 2 (counter + message_id are counted in length)
            size_t payload_length = header.length > 2 ? header.length - 2 : 0;

            if (payload_length == 0)
            {
                return {};
            }

            if (HEADER_SIZE + payload_length > packet.size())
            {
                throw std::runtime_error("Payload length exceeds packet size");
            }

            return std::vector<uint8_t>(
                packet.begin() + HEADER_SIZE,
                packet.begin() + HEADER_SIZE + payload_length);
        }

        /**
         * @brief Validate response matches request
         */
        static bool validateResponse(
            const PacketHeader &response,
            uint8_t expected_counter,
            MessageType expected_type)
        {
            // Special case: READ_FILE_DATA_REQUEST returns READ_FILE_DATA_RESPONSE
            if (expected_type == MessageType::READ_FILE_DATA_REQUEST)
            {
                return response.counter == expected_counter &&
                       response.message_id == static_cast<uint8_t>(MessageType::READ_FILE_DATA_RESPONSE);
            }

            return response.counter == expected_counter &&
                   response.message_id == static_cast<uint8_t>(expected_type);
        }
    };

}
