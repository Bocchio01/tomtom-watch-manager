#include <iostream>
#include <spdlog/spdlog.h>
#include "tomtom/manager.hpp"

int main()
{
    spdlog::set_level(spdlog::level::info);

    auto watches = tomtom::Manager::enumerate();

    for (auto &watch : watches)
    {
        watch->connection->open();
        if (watch->connection->isOpen())
        {
            std::time_t current_time;
            tomtom::WatchError err = watch->getCurrentTime(current_time);
            if (err != tomtom::WatchError::NoError)
            {
                spdlog::error("Error getting current time: {}", static_cast<int>(err));
                continue;
            }
            char buf[80];
            struct tm *timeinfo = localtime(&current_time);
            strftime(buf, sizeof(buf), "%A, %B %d, %Y %H:%M:%S", timeinfo);
            spdlog::info("Current watch time: {}", buf);
            watch->connection->close();
        }
        else
        {
            spdlog::critical("Failed to open watch.\n");
            return 1;
        }
    }

    return 0;
}