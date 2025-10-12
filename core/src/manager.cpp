#include <algorithm>
#include <spdlog/spdlog.h>

#include "tomtom/defines.hpp"
#include "tomtom/watch.hpp"
#include "tomtom/manager.hpp"

namespace tomtom
{
    Manager::Manager()
    {
        spdlog::debug("Manager initialized");
    }

    Manager::~Manager()
    {
        spdlog::debug("Manager destroyed");
    }

    void Manager::refreshDeviceCache()
    {
        spdlog::debug("Refreshing device cache");
        cachedDevices_ = USBConnection::enumerateDevices();
        spdlog::info("Found {} TomTom device(s)", cachedDevices_.size());
    }

    std::vector<WatchInfo> Manager::detectWatches()
    {
        spdlog::debug("Detecting TomTom watches");

        refreshDeviceCache();

        std::vector<WatchInfo> watches;
        watches.reserve(cachedDevices_.size());

        for (const auto &device : cachedDevices_)
        {
            WatchInfo info;
            info.product_id = device.product_id;
            info.serial_number = device.serial_number;

            // Determine watch model from product ID
            // switch (device.product_id)
            // {
            // case TOMTOM_MULTISPORT_PRODUCT_ID:
            //     info.model = "TomTom Multisport";
            //     break;
            // case TOMTOM_SPARK_MUSIC_PRODUCT_ID:
            //     info.model = "TomTom Spark/Runner Music";
            //     break;
            // case TOMTOM_SPARK_CARDIO_PRODUCT_ID:
            //     info.model = "TomTom Runner Cardio/Adventurer";
            //     break;
            // default:
            //     info.model = "Unknown TomTom Watch";
            //     break;
            // }

            watches.push_back(info);
        }

        spdlog::info("Detected {} watch(es)", watches.size());
        return watches;
    }

    std::shared_ptr<Watch> Manager::connectToWatch()
    {
        spdlog::debug("Attempting to connect to first available watch");

        refreshDeviceCache();

        if (cachedDevices_.empty())
        {
            spdlog::warn("No TomTom watches found");
            return nullptr;
        }

        return connectToWatch(0);
    }

    std::shared_ptr<Watch> Manager::connectToWatch(size_t index)
    {
        spdlog::debug("Attempting to connect to watch at index {}", index);

        if (cachedDevices_.empty())
        {
            refreshDeviceCache();
        }

        if (index >= cachedDevices_.size())
        {
            spdlog::error("Invalid watch index: {} (available: {})", index, cachedDevices_.size());
            return nullptr;
        }

        const auto &device = cachedDevices_[index];

        try
        {
            spdlog::info("Connecting to watch: {} (serial: {})", device.product_id, device.serial_number);

            auto connection = std::make_unique<USBConnection>(device);
            auto watch = std::make_shared<Watch>(std::move(connection));

            // Initialize the watch
            auto error = watch->startUp();
            if (error != WatchError::NoError)
            {
                spdlog::error("Failed to initialize watch: error code {}",
                              static_cast<int>(error));
                return nullptr;
            }

            spdlog::info("Successfully connected to watch");
            return watch;
        }
        catch (const std::exception &e)
        {
            spdlog::error("Exception while connecting to watch: {}", e.what());
            return nullptr;
        }
    }

    std::shared_ptr<Watch> Manager::connectToWatch(const std::string &serial)
    {
        spdlog::debug("Attempting to connect to watch with serial: {}", serial);

        refreshDeviceCache();

        // Find device with matching serial
        auto it = std::find_if(cachedDevices_.begin(), cachedDevices_.end(),
                               [&serial](const USBDeviceInfo &device)
                               {
                                   return device.serial_number == serial;
                               });

        if (it == cachedDevices_.end())
        {
            spdlog::warn("No watch found with serial: {}", serial);
            return nullptr;
        }

        size_t index = std::distance(cachedDevices_.begin(), it);
        return connectToWatch(index);
    }

    std::vector<std::shared_ptr<Watch>> Manager::enumerate()
    {
        spdlog::debug("Enumerating all TomTom watches");

        std::vector<std::shared_ptr<Watch>> watches;
        auto devices = USBConnection::enumerateDevices();

        spdlog::info("Found {} device(s) to enumerate", devices.size());

        for (size_t i = 0; i < devices.size(); ++i)
        {
            const auto &device = devices[i];

            try
            {
                spdlog::debug("Enumerating device {}: product ID 0x{:04X}, serial {}",
                              i, device.product_id, device.serial_number);

                auto connection = std::make_unique<USBConnection>(device);
                auto watch = std::make_shared<Watch>(std::move(connection));

                // Initialize the watch
                auto error = watch->startUp();
                if (error != WatchError::NoError)
                {
                    spdlog::warn("Failed to initialize watch {}: error code {}",
                                 i, static_cast<int>(error));
                    continue;
                }

                watches.push_back(watch);
            }
            catch (const std::exception &e)
            {
                spdlog::warn("Failed to enumerate device {}: {}", i, e.what());
                continue;
            }
        }

        spdlog::info("Successfully enumerated {} watch(es)", watches.size());
        return watches;
    }

    size_t Manager::getWatchCount() const
    {
        return cachedDevices_.size();
    }

    bool Manager::hasWatches() const
    {
        return !cachedDevices_.empty();
    }

} // namespace tomtom