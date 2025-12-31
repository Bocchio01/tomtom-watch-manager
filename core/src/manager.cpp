#include <algorithm>
#include <spdlog/spdlog.h>

#include "tomtom/defines.hpp"
#include "tomtom/watch.hpp"
#include "tomtom/manager.hpp"
#include "tomtom/connection/connection_factory.hpp"

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

            auto connection = DeviceConnectionFactory::create(device);
            auto watch = std::make_shared<Watch>(std::move(connection));

            // TODO: Startup sequence
            // Initialize the watch
            // auto error = watch->startUp();
            // if (error != WatchError::NoError)
            // {
            //     spdlog::error("Failed to initialize watch: error code {}", static_cast<int>(error));
            //     return nullptr;
            // }

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
                               [&serial](const DeviceInfo &device)
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

    size_t Manager::getWatchCount() const
    {
        return cachedDevices_.size();
    }

    bool Manager::hasWatches() const
    {
        return !cachedDevices_.empty();
    }

    void Manager::refreshDeviceCache()
    {
        spdlog::debug("Refreshing device cache");
        cachedDevices_ = DeviceConnection::enumerate();
        spdlog::info("Found {} TomTom device(s)", cachedDevices_.size());
    }

}