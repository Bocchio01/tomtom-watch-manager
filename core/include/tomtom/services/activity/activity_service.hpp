#pragma once

#include <memory>
#include <vector>
#include <string>

#include "tomtom/services/files/files.hpp"

#include "activity_models.hpp"
#include "activity_parser.hpp"

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
        explicit ActivityService(std::shared_ptr<services::files::FileService> file_service);

        /**
         * @brief List all activities on the watch
         * @return Vector of activity metadata (lightweight info)
         */
        std::vector<models::Activity> list();

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
        models::Activity get(files::FileId file_id);

        /**
         * @brief Get activity count
         * @return Number of activities on watch
         */
        size_t count();

        /**
         * @brief Delete an activity by index
         * @param index Activity index
         */
        void remove(uint16_t index);

        /**
         * @brief Delete an activity by file ID
         * @param file_id File ID
         */
        void remove(files::FileId file_id);

        /**
         * @brief Delete all activities
         * @return Number of activities deleted
         */
        size_t removeAll();

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
        bool exists(files::FileId file_id);

    private:
        std::shared_ptr<services::files::FileService> file_service_;
        ActivityParser parser_;

        /**
         * @brief List all activity file IDs on the watch
         * @return Vector of file IDs
         */
        std::vector<files::FileEntry> listActivityFiles();
    };

}
