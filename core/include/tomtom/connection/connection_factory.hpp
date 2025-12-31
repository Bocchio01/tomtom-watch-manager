#pragma once

#include <memory>
#include "device_info.hpp"
#include "connection.hpp"

namespace tomtom::connection
{
    class DeviceConnectionFactory
    {
    public:
        static std::unique_ptr<DeviceConnection> create(const DeviceInfo &info);
    };
}
