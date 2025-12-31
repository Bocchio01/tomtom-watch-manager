#include "tomtom/connection/connection_factory.hpp"

#include "tomtom/connection/usb/usb_connection.hpp"
// #include "tomtom/connection/ble/ble_connection.hpp"

namespace tomtom::connection
{
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
