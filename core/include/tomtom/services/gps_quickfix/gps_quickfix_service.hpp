// ============================================================================
// gps_quickfix_service.hpp - GPS QuickFix update service
// ============================================================================
#pragma once

#include "tomtom/services/file_service.hpp"
#include "tomtom/services/watch_control_service.hpp"

#include <memory>
#include <string>
#include <functional>

namespace tomtom::services::gps_quickfix
{

    /**
     * @brief Service for updating GPS QuickFix (ephemeris) data
     *
     * Handles downloading and uploading GPS ephemeris data to improve
     * GPS fix acquisition time. The data is downloaded from TomTom's
     * ephemeris service and written to file ID 0x00010100.
     */
    class GpsQuickFixService
    {
    public:
        /**
         * @brief Progress callback type for download operations
         * @param downloaded Bytes downloaded so far
         * @param total Total file size (0 if unknown)
         */
        using ProgressCallback = std::function<void(size_t downloaded, size_t total)>;

        /**
         * @brief Construct GPS QuickFix service
         * @param file_service Low-level file service for watch communication
         * @param control_service Watch control service for GPS reset
         */
        explicit GpsQuickFixService(
            std::shared_ptr<services::FileService> file_service,
            std::shared_ptr<services::WatchControlService> control_service);

        // ====================================================================
        // Update Operations
        // ====================================================================

        /**
         * @brief Update GPS QuickFix data from URL
         * @param url Ephemeris URL (must contain {DAYS} placeholder)
         * @param days Number of days (3 or 7)
         * @param reset_gps Whether to reset GPS processor after update
         * @param progress Optional progress callback
         * @throws std::runtime_error if update fails
         */
        void update(
            const std::string &url,
            int days = 7,
            bool reset_gps = true,
            ProgressCallback progress = nullptr);

        /**
         * @brief Update GPS QuickFix data from raw data
         * @param data Ephemeris data bytes
         * @param reset_gps Whether to reset GPS processor after update
         * @throws std::runtime_error if update fails
         */
        void updateFromData(
            const std::vector<uint8_t> &data,
            bool reset_gps = true);

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

    private:
        std::shared_ptr<services::FileService> file_service_;
        std::shared_ptr<services::WatchControlService> control_service_;

        /**
         * @brief Download file from URL
         * @param url URL to download from
         * @param progress Optional progress callback
         * @return Downloaded data
         */
        std::vector<uint8_t> downloadFile(const std::string &url, ProgressCallback progress);
    };

} // namespace tomtom::services::gps_quickfix
