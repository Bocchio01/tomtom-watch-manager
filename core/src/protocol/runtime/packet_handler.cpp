#include "tomtom/protocol/runtime/packet_handler.hpp"

namespace tomtom::protocol::runtime
{
    // Default timeout for operations
    static constexpr int DEFAULT_TIMEOUT_MS = 2000;

    PacketHandler::PacketHandler(std::shared_ptr<transport::DeviceConnection> connection)
        : connection_(std::move(connection))
    {
        if (!connection_)
        {
            throw std::invalid_argument("PacketHandler requires a valid connection");
        }
    }

    void PacketHandler::writeBytes(const uint8_t *data, size_t size)
    {
        size_t total_written = 0;

        // Retry loop could be added here, but for USB HID typically one write call is a packet
        int result = connection_->write(data, size, DEFAULT_TIMEOUT_MS);

        if (result < 0)
        {
            throw ConnectionError("Failed to write to device (IO Error)");
        }

        if (static_cast<size_t>(result) != size)
        {
            // This is critical; partial writes on packet boundaries usually mean failure
            throw ConnectionError("Incomplete write to device");
        }
    }

    void PacketHandler::readBytes(uint8_t *buffer, size_t size)
    {
        size_t total_read = 0;
        int retries = 3;

        while (total_read < size && retries > 0)
        {
            int requested = static_cast<int>(size - total_read);
            int result = connection_->read(buffer + total_read, requested, DEFAULT_TIMEOUT_MS);

            if (result < 0)
            {
                throw ConnectionError("Failed to read from device");
            }

            if (result == 0)
            {
                // Timeout logic inside connection might return 0
                retries--;
                continue;
            }

            total_read += result;
        }

        if (total_read < size)
        {
            throw TimeoutError("Timed out waiting for data from device");
        }
    }
}