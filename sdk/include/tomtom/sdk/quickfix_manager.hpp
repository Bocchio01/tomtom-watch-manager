#pragma once

#include <tomtom/watch.hpp>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>

namespace tomtom::sdk
{

    /**
     * @brief GPS QuickFix update manager
     *
     * This class orchestrates GPS QuickFix (ephemeris) updates by:
     * - Downloading ephemeris data from URLs (network operation)
     * - Writing data to the watch (device operation)
     * - Caching data locally for offline use
     *
     * Separates network/filesystem logic (lib layer) from
     * device operations (core layer).
     */
    class QuickFixManager
    {
    public:
        /**
         * @brief Progress callback type
         * @param downloaded Bytes downloaded so far
         * @param total Total file size (0 if unknown)
         */
        using ProgressCallback = std::function<void(size_t downloaded, size_t total)>;

        /**
         * @brief Default TomTom ephemeris URL
         */
        static constexpr const char *DEFAULT_URL = "https://gpsquickfix.services.tomtom.com/v1/fit/ephemeris/{DAYS}/";

        /**
         * @brief Construct QuickFix manager
         * @param watch Connected watch instance
         * @param cache_dir Optional cache directory for ephemeris files
         */
        QuickFixManager(
            std::shared_ptr<tomtom::Watch> watch,
            const std::filesystem::path &cache_dir = std::filesystem::temp_directory_path() / "tomtom_gps_cache");

        // ====================================================================
        // Update Operations
        // ====================================================================

        /**
         * @brief Update GPS QuickFix from URL
         * @param url Ephemeris URL (use {DAYS} placeholder)
         * @param days Number of days (3 or 7)
         * @param reset_gps Whether to reset GPS after update
         * @param progress Optional progress callback
         *
         * This method:
         * 1. Builds the URL with the days parameter
         * 2. Downloads ephemeris data (HTTP - lib layer)
         * 3. Writes to watch (device - core layer)
         */
        void updateFromUrl(
            const std::string &url = DEFAULT_URL,
            int days = 7,
            bool reset_gps = true,
            ProgressCallback progress = nullptr);

        /**
         * @brief Update GPS QuickFix from local file
         * @param file_path Path to ephemeris file
         * @param reset_gps Whether to reset GPS after update
         */
        void updateFromFile(
            const std::filesystem::path &file_path,
            bool reset_gps = true);

        /**
         * @brief Download and cache ephemeris data (without updating watch)
         * @param url Ephemeris URL
         * @param days Number of days
         * @param progress Optional progress callback
         * @return Path to cached file
         *
         * Useful for pre-downloading updates or offline scenarios.
         */
        std::filesystem::path downloadAndCache(
            const std::string &url = DEFAULT_URL,
            int days = 7,
            ProgressCallback progress = nullptr);

        /**
         * @brief Update from most recent cached file
         * @param reset_gps Whether to reset GPS after update
         * @return true if cached file was found and used
         */
        bool updateFromCache(bool reset_gps = true);

        // ====================================================================
        // Cache Management
        // ====================================================================

        /**
         * @brief Get cache directory
         */
        std::filesystem::path getCacheDir() const { return cache_dir_; }

        /**
         * @brief Set cache directory
         */
        void setCacheDir(const std::filesystem::path &dir);

        /**
         * @brief Clear all cached ephemeris files
         */
        void clearCache();

        /**
         * @brief Get most recent cached file
         * @return Path to cached file, or empty if none found
         */
        std::filesystem::path getMostRecentCachedFile() const;

        // ====================================================================
        // URL Helpers
        // ====================================================================

        /**
         * @brief Build ephemeris URL by replacing {DAYS} placeholder
         * @param url_template URL template with {DAYS} placeholder
         * @param days Number of days (3 or 7)
         * @return Formatted URL
         */
        static std::string buildUrl(const std::string &url_template, int days);

        /**
         * @brief Validate ephemeris URL
         * @param url URL to validate
         * @return true if URL is valid
         */
        static bool validateUrl(const std::string &url);

        /**
         * @brief Validate days parameter
         * @param days Days value
         * @return true if valid (3 or 7)
         */
        static bool validateDays(int days);

    private:
        std::shared_ptr<tomtom::Watch> watch_;
        std::filesystem::path cache_dir_;

        // Helper to generate cache filename
        std::string generateCacheFilename(int days) const;

        // Helper to ensure cache directory exists
        void ensureCacheDir();
    };

} // namespace tomtom::sdk
