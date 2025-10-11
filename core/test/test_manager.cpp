#include <iostream>
#include "tomtom/manager.hpp"

int main()
{
    std::cout << "Enumerating connected TomTom watches...\n";

    auto watches = tomtom::Manager::enumerate();

    std::cout << "Found " << watches.size() << " devices.\n";

    for (auto &watch : watches)
    {
        if (watch->connection->isOpen())
        {
            std::cout << "Watch connection opened successfully.\n";
            watch->connection->close();
        }
        else
        {
            std::cout << "Failed to open watch.\n";
        }
    }

    std::cout << "All done!\n";
    return 0;
}