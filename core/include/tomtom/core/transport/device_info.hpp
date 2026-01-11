#pragma once

#include <string>
#include <cstdint>
#include <variant>

namespace tomtom::core::transport
{
    /**
     * @brief Vendor ID enumeration.
     */
    enum class VendorID : uint16_t
    {
        TOMTOM = 0x1390,
    };

    /**
     * @brief Product ID enumeration.
     */
    enum class ProductID : uint16_t
    {
        MULTISPORT = 0x7474,
        SPARK_MUSIC = 0x7475,
        SPARK_CARDIO = 0x7477,
        TOUCH = 0x7480,
    };

    /**
     * @brief Transport type enumeration.
     */
    enum class TransportType : uint8_t
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
        VendorID vendor_id;
        ProductID product_id;
        std::string product_name;  // e.g. "Runner"
        std::string manufacturer;  // e.g. "TomTom Fitness"
        std::string serial_number; // e.g. "HC4354G00150"
        TransportType transport;
        std::variant<BLEDeviceDetails, USBDeviceDetails> details;
    };
}
