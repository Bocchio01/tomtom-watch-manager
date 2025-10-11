#include "tomtom/watch.hpp"

namespace tomtom
{
    Watch::Watch(std::unique_ptr<USBConnection> connection) : connection(std::move(connection)) {}
    Watch::~Watch()
    {
        if (connection && connection->isOpen())
        {
            connection->close();
        }
    }

    Watch::Watch(Watch &&other) noexcept
        : product_id(other.product_id),
          firmware_version(other.firmware_version),
          ble_version(other.ble_version),
          serial_number(std::move(other.serial_number)),
          connection(std::move(other.connection))
    {
        other.product_id = 0;
        other.firmware_version = 0;
        other.ble_version = 0;
        other.serial_number.clear();
    }

    Watch &Watch::operator=(Watch &&other) noexcept
    {
        if (this != &other)
        {
            product_id = other.product_id;
            firmware_version = other.firmware_version;
            ble_version = other.ble_version;
            serial_number = std::move(other.serial_number);
            connection = std::move(other.connection);

            other.product_id = 0;
            other.firmware_version = 0;
            other.ble_version = 0;
            other.serial_number.clear();
        }
        return *this;
    }

}