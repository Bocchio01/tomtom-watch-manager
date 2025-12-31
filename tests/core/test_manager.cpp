#include <iostream>
#include <spdlog/spdlog.h>

#include "tomtom/manager.hpp"

int main()
{
    spdlog::set_level(spdlog::level::info);

    tomtom::Manager manager;

    auto watches_info = manager.detectWatches();

    if (watches_info.empty())
    {
        spdlog::error("No watch connected.\n");
        return 1;
    }

    spdlog::info("Detected {} watch(es):", watches_info.size());
    for (size_t i = 0; i < watches_info.size(); ++i)
    {
        auto watch = manager.connectToWatch(i);
        if (watch)
        {
            spdlog::info("Connected to watch {}: {} - {}", i,
                         watch->getManufacturer(),
                         watch->getProductName());
        }
        else
        {
            spdlog::error("Failed to connect to watch {}\n", i);
            continue;
        }
    }

    return 0;
}