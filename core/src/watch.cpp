#include <spdlog/spdlog.h>

#include "tomtom/defines.hpp"
#include "tomtom/watch.hpp"
#include "tomtom/protocol/protocol_types.hpp"

namespace tomtom
{
    Watch::Watch(std::shared_ptr<DeviceConnection> conn) : connection(std::move(conn))
    {
        if (!connection)
        {
            throw std::invalid_argument("Connection cannot be null");
        }

        if (!connection->open())
        {
            throw std::runtime_error("Failed to open connection to the watch");
        }

        const auto &devInfo = connection->deviceInfo();
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
        : info(std::move(other.info)),
          connection(std::move(other.connection))
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
}