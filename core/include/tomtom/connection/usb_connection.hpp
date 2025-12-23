#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace tomtom
{
    /**
     * @brief Device information structure.
     */
    struct USBDeviceInfo
    {
        uint16_t vendor_id;
        uint16_t product_id;
        std::string serial_number;
        std::string manufacturer;
        std::string product_name;
        std::string device_path;
    };

    /**
     * @brief Platform-independent USB connection wrapper.
     *
     * This class uses the PImpl pattern to hide platform-specific details.
     */
    class USBConnection
    {
    private:
        class Impl;
        std::unique_ptr<Impl> pImpl;

    public:
        /**
         * @brief Constructs a USB connection for the given device.
         * @param device_info Information about the device to connect to.
         */
        explicit USBConnection(const USBDeviceInfo &device_info);

        ~USBConnection();

        // Non-copyable
        USBConnection(const USBConnection &) = delete;
        USBConnection &operator=(const USBConnection &) = delete;

        // Movable
        USBConnection(USBConnection &&) noexcept;
        USBConnection &operator=(USBConnection &&) noexcept;

        bool open();
        void close();
        bool isOpen() const;
        int read(uint8_t *buffer, size_t size, int timeout_ms);
        int write(const uint8_t *buffer, size_t size, int timeout_ms);

        /**
         * @brief Enumerates all connected TomTom devices.
         * @return Vector of device information structures.
         */
        static std::vector<USBDeviceInfo> enumerateDevices();

        /**
         * @brief Gets the device information for this connection.
         * @return Reference to the DeviceInfo structure.
         */
        const USBDeviceInfo &getDeviceInfo() const;
    };

} // namespace tomtom