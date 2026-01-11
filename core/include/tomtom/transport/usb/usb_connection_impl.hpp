#pragma once

#include <cstdint>
#include <vector>
#include "../device_info.hpp"
#include "usb_connection.hpp"

namespace tomtom::transport
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

        // Public members for callback access (macOS)
        std::vector<uint8_t> cached_report_;
        size_t cached_offset_ = 0;
        bool has_pending_data_ = false;

    private:
        DeviceInfo device_info_;
        bool is_open_ = false;

#ifdef _WIN32
        // Windows specific members
        void *device_handle_ = (void *)-1; // INVALID_HANDLE_VALUE
        uint16_t input_report_len_ = 0;
        uint16_t output_report_len_ = 0;

#elif __APPLE__
        // macOS specific members
        void *device_handle_ = nullptr; // IOHIDDeviceRef
        void *hid_manager_ = nullptr;   // IOHIDManagerRef
        uint16_t input_report_len_ = 0;
        uint16_t output_report_len_ = 0;

#else
        // Unix/Linux specific members (libusb)
        void *device_handle_ = nullptr;  // libusb_device_handle*
        uint8_t input_endpoint_ = 0x81;  // Default IN endpoint
        uint8_t output_endpoint_ = 0x01; // Default OUT endpoint
        uint16_t input_report_len_ = 64;
        uint16_t output_report_len_ = 64;
        bool attach_kernel_driver_ = false;

#endif
    };
}