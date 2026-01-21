#pragma once

#include <memory>
#include <vector>
#include <functional>

#include <tomtom/core/watch.hpp>

#include "tomtom/sdk/utils/files_identifier.hpp"
#include "tomtom/sdk/models/activity_model.hpp"
#include "tomtom/sdk/parsers/activity_parser.hpp"
#include "tomtom/sdk/store/local_store.hpp"
#include "tomtom/sdk/converters/activity_converter.hpp"

namespace tomtom::sdk::services
{
    /**
     * @brief Progress callback for activity operations
     */
    using ProgressCallback = std::function<void(size_t current, size_t total, const std::string &message)>;

    /**
     * @brief Service for activity/workout management
     *
     * Handles listing, downloading, and deleting activities from the watch.
     * Integrates with LocalStore for automatic persistence and exports.
     */
    class ActivityService
    {
    public:
        /**
         * @brief Construct activity service
         * @param watch Watch instance for communication
         * @param store Optional local store for persistence (null = no auto-save)
         */
        explicit ActivityService(std::shared_ptr<core::Watch> watch,
                                 std::shared_ptr<store::LocalStore> store = nullptr);

        /**
         * @brief List all activities on the watch
         *
         * @return Vector of file IDs for activities
         */
        std::vector<utils::FileId> listActivities();

        /**
         * @brief Download and parse an activity
         *
         * If a LocalStore is configured, the raw TTBIN data will be automatically
         * saved to the archive directory for future reference.
         *
         * @param fileId Activity file ID (0x0083xxxx)
         * @param progress Optional progress callback
         * @return Parsed activity
         */
        std::unique_ptr<models::Activity> downloadActivity(utils::FileId fileId, ProgressCallback progress = nullptr);

        /**
         * @brief Download all activities
         */
        std::vector<std::unique_ptr<models::Activity>> downloadAllActivities(ProgressCallback progress = nullptr);

        /**
         * @brief Delete an activity from the watch
         */
        void deleteActivity(utils::FileId fileId);

        /**
         * @brief Delete all activities from the watch
         */
        void deleteAllActivities(ProgressCallback progress = nullptr);

        /**
         * @brief Export an activity using a specific converter
         *
         * If a LocalStore is configured, the exported file will be saved to the
         * configured export directory.
         *
         * @param activity Activity to export
         * @param converter Converter to use (e.g., GpxConverter, KmlConverter)
         * @return Exported content as string
         */
        std::string exportActivity(const models::Activity &activity,
                                   converters::ActivityConverter &converter);

        /**
         * @brief Export and save activity to local store
         *
         * Saves the exported activity to the configured directory layout.
         * Requires LocalStore to be configured.
         *
         * @param activity Activity to export
         * @param converter Converter to use
         * @return Path to saved file
         * @throws std::runtime_error if no LocalStore is configured
         */
        std::filesystem::path exportAndSaveActivity(const models::Activity &activity,
                                                    converters::ActivityConverter &converter);

    private:
        std::shared_ptr<core::Watch> watch_;
        std::shared_ptr<store::LocalStore> store_;
        parsers::ActivityParser parser_;
    };

}
