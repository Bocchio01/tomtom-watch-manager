#include <algorithm>
#include <map>
#include <spdlog/spdlog.h>

#include "tomtom/services/files/files.hpp"
#include "tomtom/services/activity/activity_service.hpp"

namespace tomtom::services::activity
{

    ActivityService::ActivityService(std::shared_ptr<services::files::FileService> file_service)
        : file_service_(std::move(file_service)) {}

    std::vector<models::Activity> ActivityService::list()
    {
        std::vector<models::Activity> activities;

        auto files = listActivityFiles();
        for (const auto &file : files)
        {
            try
            {
                auto data = file_service_->readFile(file.id);
                auto activity = parser_.parse(data);
                activities.push_back(activity);
            }
            catch (const std::exception &e)
            {
                spdlog::warn("Failed to read activity file 0x{:08X}: {}", file.id.value, e.what());
            }
        }

        return activities;
    }

    models::Activity ActivityService::get(uint16_t index)
    {
        return get(files::activityFile(index));
    }

    models::Activity ActivityService::get(files::FileId file_id)
    {
        auto data = file_service_->readFile(file_id);
        return parser_.parse(data);
    }

    size_t ActivityService::count()
    {
        return listActivityFiles().size();
    }

    void ActivityService::remove(uint16_t index)
    {
        remove(files::activityFile(index));
    }

    void ActivityService::remove(files::FileId file_id)
    {
        file_service_->deleteFile(file_id);
    }

    size_t ActivityService::removeAll()
    {
        size_t deleted_count = 0;
        auto files = listActivityFiles();

        for (const auto &file : files)
        {
            try
            {
                remove(file.id);
                deleted_count++;
            }
            catch (const std::exception &e)
            {
                spdlog::warn("Failed to delete activity {}: {}", file.id.value, e.what());
            }
        }

        return deleted_count;
    }

    bool ActivityService::exists(uint16_t index)
    {
        return exists(files::activityFile(index));
    }

    bool ActivityService::exists(files::FileId file_id)
    {
        return file_service_->fileExists(file_id);
    }

    std::vector<files::FileEntry> ActivityService::listActivityFiles()
    {
        std::vector<files::FileEntry> activity_files;
        auto files = file_service_->listFiles();
        for (const auto &file : files)
        {
            if (files::isActivityFile(file.id))
            {
                activity_files.push_back(file);
            }
        }

        return activity_files;
    }

}
