#include <iostream>
#include <iomanip>
#include <fstream>
#include <spdlog/spdlog.h>

#include "tomtom/manager.hpp"
#include "tomtom/services/files/files.hpp"
#include "tomtom/sdk/activity_exporter.hpp"

using namespace tomtom;

int main()
{
    spdlog::set_level(spdlog::level::off);

    std::cout << "===========================================\n";
    std::cout << "TomTom Watch Manager: Parse Activity Example\n";
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

        auto activities = watch->activities().list();
        std::cout << "Found " << activities.size() << " activities on the watch.\n";

        for (size_t i = 0; i < activities.size(); i++)
        {
            const auto &activity = activities[i];

            std::cout << "-------------------------------------------\n";
            std::cout << "Activity Index: " << i << "\n";
            std::cout << "Type         : " << services::activity::toString(activity.type) << "\n";
            std::cout << "Start Time   : " << std::asctime(std::gmtime(&activity.start_time));
            std::cout << "Duration     : " << activity.duration_seconds / 60 << " minutes\n";
            std::cout << "Distance     : " << activity.distance_meters / 1000.0 << " km\n";
            std::cout << "Records      : " << activity.records.size() << "\n";

            // Use the new SDK exporter to convert to GPX
            auto gpx_data = tomtom::sdk::ActivityExporter::exportFile(
                activity,
                sdk::ActivityExporter::Format::GPX);

            std::string gpx_filename = "activity_" + std::to_string(i) + ".gpx";
            std::ofstream gpx_file(gpx_filename);
            gpx_file << gpx_data;
            gpx_file.close();
            std::cout << "  Exported GPX to " << gpx_filename << "\n";
        }

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "\nError: " << e.what() << "\n";
        return 1;
    }
}
