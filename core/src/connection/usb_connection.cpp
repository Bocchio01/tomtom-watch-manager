#include "tomtom/connection/usb_connection.hpp"

#ifdef __linux__
#include "tomtom/connection/platforms/linux_usb.hpp"
using PlatformImpl = tomtom::platforms::LinuxUSBImpl;
#elif _WIN32
#include "tomtom/connection/platforms/windows_usb.hpp"
using PlatformImpl = tomtom::platforms::WindowsUSBImpl;
#else
#error "Unsupported platform"
#endif

namespace tomtom
{

    // PImpl implementation
    class USBConnection::Impl
    {
    public:
        PlatformImpl platform_impl;

        explicit Impl(const DeviceInfo &info) : platform_impl(info) {}
    };

    USBConnection::USBConnection(const DeviceInfo &device_info) : pImpl(std::make_unique<Impl>(device_info)) {}
    USBConnection::~USBConnection() = default;

    USBConnection::USBConnection(USBConnection &&) noexcept = default;
    USBConnection &USBConnection::operator=(USBConnection &&) noexcept = default;

    bool USBConnection::open()
    {
        return pImpl->platform_impl.open();
    }

    void USBConnection::close()
    {
        pImpl->platform_impl.close();
    }

    bool USBConnection::isOpen() const
    {
        return pImpl->platform_impl.isOpen();
    }

    int USBConnection::read(uint8_t *buffer, size_t size, int timeout_ms)
    {
        return pImpl->platform_impl.read(buffer, size, timeout_ms);
    }

    int USBConnection::write(const uint8_t *buffer, size_t size, int timeout_ms)
    {
        return pImpl->platform_impl.write(buffer, size, timeout_ms);
    }

    std::vector<DeviceInfo> USBConnection::enumerateDevices()
    {
        return PlatformImpl::enumerateDevices();
    }

    const DeviceInfo &USBConnection::getDeviceInfo() const
    {
        return pImpl->platform_impl.getDeviceInfo();
    }

} // namespace tomtom