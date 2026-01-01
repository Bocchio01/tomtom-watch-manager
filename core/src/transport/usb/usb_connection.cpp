#include "tomtom/transport/device_info.hpp"
#include "tomtom/transport/usb/usb_connection.hpp"
#include "tomtom/transport/usb/usb_connection_impl.hpp"

namespace tomtom::transport
{
    USBDeviceConnection::USBDeviceConnection(const DeviceInfo &info)
        : impl_(std::make_unique<Impl>(info))
    {
    }

    USBDeviceConnection::~USBDeviceConnection() = default;

    bool USBDeviceConnection::open()
    {
        return impl_->open();
    }

    void USBDeviceConnection::close()
    {
        impl_->close();
    }

    bool USBDeviceConnection::isOpen() const
    {
        return impl_->isOpen();
    }

    int USBDeviceConnection::read(uint8_t *buffer, size_t size, int timeout_ms)
    {
        return impl_->read(buffer, size, timeout_ms);
    }

    int USBDeviceConnection::write(const uint8_t *buffer, size_t size, int timeout_ms)
    {
        return impl_->write(buffer, size, timeout_ms);
    }

    const DeviceInfo &USBDeviceConnection::deviceInfo() const
    {
        return impl_->deviceInfo();
    }

    std::vector<DeviceInfo> USBDeviceConnection::enumerate()
    {
        return Impl::enumerate();
    }
}
