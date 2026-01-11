#include <iostream>
#include <spdlog/spdlog.h>

#include "tomtom/manager.hpp"
#include "tomtom/sdk/sync_service.hpp"

using namespace tomtom;

int main()
{
    spdlog::set_level(spdlog::level::info);

    std::cout << "===========================================\n";
    std::cout << "TomTom Watch Manager: SDK Demo \n";
    std::cout << "===========================================\n";

    try
    {
        Manager manager;
        auto watch = manager.connectToWatch();
        if (!watch)
        {
            std::cerr << "No TomTom watch found. Please connect a watch and try again.\n";
            return 1;
        }

        std::cout << "Connected to watch: " << watch->getProductName() << "\n";

        // Create SyncService
        sdk::SyncService sync_service(watch, "activities", {sdk::ActivityExporter::Format::GPX});

        // Configure sync operations
        sdk::SyncService::Config sync_config;
        sync_config.sync_time = true;
        sync_config.update_gps = true;
        sync_config.download_activities = true;

        // Run auto-sync with progress callback
        sync_service.runAutoSync(
            sync_config,
            [](const std::string &stage, int progress, const std::string &message)
            {
                std::cout << "[" << stage << "] " << progress << "% - " << message << "\n";
            });

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "\nError: " << e.what() << "\n";
        return 1;
    }
}
