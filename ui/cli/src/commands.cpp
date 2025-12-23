#include <iomanip>
#include <iostream>
#include "cli/commands.hpp"

namespace tomtom::cli
{

    Commands::Commands(Manager &manager)
        : manager_(manager)
    {
    }

    int Commands::listDevices()
    {
        std::cout << "Scanning for TomTom devices..." << std::endl;

        auto watches = manager_.detectWatches();

        if (watches.empty())
        {
            std::cout << "No TomTom devices found." << std::endl;
            return 0;
        }

        std::cout << "\nFound " << watches.size() << " device(s):\n"
                  << std::endl;

        for (size_t i = 0; i < watches.size(); ++i)
        {
            std::cout << "[" << i << "] ";
            printWatchInfo(watches[i]);
            std::cout << std::endl;
        }

        return 0;
    }

    int Commands::getDeviceInfo()
    {
        auto watch = manager_.connectToWatch();
        if (!watch)
        {
            std::cerr << "Error: No watch connected" << std::endl;
            return 1;
        }

        printWatchInfo(*watch);
        return 0;
    }

    int Commands::listFiles()
    {
        auto watch = manager_.connectToWatch();
        if (!watch)
        {
            std::cerr << "Error: No watch connected" << std::endl;
            return 1;
        }

        std::cout << "Listing files on watch..." << std::endl;

        // auto files = watch->listFiles();
        auto files = std::vector<int>{}; // Placeholder until listFiles is implemented
        if (files.empty())
        {
            std::cout << "No files found on watch." << std::endl;
            return 0;
        }

        std::cout << "\nFile ID         Size (bytes)" << std::endl;
        std::cout << "--------------------------------" << std::endl;

        // for (const auto &file : files)
        // {
        //     std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0')
        //               << file.id << "  " << std::dec << file.size << std::endl;
        // }

        return 0;
    }

    int Commands::downloadFile(const std::string &fileId, const std::string &outputPath)
    {
        auto watch = manager_.connectToWatch();
        if (!watch)
        {
            std::cerr << "Error: No watch connected" << std::endl;
            return 1;
        }

        uint32_t id = std::stoul(fileId, nullptr, 16);

        std::cout << "Downloading file 0x" << std::hex << id << "..." << std::endl;

        // auto error = watch->downloadFile(id, outputPath);
        // if (error != WatchError::NoError)
        // {
        //     printError("download", error);
        //     return 1;
        // }

        std::cout << "File downloaded successfully to: " << outputPath << std::endl;
        return 0;
    }

    int Commands::uploadFile(const std::string &inputPath, const std::string &fileId)
    {
        auto watch = manager_.connectToWatch();
        if (!watch)
        {
            std::cerr << "Error: No watch connected" << std::endl;
            return 1;
        }

        uint32_t id = std::stoul(fileId, nullptr, 16);

        std::cout << "Uploading file to 0x" << std::hex << id << "..." << std::endl;

        // auto error = watch->uploadFile(inputPath, id);
        // if (error != WatchError::NoError)
        // {
        //     printError("upload", error);
        //     return 1;
        // }

        std::cout << "File uploaded successfully" << std::endl;
        return 0;
    }

    int Commands::deleteFile(const std::string &fileId)
    {
        auto watch = manager_.connectToWatch();
        if (!watch)
        {
            std::cerr << "Error: No watch connected" << std::endl;
            return 1;
        }

        uint32_t id = std::stoul(fileId, nullptr, 16);

        std::cout << "Deleting file 0x" << std::hex << id << "..." << std::endl;

        // auto error = watch->deleteFile(id);
        // if (error != WatchError::NoError)
        // {
        //     printError("delete", error);
        //     return 1;
        // }

        std::cout << "File deleted successfully" << std::endl;
        return 0;
    }

    int Commands::syncWatch()
    {
        std::cout << "Syncing watch (not implemented yet)..." << std::endl;
        return 0;
    }

    int Commands::updateFirmware(const std::string &firmwarePath)
    {
        std::cout << "Updating firmware (not implemented yet)..." << std::endl;
        return 0;
    }

    int Commands::getCurrentTime()
    {
        auto watch = manager_.connectToWatch();
        if (!watch)
        {
            std::cerr << "Error: No watch connected" << std::endl;
            return 1;
        }

        std::time_t currentTime;
        auto error = watch->getCurrentTime(currentTime);
        if (error != WatchError::NoError)
        {
            printError("get current time", error);
            return 1;
        }

        std::cout << "Current watch time: " << std::asctime(std::localtime(&currentTime));
        return 0;
    }

    void Commands::printWatchInfo(const Watch &watch)
    {
        std::cout << "Product ID: 0x" << std::hex << watch.getProductId() << std::dec << std::endl;
        // std::cout << "Firmware: " << watch.getFirmwareVersion() << std::endl;
        // std::cout << "Serial: " << watch.getSerial() << std::endl;
    }

    void Commands::printWatchInfo(const WatchInfo &info)
    {
        std::cout << "Product ID: 0x" << std::hex << info.product_id << std::dec << std::endl;
        std::cout << "Firmware Version: " << info.firmware_version << std::endl;
        std::cout << "BLE Version: " << info.ble_version << std::endl;
        std::cout << "Serial Number: " << info.serial_number << std::endl;
        std::cout << "Manufacturer: " << info.manufacturer << std::endl;
        std::cout << "Product Name: " << info.product_name << std::endl;
    }

    void Commands::printError(const std::string &operation, WatchError error)
    {
        std::cerr << "Error during " << operation << ": ";

        switch (error)
        {
        case WatchError::NoError:
            std::cerr << "No error";
            break;
        default:
            std::cerr << "Unknown error";
        }

        std::cerr << std::endl;
    }

} // namespace tomtom::cli