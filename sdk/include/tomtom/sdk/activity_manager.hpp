#pragma once

#include "activity_exporter.hpp"
#include <tomtom/watch.hpp>
#include <filesystem>
#include <functional>
#include <memory>

namespace tomtom::sdk
{

    /**
     * @brief Simplified activity manager for downloading and managing activities
     *
     * Handles the filesystem and batch operations for activities.
     * Downloads from watch, saves .ttbin files, and optionally exports to GPX/TCX/CSV.
     */
    class ActivityManager
    {
    public:
        /**
         * @brief Progress callback type
         * @param current Current progress value
         * @param total Total progress value
         * @param message Status message
         */
        using ProgressCallback = std::function<void(size_t current, size_t total, const std::string &message)>;

        /**
         * @brief Activity summary for listing
         */
        struct ActivitySummary
        {
            uint16_t index;
            std::string type;
            std::time_t start_time;
            uint32_t duration_seconds;
            float distance_meters;
            uint16_t calories;
        };

        /**
         * @brief Construct activity manager
         * @param watch Connected watch instance
         */
        explicit ActivityManager(std::shared_ptr<tomtom::Watch> watch);

        // ====================================================================
        // Download Operations
        // ====================================================================

        /**
         * @brief Download new activities from watch, save .ttbin, and export
         * @param destination Directory to save files
         * @param export_formats Vector of formats to export (empty = no export)
         * @param progress Optional progress callback
         * @return Number of activities downloaded
         *
         * Note: Currently exports only to specified formats (GPX/TCX/CSV).
         * Raw .ttbin saving to be added in future update.
         */
        size_t downloadNewActivities(
            const std::filesystem::path &destination,
            const std::vector<ActivityExporter::Format> &export_formats = {},
            ProgressCallback progress = nullptr);

        /**
         * @brief Download specific activity by index
         * @param index Activity index on watch
         * @param destination Directory to save file
         * @param export_formats Vector of formats to export
         * @param progress Optional progress callback
         * @return Paths to saved files
         */
        std::vector<std::filesystem::path> downloadActivity(
            uint16_t index,
            const std::filesystem::path &destination,
            const std::vector<ActivityExporter::Format> &export_formats = {},
            ProgressCallback progress = nullptr);

        // ====================================================================
        // List Operations
        // ====================================================================

        /**
         * @brief List activities currently on the watch
         * @return Vector of activity summaries
         */
        std::vector<ActivitySummary> listWatchFiles();

        /**
         * @brief List local activity files in a directory
         * @param dir Directory to scan
         * @return Vector of file paths (all activity-related files)
         */
        std::vector<std::filesystem::path> listLocalFiles(const std::filesystem::path &dir);

        // ====================================================================
        // Delete Operations
        // ====================================================================

        /**
         * @brief Clear all activities from watch
         * @return Number of activities deleted
         */
        size_t clearWatchHistory();

    private:
        std::shared_ptr<tomtom::Watch> watch_;

        // Helper to generate filename from activity
        std::string generateFilename(
            const tomtom::services::activity::models::Activity &activity,
            const std::string &extension) const;
    };

} // namespace tomtom::sdk
