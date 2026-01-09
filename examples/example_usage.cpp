// ============================================================================
// example_domain_usage.cpp - Demonstrates domain-based file architecture
// ============================================================================

#include <spdlog/spdlog.h>
#include <tomtom/manager.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace tomtom;
using namespace tomtom::files;

int main(int argc, char *argv[])
{
    spdlog::set_level(spdlog::level::info);

    std::cout << "===========================================\n";
    std::cout << "TomTom Watch Manager\n";
    std::cout << "===========================================\n";

    try
    {
        // Connect to watch
        Manager manager;
        std::cout << "\nSearching for TomTom watch...\n";

        auto watch = manager.connectToWatch();

        std::cout << "Connected to watch:\n";
        std::cout << "  Product: " << watch->getProductName() << "\n";
        std::cout << "  Serial: " << watch->getSerialNumber() << "\n";
        std::cout << "  Name: " << watch->preferences().getWatchName() << "\n";

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "\n❌ Error: " << e.what() << "\n";
        return 1;
    }
}
