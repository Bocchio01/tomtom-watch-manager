#include <iostream>
#include <spdlog/spdlog.h>
#include "tomtom/manager.hpp"

int main()
{
    spdlog::set_level(spdlog::level::debug);

    auto watches = tomtom::Manager::enumerate();
    std::cout << "Found " << watches.size() << " devices.\n";

    for (auto &watch : watches)
    {
        watch->connection->open();
        if (watch->connection->isOpen())
        {
            std::cout << "Watch connection opened successfully.\n";
            watch->connection->close();
        }
        else
        {
            std::cout << "Failed to open watch.\n";
            return 1;
        }
    }

    return 0;
}