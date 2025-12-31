#include "tomtom/connection/device_connection.hpp"
#include "tomtom/connection/usb/usb_device_connection.hpp"
// #include "tomtom/connection/ble/ble_device_connection.hpp"

namespace tomtom
{
    std::vector<DeviceInfo> DeviceConnection::enumerate()
    {
        std::vector<DeviceInfo> all_devices;

        // Enumerate USB devices
        auto usb_devices = USBDeviceConnection::enumerate();
        all_devices.insert(all_devices.end(), usb_devices.begin(), usb_devices.end());

        // TODO: Enumerate BLE devices when BLE support is added
        // auto ble_devices = BLEDeviceConnection::enumerate();
        // all_devices.insert(all_devices.end(), ble_devices.begin(), ble_devices.end());

        return all_devices;
    }
}
