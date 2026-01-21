#include <algorithm>
#include <spdlog/spdlog.h>
#include <filesystem>

#include <tomtom/core/watch.hpp>

#include "tomtom/sdk/utils/files_identifier.hpp"
#include "tomtom/sdk/services/activity_service.hpp"
#include "tomtom/sdk/parsers/activity_parser.hpp"

using namespace tomtom::sdk::utils;
using namespace tomtom::sdk::models;

namespace tomtom::sdk::services
{

    ActivityService::ActivityService(std::shared_ptr<core::Watch> watch,
                                     std::shared_ptr<store::LocalStore> store)
        : watch_(std::move(watch)), store_(std::move(store)) {}

    std::vector<FileId> ActivityService::listActivities()
    {
        auto entries = watch_->files().listFiles();
        std::vector<FileId> fileIds;
        fileIds.reserve(entries.size());

        for (const auto &entry : entries)
        {
            FileId fileId = (FileId)entry.id;
            if (isActivityFile(fileId))
            {
                fileIds.push_back(fileId);
            }
        }

        return fileIds;
    }

    std::unique_ptr<Activity> ActivityService::downloadActivity(FileId fileId, ProgressCallback progress)
    {
        if (progress)
        {
            progress(0, 1, "Downloading activity...");
        }

        auto data = watch_->files().readFile(fileId.value);

        auto activity = std::make_unique<Activity>(parser_.parse(data));

        // Save raw activity data to local store if configured
        if (store_ && !store_->hasRawActivity(*activity))
        {
            spdlog::debug("Saving raw activity to local store");
            try
            {
                store_->saveRawActivity(*activity, data);
            }
            catch (const std::exception &e)
            {
                spdlog::warn("Failed to save raw activity to local store: {}", e.what());
            }
        }

        if (progress)
        {
            progress(1, 1, "Download complete");
        }

        return activity;
    }

    std::vector<std::unique_ptr<Activity>> ActivityService::downloadAllActivities(ProgressCallback progress)
    {
        auto fileIds = listActivities();
        std::vector<std::unique_ptr<Activity>> activities;
        activities.reserve(fileIds.size());

        for (size_t i = 0; i < fileIds.size(); ++i)
        {
            if (progress)
            {
                progress(i, fileIds.size(), "Downloading activities...");
            }

            try
            {
                activities.push_back(downloadActivity(fileIds[i], progress));
            }
            catch (const std::exception &e)
            {
                spdlog::warn("Failed to download activity 0x{:08X}: {}", fileIds[i].value, e.what());
            }
        }

        return activities;
    }

    void ActivityService::deleteActivity(FileId fileId)
    {
        watch_->files().deleteFile(fileId.value);
    }

    void ActivityService::deleteAllActivities(ProgressCallback progress)
    {
        auto fileIds = listActivities();

        for (size_t i = 0; i < fileIds.size(); ++i)
        {
            if (progress)
            {
                progress(i, fileIds.size(), "Deleting activities...");
            }

            try
            {
                deleteActivity(fileIds[i]);
            }
            catch (const std::exception &e)
            {
                spdlog::warn("Failed to delete activity 0x{:08X}: {}", fileIds[i].value, e.what());
            }
        }
    }

    std::string ActivityService::exportActivity(const models::Activity &activity,
                                                converters::ActivityConverter &converter)
    {
        spdlog::debug("Exporting activity using {} converter", converter.getExtension());
        return converter.convert(activity);
    }

    std::filesystem::path ActivityService::exportAndSaveActivity(
        const models::Activity &activity,
        converters::ActivityConverter &converter)
    {
        if (!store_)
        {
            throw std::runtime_error("LocalStore not configured for ActivityService");
        }

        spdlog::debug("Exporting and saving activity as {}", converter.getExtension());
        std::string content = converter.convert(activity);
        return store_->saveExportedActivity(activity, content, converter.getExtension());
    }

}
