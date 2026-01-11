#pragma once

#include "activity_manager.hpp"
#include "gps_manager.hpp"
#include "settings_manager.hpp"
#include <memory>
#include <functional>

namespace tomtom::sdk
{

    /**
     * @brief Orchestrator for automated synchronization operations
     *
     * Provides the "One Button" sync functionality that combines
     * time synchronization, GPS updates, and activity downloads.
     */
    class SyncService
    {
    public:
        /**
         * @brief Configuration for auto-sync operations
         */
        struct Config
        {
            bool sync_time = true;           ///< Sync watch time with PC
            bool update_gps = true;          ///< Update GPS QuickFix data
            bool download_activities = true; ///< Download activities from watch
        };

        /**
         * @brief Progress callback type
         * @param stage Current stage name
         * @param stage_progress Stage progress (0-100)
         * @param message Status message
         */
        using ProgressCallback = std::function<void(const std::string &stage, int stage_progress, const std::string &message)>;

        /**
         * @brief Construct sync service
         * @param watch Connected watch instance
         * @param activity_destination Directory for downloading activities
         * @param export_formats Formats to export activities (empty = .ttbin only)
         */
        SyncService(
            std::shared_ptr<tomtom::Watch> watch,
            const std::filesystem::path &activity_destination = std::filesystem::current_path() / "activities",
            const std::vector<ActivityExporter::Format> &export_formats = {});

        /**
         * @brief Run automated synchronization
         * @param config Sync configuration
         * @param progress Optional progress callback
         *
         * Executes enabled sync operations in sequence:
         * 1. Time sync (if enabled)
         * 2. GPS QuickFix update (if enabled)
         * 3. Activity download (if enabled)
         */
        void runAutoSync(Config config, ProgressCallback progress = nullptr);

        /**
         * @brief Set activity destination directory
         * @param destination New destination path
         */
        void setActivityDestination(const std::filesystem::path &destination);

        /**
         * @brief Set export formats for activities
         * @param formats Vector of formats to export
         */
        void setExportFormats(const std::vector<ActivityExporter::Format> &formats);

    private:
        std::shared_ptr<tomtom::Watch> watch_;
        std::filesystem::path activity_destination_;
        std::vector<ActivityExporter::Format> export_formats_;

        std::unique_ptr<SettingsManager> settings_;
        std::unique_ptr<GpsManager> gps_;
        std::unique_ptr<ActivityManager> activity_;
    };

} // namespace tomtom::sdk
