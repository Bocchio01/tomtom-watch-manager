#pragma once

#include <tomtom/watch.hpp>
#include <tomtom/services/preferences/preferences.hpp>
#include <memory>
#include <string>

namespace tomtom::sdk
{

    /**
     * @brief Settings and preferences manager
     *
     * Combines watch preferences, time synchronization, and device settings
     * into a single high-level manager.
     */
    class SettingsManager
    {
    public:
        /**
         * @brief Construct settings manager
         * @param watch Connected watch instance
         */
        explicit SettingsManager(std::shared_ptr<tomtom::Watch> watch);

        // ====================================================================
        // Preferences Operations
        // ====================================================================

        /**
         * @brief Get current preferences from watch
         * @return Preferences structure
         */
        tomtom::services::preferences::models::Preferences getPreferences();

        /**
         * @brief Set preferences on watch
         * @param prefs Preferences to write
         */
        void setPreferences(const tomtom::services::preferences::models::Preferences &prefs);

        // ====================================================================
        // High-Level Actions
        // ====================================================================

        /**
         * @brief Sync PC time to watch
         *
         * Gets current system time, converts to watch format, and sends to device.
         */
        void syncTime();

        /**
         * @brief Set watch name
         * @param name New watch name
         *
         * Updates just the watch name field in preferences.
         */
        void setWatchName(const std::string &name);

        /**
         * @brief Get watch name
         * @return Current watch name from preferences
         */
        std::string getWatchName();

        /**
         * @brief Perform factory reset
         *
         * Formats watch storage, erasing all user data.
         * @warning This is destructive and cannot be undone!
         */
        void factoryReset();

    private:
        std::shared_ptr<tomtom::Watch> watch_;
    };

} // namespace tomtom::sdk
