#pragma once
#ifdef __linux__

#include <libusb-1.0/libusb.h>

namespace tomtom
{
    namespace platforms
    {

        /**
         * @brief Linux-specific USB implementation using libusb.
         */
        class LinuxUSBImpl
        {
        private:
            libusb_context *context = nullptr;
            libusb_device_handle *handle = nullptr;
            DeviceInfo device_info;
            bool is_open = false;

        public:
            explicit LinuxUSBImpl(const DeviceInfo &info);
            ~LinuxUSBImpl();

            bool open();
            void close();
            bool isOpen() const { return is_open; }
            int read(uint8_t *buffer, size_t size, int timeout_ms);
            int write(const uint8_t *buffer, size_t size, int timeout_ms);
            std::string getDevicePath() const { return device_info.device_path; }

            static std::vector<DeviceInfo> enumerateDevices();
        };

    } // namespace platform
} // namespace tomtom

#endif // __linux__