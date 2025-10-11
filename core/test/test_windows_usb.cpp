#include <iostream>
#include "tomtom/connection/platforms/windows_usb.hpp"

int main()
{
    auto devices = tomtom::platforms::WindowsUSBImpl::enumerateDevices();
    if (!devices.empty())
    {
        tomtom::platforms::WindowsUSBImpl usb(devices[0]);
        if (usb.open())
        {
            uint8_t buffer[64];
            int bytesRead = usb.read(buffer, sizeof(buffer), 1000);
            if (bytesRead > 0)
            {
                std::cout << "Read " << bytesRead << " bytes from device." << std::endl;
            }
            usb.close();
        }
    }
    return 0;
}
