#include <algorithm>
#include <spdlog/spdlog.h>

#include <tomtom/core/transport/connection_factory.hpp>
#include <tomtom/core/watch.hpp>

#include "tomtom/sdk/services/activity_service.hpp"
#include "tomtom/sdk/services/preferences_service.hpp"
#include "tomtom/sdk/services/gps_service.hpp"
#include "tomtom/sdk/services/device_service.hpp"
#include "tomtom/sdk/store/local_store.hpp"

#include "tomtom/sdk/manager.hpp"

namespace tomtom::sdk
{
    Manager::Manager()
    {
        // Initialize logging immediately using a default level
        spdlog::set_level(spdlog::level::debug);

        // Initialize LocalStore (loads config)
        localStore_ = std::make_unique<store::LocalStore>("tomtom_config.json");

        // Now re-configure logging based on what the config said
        auto logDir = localStore_->getLogDirectory();
        // TODO: Setup file logger here using spdlog::sinks::basic_file_sink_mt

        spdlog::debug("Manager initialized with storage at: {}",
                      localStore_->getConfig().storage_root.string());
    }

    Manager::~Manager() = default;

    std::vector<core::transport::DeviceInfo> Manager::detectWatches()
    {
        spdlog::debug("Detecting TomTom watches");

        refreshDeviceCache();

        std::vector<core::transport::DeviceInfo> watches;
        watches.reserve(cachedDevices_.size());

        for (const auto &device : cachedDevices_)
        {
            core::transport::DeviceInfo info;
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

    std::shared_ptr<core::Watch> Manager::connectToWatch()
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

    std::shared_ptr<core::Watch> Manager::connectToWatch(size_t index)
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
            spdlog::info("Connecting to watch: {} (serial: {})", static_cast<uint16_t>(device.product_id), device.serial_number);

            auto connection = core::transport::DeviceConnectionFactory::create(device);
            auto watch = std::make_shared<core::Watch>(std::move(connection));

            // TODO: Startup sequence
            // Initialize the watch
            // auto error = watch->startUp();
            // if (error != WatchError::NoError)
            // {
            //     spdlog::error("Failed to initialize watch: error code {}", static_cast<int>(error));
            //     return nullptr;
            // }

            connectedWatch_ = watch;

            // Initialize services
            activityService_ = std::make_unique<services::ActivityService>(connectedWatch_, localStore_);
            preferencesService_ = std::make_shared<services::PreferencesService>(connectedWatch_);
            gpsService_ = std::make_unique<services::GpsService>(connectedWatch_, preferencesService_);
            deviceService_ = std::make_unique<services::DeviceService>(connectedWatch_);

            spdlog::info("Successfully connected to watch");
            return watch;
        }
        catch (const std::exception &e)
        {
            spdlog::error("Exception while connecting to watch: {}", e.what());
            return nullptr;
        }
    }

    std::shared_ptr<core::Watch> Manager::connectToWatch(const std::string &serial)
    {
        spdlog::debug("Attempting to connect to watch with serial: {}", serial);

        refreshDeviceCache();

        // Find device with matching serial
        auto it = std::find_if(cachedDevices_.begin(), cachedDevices_.end(),
                               [&serial](const core::transport::DeviceInfo &device)
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
        cachedDevices_ = core::transport::DeviceConnectionFactory::enumerate();
        spdlog::info("Found {} TomTom device(s)", cachedDevices_.size());
    }

    // ====================================================================
    // Service Access
    // ====================================================================

    services::DeviceService *Manager::device()
    {
        if (!deviceService_)
        {
            throw std::runtime_error("No watch connected - call connectToWatch() first");
        }
        return deviceService_.get();
    }

    services::ActivityService *Manager::activity()
    {
        if (!activityService_)
        {
            throw std::runtime_error("No watch connected - call connectToWatch() first");
        }
        return activityService_.get();
    }

    services::GpsService *Manager::gps()
    {
        if (!gpsService_)
        {
            throw std::runtime_error("No watch connected - call connectToWatch() first");
        }
        return gpsService_.get();
    }

    services::PreferencesService *Manager::preferences()
    {
        if (!preferencesService_)
        {
            throw std::runtime_error("No watch connected - call connectToWatch() first");
        }
        return preferencesService_.get();
    }

    store::LocalStore &Manager::store()
    {
        return *localStore_;
    }

}