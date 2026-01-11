#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "device_info.hpp"

namespace tomtom::core::transport
{
    /**
     * @brief Abstract transport connection.
     */
    class DeviceConnection
    {
    public:
        virtual ~DeviceConnection() = default;

        DeviceConnection(const DeviceConnection &) = delete;
        DeviceConnection &operator=(const DeviceConnection &) = delete;

        DeviceConnection(DeviceConnection &&) = default;
        DeviceConnection &operator=(DeviceConnection &&) = default;

        virtual bool open() = 0;
        virtual void close() = 0;
        virtual bool isOpen() const = 0;

        virtual int read(uint8_t *buffer, size_t size, int timeout_ms) = 0;
        virtual int write(const uint8_t *buffer, size_t size, int timeout_ms) = 0;

        virtual const DeviceInfo &deviceInfo() const = 0;

    protected:
        DeviceConnection() = default;
    };
}
