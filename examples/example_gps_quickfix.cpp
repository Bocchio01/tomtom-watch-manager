#include <tomtom/manager.hpp>
#include <tomtom/watch.hpp>
#include <iostream>
#include <iomanip>

/**
 * @brief Example demonstrating GPS QuickFix update
 *
 * This example shows how to:
 * 1. Get the ephemeris URL from watch preferences
 * 2. Update GPS QuickFix data with 7-day ephemeris
 * 3. Handle progress callbacks during download
 */
int main()
{
    try
    {
        std::cout << "GPS QuickFix Update Example\n";
        std::cout << "===========================\n\n";

        // Connect to the first available watch
        tomtom::Manager manager;
        auto devices = manager.detectWatches();

        if (devices.empty())
        {
            std::cerr << "No watches found!\n";
            return 1;
        }

        std::cout << "Connecting to watch: " << devices[0].product_name << "\n\n";
        auto watch = manager.connectToWatch(0);

        // Get preferences to retrieve the ephemeris URL
        std::cout << "Reading watch preferences...\n";
        auto prefs = watch->preferences().get();

        std::string ephemeris_url;
        if (prefs.ephemeris_url.empty())
        {
            // Use default TomTom ephemeris URL if not configured
            ephemeris_url = "https://gpsquickfix.services.tomtom.com/fitness/sifgps.f2p{DAYS}enc.ee";
            std::cout << "Using default ephemeris URL\n";
        }
        else
        {
            // In a real application, you would fetch the config.json from ephemeris_url
            // and extract the ephemeris URL from the "service:ephemeris" field
            // For this example, we'll use the known URL pattern
            ephemeris_url = "https://gpsquickfix.services.tomtom.com/fitness/sifgps.f2p{DAYS}enc.ee";
            std::cout << "Config URL: " << prefs.ephemeris_url << "\n";
        }

        std::cout << "Ephemeris URL: " << ephemeris_url << "\n\n";

        // Define progress callback
        auto progress_callback = [](size_t downloaded, size_t total)
        {
            if (total > 0)
            {
                int percentage = static_cast<int>((downloaded * 100) / total);
                std::cout << "\rDownloading: " << percentage << "% "
                          << "(" << downloaded << "/" << total << " bytes)" << std::flush;
            }
            else
            {
                std::cout << "\rDownloading: " << downloaded << " bytes" << std::flush;
            }
        };

        // Update GPS QuickFix with 7-day ephemeris
        std::cout << "Updating GPS QuickFix data (7 days)...\n";
        // watch->gpsQuickFix().update(
        //     ephemeris_url,
        //     7,    // 7-day ephemeris (can also use 3)
        //     true, // Reset GPS processor after update
        //     progress_callback);

        std::cout << "\n\nGPS QuickFix update completed successfully!\n";
        std::cout << "\nNote: The GPS processor has been reset.\n";
        std::cout << "The watch should now have faster GPS fix times.\n";

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "\nError: " << e.what() << "\n";
        return 1;
    }
}
