#pragma once

#include <memory>
#include "device_info.hpp"
#include "device_connection.hpp"

namespace tomtom
{
    class DeviceConnectionFactory
    {
    public:
        static std::unique_ptr<DeviceConnection> create(const DeviceInfo &info);
    };
}
