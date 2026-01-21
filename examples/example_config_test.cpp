#include <iostream>
#include <filesystem>
#include "tomtom/sdk/filesystem/local_store.hpp"

int main()
{
    std::cout << "Testing LocalStore JSON Configuration...\n\n";

    // Create a temporary config path
    std::filesystem::path testConfigPath = "test_config.json";

    // Clean up if exists
    if (std::filesystem::exists(testConfigPath))
    {
        std::filesystem::remove(testConfigPath);
    }

    try
    {
        // Test 1: Create LocalStore with default config
        std::cout << "1. Creating LocalStore with defaults...\n";
        tomtom::sdk::filesystem::LocalStore store(testConfigPath);

        // Test 2: Verify default values
        const auto &config = store.getConfig();
        std::cout << "   Storage root: " << config.storage_root << "\n";
        std::cout << "   Layout: " << static_cast<int>(config.layout) << "\n";
        std::cout << "   Archive raw: " << (config.archive_raw_data ? "true" : "false") << "\n";
        std::cout << "   Export formats: ";
        for (const auto &fmt : config.export_formats)
        {
            std::cout << fmt << " ";
        }
        std::cout << "\n\n";

        // Test 3: Modify and save config
        std::cout << "2. Modifying configuration...\n";
        auto newConfig = config;
        newConfig.storage_root = std::filesystem::current_path() / "MyTomTomData";
        newConfig.layout = tomtom::sdk::filesystem::DirectoryLayout::BySport;
        newConfig.export_formats = {"gpx", "json", "kml"};
        newConfig.archive_raw_data = false;
        newConfig.log_level = "debug";

        store.setConfig(newConfig);
        store.saveConfig();
        std::cout << "   Configuration saved to: " << testConfigPath << "\n\n";

        // Test 4: Read the saved JSON file
        std::cout << "3. Displaying saved JSON file:\n";
        std::ifstream file(testConfigPath);
        if (file.is_open())
        {
            std::string line;
            while (std::getline(file, line))
            {
                std::cout << "   " << line << "\n";
            }
            file.close();
        }
        std::cout << "\n";

        // Test 5: Load config from file
        std::cout << "4. Loading configuration from file...\n";
        tomtom::sdk::filesystem::LocalStore store2(testConfigPath);
        const auto &loadedConfig = store2.getConfig();

        std::cout << "   Storage root: " << loadedConfig.storage_root << "\n";
        std::cout << "   Layout: " << static_cast<int>(loadedConfig.layout) << "\n";
        std::cout << "   Archive raw: " << (loadedConfig.archive_raw_data ? "true" : "false") << "\n";
        std::cout << "   Log level: " << loadedConfig.log_level << "\n";
        std::cout << "   Export formats: ";
        for (const auto &fmt : loadedConfig.export_formats)
        {
            std::cout << fmt << " ";
        }
        std::cout << "\n\n";

        // Test 6: Verify values match
        bool success = true;
        if (loadedConfig.storage_root != newConfig.storage_root)
        {
            std::cout << "ERROR: storage_root mismatch!\n";
            success = false;
        }
        if (loadedConfig.layout != newConfig.layout)
        {
            std::cout << "ERROR: layout mismatch!\n";
            success = false;
        }
        if (loadedConfig.archive_raw_data != newConfig.archive_raw_data)
        {
            std::cout << "ERROR: archive_raw_data mismatch!\n";
            success = false;
        }
        if (loadedConfig.log_level != newConfig.log_level)
        {
            std::cout << "ERROR: log_level mismatch!\n";
            success = false;
        }
        if (loadedConfig.export_formats != newConfig.export_formats)
        {
            std::cout << "ERROR: export_formats mismatch!\n";
            success = false;
        }

        if (success)
        {
            std::cout << "✓ All tests passed! Configuration serialization works correctly.\n";
        }
        else
        {
            std::cout << "✗ Some tests failed!\n";
            return 1;
        }

        // Cleanup
        std::filesystem::remove(testConfigPath);
    }
    catch (const std::exception &e)
    {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
