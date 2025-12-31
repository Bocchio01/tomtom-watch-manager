#pragma once

#include <cstdint>
#include <vector>
#include "../device_info.hpp"
#include "usb_device_connection.hpp"

namespace tomtom
{
    /**
     * @brief Private implementation of USBDeviceConnection.
     */
    class USBDeviceConnection::Impl
    {
    public:
        explicit Impl(const DeviceInfo &info);
        ~Impl();

        // Non-copyable
        Impl(const Impl &) = delete;
        Impl &operator=(const Impl &) = delete;

        bool open();
        void close();
        bool isOpen() const;

        int read(uint8_t *buffer, size_t size, int timeout_ms);
        int write(const uint8_t *buffer, size_t size, int timeout_ms);

        const DeviceInfo &deviceInfo() const;
        static std::vector<DeviceInfo> enumerate();

    private:
        DeviceInfo device_info_;
        bool is_open_ = false;

#ifdef _WIN32
        // Windows specific members
        // stored as void* to avoid including windows.h in the header
        void *device_handle_ = (void *)-1; // INVALID_HANDLE_VALUE
        uint16_t input_report_len_ = 0;
        uint16_t output_report_len_ = 0;
#endif
    };
}