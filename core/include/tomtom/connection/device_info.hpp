#pragma once

#include <string>
#include <cstdint>
#include <variant>

namespace tomtom::connection
{
    /**
     * @brief Transport type enumeration.
     */
    enum class TransportType
    {
        USB,
        BLE,
    };

    /**
     * @brief BLE-specific device information.
     */
    struct BLEDeviceDetails
    {
        uint8_t bus_number;
        uint8_t device_address;
    };

    /**
     * @brief USB-specific device information.
     */
    struct USBDeviceDetails
    {
        std::string device_path; // OS-specific
    };

    /**
     * @brief Transport-agnostic device identity.
     */
    struct DeviceInfo
    {
        std::string manufacturer;  // e.g. "TomTom Fitness"
        std::string product_name;  // e.g. "Runner"
        std::string serial_number; // e.g. "HC4354G00150"
        uint16_t vendor_id;        // e.g. 0x0483
        uint16_t product_id;       // e.g. 0x5750
        TransportType transport;   // e.g. USB, BLE

        // Type-safe union using std::variant
        std::variant<BLEDeviceDetails, USBDeviceDetails> details;
    };
}
