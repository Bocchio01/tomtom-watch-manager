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

        packet_handler_ = std::make_shared<protocol::runtime::PacketHandler>(connection);
        file_service_ = std::make_unique<protocol::services::FileService>(packet_handler_);
        info_service_ = std::make_unique<protocol::services::WatchInfoService>(packet_handler_);
        control_service_ = std::make_unique<protocol::services::WatchControlService>(packet_handler_);

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
}