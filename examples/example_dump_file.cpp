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
    std::cout << "TomTom Watch Manager: Dump File Example\n";
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

        // Get file ID from command line or use default
        services::FileId test_file_id(services::PREFERENCES);
        if (argc > 1)
        {
            test_file_id.value = std::stoul(argv[1], nullptr, 0);
        }

        // Read file data
        auto data = watch->files().readFile(test_file_id);
        std::cout << "Read " << data.size() << " bytes from file ID 0x"
                  << std::hex << std::uppercase << std::setw(8) << std::setfill('0')
                  << test_file_id.value << "\n";

        // Print the file data (hex + ASCII dump)
        std::ostringstream oss_hex;
        std::ostringstream oss_ascii;

        for (size_t i = 0; i < data.size(); ++i)
        {
            // Hex dump
            oss_hex << std::hex << std::uppercase << std::setw(2)
                    << std::setfill('0') << static_cast<unsigned>(data[i]) << ' ';

            // ASCII dump (printable characters or '.')
            if (std::isprint(data[i]) || data[i] == '\n' || data[i] == '\r' || data[i] == '\t')
                oss_ascii << static_cast<char>(data[i]);
            else
                oss_ascii << '.';
        }

        // Log both
        std::cout << "HEX  : " << std::endl
                  << oss_hex.str() << std::endl;
        std::cout << "ASCII: " << std::endl
                  << oss_ascii.str() << std::endl;

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "\nError: " << e.what() << "\n";
        return 1;
    }
}
