#include "tomtom/sdk/settings_manager.hpp"
#include <spdlog/spdlog.h>
#include <ctime>
#include <chrono>

namespace tomtom::sdk
{

    SettingsManager::SettingsManager(std::shared_ptr<tomtom::Watch> watch)
        : watch_(std::move(watch))
    {
        if (!watch_)
        {
            throw std::invalid_argument("Watch cannot be null");
        }
    }

    tomtom::services::preferences::models::Preferences SettingsManager::getPreferences()
    {
        spdlog::debug("Reading preferences from watch");
        return watch_->preferences().get();
    }

    void SettingsManager::setPreferences(const tomtom::services::preferences::models::Preferences &prefs)
    {
        spdlog::debug("Writing preferences to watch");
        watch_->preferences().set(prefs);
        spdlog::info("Preferences updated successfully");
    }

    void SettingsManager::syncTime()
    {
        // TODO: Implement time synchronization
        // The watch protocol may not support direct time setting
        // Time might be set automatically when syncing with TomTom servers
        // or through a specific protocol command not yet implemented

        spdlog::warn("Time synchronization not yet implemented");
        spdlog::info("Watch may auto-sync time when connected to TomTom MySports");
    }

    void SettingsManager::setWatchName(const std::string &name)
    {
        spdlog::info("Setting watch name to: {}", name);

        auto prefs = watch_->preferences().get();
        prefs.watch_name = name;
        watch_->preferences().set(prefs);

        spdlog::info("Watch name updated successfully");
    }

    std::string SettingsManager::getWatchName()
    {
        auto prefs = watch_->preferences().get();
        return prefs.watch_name;
    }

    void SettingsManager::factoryReset()
    {
        spdlog::warn("Performing factory reset - all data will be erased!");

        watch_->watch().formatWatch();

        spdlog::info("Factory reset completed");
    }

} // namespace tomtom::sdk
