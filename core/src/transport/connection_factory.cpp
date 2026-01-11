#include "tomtom/core/transport/device_info.hpp"
#include "tomtom/core/transport/connection_factory.hpp"

#include "tomtom/core/transport/usb/usb_connection.hpp"
// #include "tomtom/core/transport/ble/ble_connection.hpp"

namespace tomtom::core::transport
{
    std::vector<DeviceInfo> DeviceConnectionFactory::enumerate()
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

    std::unique_ptr<DeviceConnection> DeviceConnectionFactory::create(const DeviceInfo &info)
    {
        switch (info.transport)
        {
        case TransportType::USB:
            return std::make_unique<USBDeviceConnection>(info);

        case TransportType::BLE:
            //     return std::make_unique<BLEDeviceConnection>(info);
            return nullptr;
        }

        return nullptr;
    }
}
