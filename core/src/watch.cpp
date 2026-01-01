#include <spdlog/spdlog.h>

#include "tomtom/defines.hpp"
#include "tomtom/watch.hpp"
#include "tomtom/protocol/definition/protocol.hpp"

namespace tomtom
{
    Watch::Watch(std::shared_ptr<transport::DeviceConnection> conn) : connection(std::move(conn))
    {
        if (!connection)
        {
            throw std::invalid_argument("Connection cannot be null");
        }

        if (!connection->open())
        {
            throw std::runtime_error("Failed to open connection to the watch");
        }

        packet_handler_ = std::make_unique<protocol::runtime::PacketHandler>(connection);

        spdlog::info("Connected to watch: {} (Product ID: 0x{:04X}, Serial: {})",
                     getProductName(), getProductId(), getSerialNumber());
    }

    Watch::~Watch()
    {
        if (connection && connection->isOpen())
        {
            connection->close();
        }
    }

    Watch::Watch(Watch &&other) noexcept : connection(std::move(other.connection))
    {
    }

    Watch &Watch::operator=(Watch &&other) noexcept
    {
        if (this != &other)
        {
            connection = std::move(other.connection);
        }
        return *this;
    }

    std::time_t Watch::getTime()
    {
        spdlog::debug("Requesting time from watch...");

        // 1. Create the request packet
        protocol::definition::GetWatchTimeTx request;

        // 2. Execute the transaction
        auto response = packet_handler_->transaction<protocol::definition::GetWatchTimeTx, protocol::definition::GetWatchTimeRx>(request);

        // 3. Process the data
        uint32_t raw_time = response.payload.time;

        // Convert big-endian to host-endian
        std::time_t t = static_cast<std::time_t>(TT_BIGENDIAN(raw_time));

        // Optional: Print it immediately like your old function did
        spdlog::info("Watch Time: {}", std::asctime(std::gmtime(&t)));

        return t;
    }
}