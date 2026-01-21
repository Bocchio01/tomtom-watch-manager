#pragma once

#include <memory>
#include <functional>
#include <tomtom/core/watch.hpp>
#include "tomtom/sdk/services/preferences_service.hpp"

namespace tomtom::sdk::services
{
    /**
     * @brief Service for managing GPS QuickFix data
     *
     * QuickFix is satellite ephemeris data that allows the watch to acquire
     * GPS lock much faster. Without regular updates, GPS lock can take several
     * minutes. With fresh data, it typically takes 10-30 seconds.
     *
     * This service:
     * - Checks if QuickFix data is stale (older than 3 days)
     * - Downloads fresh ephemeris data from TomTom servers
     * - Uploads it to the watch
     * - Updates the timestamp in preferences
     *
     * Example usage:
     * ```cpp
     * auto gps = manager.gps();
     * if (gps->isUpdateNeeded()) {
     *     gps->updateQuickFix([](float progress, const std::string& status) {
     *         std::cout << status << " (" << (progress * 100) << "%)\n";
     *     });
     * }
     * ```
     */
    class GpsService
    {
    public:
        /**
         * @brief Callback for update progress (0.0 to 1.0)
         * @param progress Progress value between 0.0 and 1.0
         * @param status Human-readable status message
         */
        using ProgressCallback = std::function<void(float progress, const std::string &status)>;

        /**
         * @brief Constructor
         * @param watch Connected watch instance
         * @param prefsService Preferences service for reading/writing config
         */
        GpsService(std::shared_ptr<core::Watch> watch,
                   std::shared_ptr<PreferencesService> prefsService);

        /**
         * @brief Checks if the QuickFix data on the watch is expired (older than 3 days)
         *
         * QuickFix data should be updated every 3 days for optimal GPS performance.
         * If the data is older, GPS lock times will gradually degrade.
         *
         * @return true if update is recommended, false otherwise
         */
        bool isUpdateNeeded() const;

        /**
         * @brief Downloads fresh QuickFix data and uploads it to the watch
         *
         * Process:
         * 1. Reads Ephemeris URL from watch preferences
         * 2. Downloads the file (injecting {DAYS} parameter)
         * 3. Uploads to File ID 0x00010100
         * 4. Updates 'ephemeris_modified' timestamp in preferences
         *
         * @param progress Optional callback for progress updates
         * @throws std::runtime_error if download or upload fails
         */
        void updateQuickFix(ProgressCallback progress = nullptr);

    private:
        std::shared_ptr<core::Watch> watch_;
        std::shared_ptr<PreferencesService> prefsService_;

        /**
         * @brief Helper to expand URL placeholders like {DAYS} or {Model}
         * @param url URL template string
         * @param days Number of days to inject
         * @return Expanded URL
         */
        std::string expandUrl(std::string url, int days) const;
    };
}
