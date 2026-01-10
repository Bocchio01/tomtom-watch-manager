#pragma once

#include "activity_converter.hpp"
#include <tomtom/watch.hpp>
#include <filesystem>
#include <functional>
#include <memory>

namespace tomtom::sdk
{

    /**
     * @brief High-level activity management and orchestration
     *
     * This class bridges the gap between the device-level core library
     * and the user-facing application. It handles:
     * - Downloading activities from the watch
     * - Converting to various formats (GPX/TCX/CSV)
     * - Saving to the local filesystem
     * - Managing local activity files
     *
     * This is shared logic used by both CLI and GUI applications.
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
         * @brief Construct activity manager
         * @param watch Connected watch instance
         * @param storage_dir Directory for storing activities
         */
        ActivityManager(
            std::shared_ptr<tomtom::Watch> watch,
            const std::filesystem::path &storage_dir);

        // ====================================================================
        // Download & Export Operations
        // ====================================================================

        /**
         * @brief Download activity and save to disk
         * @param index Activity index on watch
         * @param format Export format (GPX, TCX, CSV)
         * @param progress Optional progress callback
         * @return Path to saved file
         *
         * This method:
         * 1. Downloads the activity from the watch
         * 2. Converts it to the requested format
         * 3. Saves it to the storage directory
         */
        std::filesystem::path downloadAndSave(
            uint16_t index,
            ExportFormat format,
            ProgressCallback progress = nullptr);

        /**
         * @brief Download all activities from watch
         * @param format Export format
         * @param progress Optional progress callback
         * @return Number of activities downloaded
         */
        size_t downloadAll(
            ExportFormat format,
            ProgressCallback progress = nullptr);

        /**
         * @brief Convert existing activity file to different format
         * @param source_path Path to source activity file
         * @param target_format Target format
         * @return Path to converted file
         *
         * Note: Source must be a .ttbin file (raw watch format)
         */
        std::filesystem::path convertFile(
            const std::filesystem::path &source_path,
            ExportFormat target_format);

        // ====================================================================
        // Local File Management
        // ====================================================================

        /**
         * @brief List activities in local storage directory
         * @return Vector of file paths
         */
        std::vector<std::filesystem::path> listLocal() const;

        /**
         * @brief Get storage directory
         * @return Storage directory path
         */
        std::filesystem::path getStorageDir() const { return storage_dir_; }

        /**
         * @brief Set storage directory
         * @param dir New storage directory
         */
        void setStorageDir(const std::filesystem::path &dir);

        // ====================================================================
        // Watch Operations (passthrough)
        // ====================================================================

        /**
         * @brief Get count of activities on watch
         * @return Number of activities
         */
        size_t getWatchActivityCount() const;

        /**
         * @brief List activities on watch
         * @return Activity metadata
         */
        std::vector<tomtom::services::activity::models::ActivityInfo> listOnWatch() const;

    private:
        std::shared_ptr<tomtom::Watch> watch_;
        std::filesystem::path storage_dir_;

        // Helper to generate filename for activity
        std::string generateFilename(
            const tomtom::services::activity::models::Activity &activity,
            ExportFormat format) const;

        // Helper to ensure storage directory exists
        void ensureStorageDir();
    };

} // namespace tomtom::sdk
