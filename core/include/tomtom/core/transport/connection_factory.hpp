#pragma once

#include <memory>
#include "device_info.hpp"
#include "connection.hpp"

namespace tomtom::core::transport
{
    class DeviceConnectionFactory
    {
    public:
        static std::vector<DeviceInfo> enumerate();
        static std::unique_ptr<DeviceConnection> create(const DeviceInfo &info);
    };
}
