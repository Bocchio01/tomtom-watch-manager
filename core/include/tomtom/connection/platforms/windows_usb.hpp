#pragma once

#ifdef _WIN32

#include <windows.h>
#include <setupapi.h>
#include <winusb.h>
#include <vector>
#include <string>

#include "tomtom/connection/usb_connection.hpp"

namespace tomtom
{
    namespace platforms
    {
        struct HidCaps
        {
            USHORT input_len = 0;
            USHORT output_len = 0;
            USHORT feature_len = 0;
        };

        /**
         * @brief Windows-specific USB implementation using WinUSB.
         */
        class WindowsUSBImpl
        {
        private:
            HANDLE device_handle = INVALID_HANDLE_VALUE;
            WINUSB_INTERFACE_HANDLE usb_handle = nullptr;
            DeviceInfo device_info;
            HidCaps capabilities;
            bool is_open = false;

        public:
            explicit WindowsUSBImpl(const DeviceInfo &info);
            ~WindowsUSBImpl();

            bool open();
            void close();
            bool isOpen() const;
            int read(uint8_t *buffer, size_t size, int timeout_ms);
            int write(const uint8_t *buffer, size_t size, int timeout_ms);
            std::string getDevicePath() const;

            static std::vector<DeviceInfo> enumerateDevices();
        };

    } // namespace platform
} // namespace tomtom

#endif // _WIN32