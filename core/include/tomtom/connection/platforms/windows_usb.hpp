#pragma once

#ifdef _WIN32

#include <windows.h>
#include <setupapi.h>
#include <winusb.h>

namespace tomtom
{
    namespace platforms
    {

        /**
         * @brief Windows-specific USB implementation using WinUSB.
         */
        class WindowsUSBImpl
        {
        private:
            HANDLE device_handle = INVALID_HANDLE_VALUE;
            WINUSB_INTERFACE_HANDLE usb_handle = nullptr;
            DeviceInfo device_info;
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