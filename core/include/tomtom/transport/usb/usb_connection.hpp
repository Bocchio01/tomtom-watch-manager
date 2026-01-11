#pragma once
#include <memory>
#include <vector>

#include "../connection.hpp"
#include "../device_info.hpp"

namespace tomtom::transport
{
    /**
     * @brief USB transport implementation.
     */
    class USBDeviceConnection final : public DeviceConnection
    {
    public:
        explicit USBDeviceConnection(const DeviceInfo &info);
        ~USBDeviceConnection();

        bool open() override;
        void close() override;
        bool isOpen() const override;

        int read(uint8_t *buffer, size_t size, int timeout_ms) override;
        int write(const uint8_t *buffer, size_t size, int timeout_ms) override;

        const DeviceInfo &deviceInfo() const override;
        static std::vector<DeviceInfo> enumerate();

    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };
}
