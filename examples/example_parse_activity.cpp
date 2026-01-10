#include <iostream>
#include <iomanip>
#include <fstream>
#include <spdlog/spdlog.h>

#include "tomtom/manager.hpp"
#include "tomtom/services/file_ids.hpp"

using namespace tomtom;

int main(int argc, char *argv[])
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

        for (const auto &info : activities)
        {
            std::cout << "-------------------------------------------\n";
            std::cout << "Activity Index: " << info.index << "\n";
            std::cout << "File ID      : 0x" << std::hex << std::uppercase
                      << std::setw(8) << std::setfill('0') << info.file_id.value << std::dec << "\n";
            std::cout << "Type         : " << static_cast<int>(info.type) << "\n";
            std::cout << "Start Time   : " << std::asctime(std::gmtime(&info.start_time));
            std::cout << "Duration     : " << info.getDurationString() << "\n";
            std::cout << "Distance     : " << info.getDistanceKm() << " km\n";
            std::cout << "File Size    : " << info.file_size << " bytes\n";

            // Load full activity data
            auto activity = watch->activities().get(info.index);
            std::cout << "  Loaded activity with " << activity.records.size() << " records.\n";

            auto gpx_data = watch->activities().exportToGPX(activity);
            std::string gpx_filename = "activity_" + std::to_string(info.index) + ".gpx";
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
