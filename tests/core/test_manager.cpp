#include <iostream>
#include <spdlog/spdlog.h>
#include "tomtom/manager.hpp"

int main()
{
    spdlog::set_level(spdlog::level::info);

    tomtom::Manager manager;

    auto watch = manager.connectToWatch();
    if (!watch)
    {
        spdlog::error("No watch connected.\n");
        return 1;
    }

    try
    {
        std::time_t current_time;
        tomtom::WatchError err = watch->getCurrentTime(current_time);
        if (err != tomtom::WatchError::NoError)
        {
            spdlog::error("Error getting current time: {}", static_cast<int>(err));
        }
        char buf[80];
        struct tm *timeinfo = localtime(&current_time);
        strftime(buf, sizeof(buf), "%A, %B %d, %Y %H:%M:%S", timeinfo);
        spdlog::info("Current watch time: {}", buf);
        watch->connection->close();
    }
    catch (const std::exception &e)
    {
        spdlog::critical(e.what());
        return 1;
    }

    return 0;
}