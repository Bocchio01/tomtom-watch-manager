// ============================================================================
// activity_service.hpp - High-level activity operations
// ============================================================================
#pragma once

#include "activity_models.hpp"
#include "activity_parser.hpp"
#include "../common/export_formats.hpp"
#include "tomtom/services/file_service.hpp"

#include <memory>
#include <vector>
#include <string>

namespace tomtom::services::activity
{

    /**
     * @brief High-level service for activity management
     *
     * Provides convenient methods for listing, retrieving, deleting,
     * and exporting activities from the watch.
     */
    class ActivityService
    {
    public:
        /**
         * @brief Construct activity service
         * @param file_service Low-level file service for watch communication
         */
        explicit ActivityService(
            std::shared_ptr<services::FileService> file_service);

        // ====================================================================
        // Activity Listing and Retrieval
        // ====================================================================

        /**
         * @brief List all activities on the watch
         * @return Vector of activity metadata (lightweight info)
         */
        std::vector<models::ActivityInfo> list();

        /**
         * @brief Get full activity data by index
         * @param index Activity index (0-based)
         * @return Parsed activity with all records
         */
        models::Activity get(uint16_t index);

        /**
         * @brief Get full activity data by file ID
         * @param file_id The file ID
         * @return Parsed activity with all records
         */
        models::Activity get(FileId file_id);

        /**
         * @brief Get activity count
         * @return Number of activities on watch
         */
        size_t count();

        // ====================================================================
        // Activity Management
        // ====================================================================

        /**
         * @brief Delete an activity by index
         * @param index Activity index
         */
        void remove(uint16_t index);

        /**
         * @brief Delete an activity by file ID
         * @param file_id File ID
         */
        void remove(FileId file_id);

        /**
         * @brief Delete all activities
         * @return Number of activities deleted
         */
        size_t removeAll();

        // ====================================================================
        // Export Functions
        // ====================================================================

        /**
         * @brief Export activity to GPX format
         * @param activity The activity to export
         * @return GPX XML string
         */
        std::string exportToGPX(const models::Activity &activity);

        /**
         * @brief Export activity to TCX format
         * @param activity The activity to export
         * @return TCX XML string
         */
        std::string exportToTCX(const models::Activity &activity);

        /**
         * @brief Export activity to CSV format
         * @param activity The activity to export
         * @return CSV string
         */
        std::string exportToCSV(const models::Activity &activity);

        // ====================================================================
        // Utility Functions
        // ====================================================================

        /**
         * @brief Get the next available activity file ID
         * @return File ID for next activity
         */
        FileId getNextActivityFileId();

        /**
         * @brief Check if an activity exists
         * @param index Activity index
         * @return true if activity exists
         */
        bool exists(uint16_t index);

        /**
         * @brief Check if an activity exists
         * @param file_id File ID
         * @return true if activity exists
         */
        bool exists(FileId file_id);

    private:
        std::shared_ptr<services::FileService> file_service_;

        // Helper to convert index to FileId
        FileId indexToFileId(uint16_t index) const;

        // Helper to build metadata from activity file
        models::ActivityInfo buildActivityInfo(FileId file_id, const std::vector<uint8_t> &data);

        // Helper to convert activity records to track points
        std::vector<common::TrackPoint> buildTrackPoints(const models::Activity &activity);

        // Helper to build activity metadata for export
        common::ActivityMetadata buildExportMetadata(const models::Activity &activity);
    };

} // namespace tomtom::services::activity
