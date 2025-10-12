#include <windows.h>
#include <spdlog/spdlog.h>

#include "tomtom/defines.hpp"
#include "tomtom/packets.hpp"
#include "tomtom/watch.hpp"

namespace tomtom
{
    Watch::Watch(std::unique_ptr<USBConnection> conn) : connection(std::move(conn))
    {
        if (!connection)
        {
            throw std::invalid_argument("Connection cannot be null");
        }

        if (!connection->open())
        {
            throw std::runtime_error("Failed to open connection to the watch");
        }

        const auto &devInfo = connection->getDeviceInfo();
        info.product_id = devInfo.product_id;
        info.serial_number = devInfo.serial_number;
        info.manufacturer = devInfo.manufacturer;
        info.product_name = devInfo.product_name;

        spdlog::info("Connected to watch: {} (Product ID: 0x{:04X}, Serial: {})", info.product_name, info.product_id, info.serial_number);
    }

    Watch::~Watch()
    {
        if (connection && connection->isOpen())
        {
            connection->close();
        }
    }

    Watch::Watch(Watch &&other) noexcept
        : connection(std::move(other.connection)),
          info(std::move(other.info))
    {
    }

    Watch &Watch::operator=(Watch &&other) noexcept
    {
        if (this != &other)
        {
            connection = std::move(other.connection);
            info = std::move(other.info);
        }
        return *this;
    }

    WatchError Watch::startUp()
    {
        spdlog::debug("Starting up watch with Product ID: 0x{:04X}", info.product_id);

        sendPacket(MSG_UNKNOWN_0D, 0, 0, 20, 0);
        sendPacket(MSG_FIND_CLOSE, 0, 0, 0, 0);
        sendPacket(MSG_UNKNOWN_22, 0, 0, 1, 0);
        sendPacket(MSG_UNKNOWN_22, 0, 0, 1, 0);
        sendPacket(MSG_GET_PRODUCT_ID, 0, 0, 4, 0);
        sendPacket(MSG_FIND_CLOSE, 0, 0, 0, 0);
        sendPacket(MSG_GET_BLE_VERSION, 0, 0, 4, 0);
        sendPacket(MSG_GET_BLE_VERSION, 0, 0, 4, 0);
        sendPacket(MSG_UNKNOWN_1F, 0, 0, 4, 0);

        return WatchError::NoError;
    }

    // And update your Watch::sendPacket to NOT wrap in hid_packet:
    WatchError Watch::sendPacket(
        uint8_t msg,
        uint8_t tx_length,
        const uint8_t *tx_data,
        uint8_t rx_length,
        uint8_t *rx_data)
    {
        static uint8_t message_counter = 0;
        uint8_t packet[256] = {0};

        // Create the tx packet
        packet[0] = 0x09;
        packet[1] = tx_length + 2;
        packet[2] = message_counter++;
        packet[3] = msg;
        memcpy(packet + 4, tx_data, tx_length);

        uint16_t packet_size;
        if (info.product_id == TOMTOM_MULTISPORT_PRODUCT_ID)
        {
            packet_size = tx_length + 4;
        }
        else if (IS_SPARK(info.product_id))
        {
            packet_size = 256;
        }
        else
            return WatchError::UnableToSendPacket;

        spdlog::debug("Sending packet: msg=0x{:02X}, tx_len={}, packet_size={}",
                      msg, tx_length, packet_size);

        // Send the packet DIRECTLY - no HID wrapping needed!
        if (connection->write(packet, packet_size, 5000) < 0)
        {
            return WatchError::UnableToSendPacket;
        }

        // Read response DIRECTLY - no HID unwrapping needed!
        std::vector<uint8_t> response(256, 0);

        int bytes_read = connection->read(response.data(), 256, 20000);
        if (bytes_read < 0)
        {
            return WatchError::UnableToReceivePacket;
        }

        spdlog::debug("Received {} bytes", bytes_read);

        // The response IS the packet - no offset needed!
        memcpy(packet, response.data(), bytes_read);

        // Validation
        if (packet[0] != 0x01)
            return WatchError::InvalidResponse;
        if ((rx_length < 60) && (packet[1] != (rx_length + 2)))
            return WatchError::IncorrectResponseLength;
        if (packet[2] != (uint8_t)(message_counter - 1))
            return WatchError::OutOfSyncResponse;

        if (msg == MSG_READ_FILE_DATA_REQUEST)
        {
            if (packet[3] != MSG_READ_FILE_DATA_RESPONSE)
                return WatchError::UnexpectedResponse;
        }
        else
        {
            if (packet[3] != msg)
                return WatchError::UnexpectedResponse;
        }

        if (rx_data)
            memcpy(rx_data, packet + 4, packet[1] - 2);

        return WatchError::NoError;
    }

    WatchError Watch::getCurrentTime(std::time_t &time)
    {
        // Validate watch state
        // if (is_file_open)
        // {
        //     spdlog::error("Cannot get time while file operation is in progress");
        //     return WatchError::FileOpen;
        // }

        RXGetCurrentTimePacket response = {0};

        // Send packet and get response
        WatchError result = sendPacket(
            MSG_GET_CURRENT_TIME,
            0,                                     // tx_length
            nullptr,                               // tx_data
            sizeof(response),                      // rx_length
            reinterpret_cast<uint8_t *>(&response) // rx_data
        );

        if (result != WatchError::NoError)
        {
            spdlog::error("Failed to get current time from watch");
            return result;
        }

        // Convert big-endian to host byte order
        time = static_cast<std::time_t>(TT_BIGENDIAN(response.utc_time));

        spdlog::debug("Watch time retrieved: {}", time);

        return WatchError::NoError;
    }

}