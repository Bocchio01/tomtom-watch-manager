#include <vector>
#include <memory>
#include <cstring>
#include <iostream>

#include "tomtom/watch.hpp"
#include "tomtom/manager.hpp"
#include "tomtom/connection/usb_connection.hpp"

namespace tomtom
{
    std::vector<std::shared_ptr<Watch>> Manager::enumerate()
    {
        std::vector<std::shared_ptr<Watch>> watches;

        auto devices = USBConnection::enumerateDevices();

        for (const auto &device : devices)
        {
            auto connection = std::make_unique<USBConnection>(device);
            auto watch = std::make_shared<Watch>(std::move(connection));
            watches.push_back(std::move(watch));
        }

        return watches;
    }

}
