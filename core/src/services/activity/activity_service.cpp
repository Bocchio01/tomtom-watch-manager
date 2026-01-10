// ============================================================================
// activity_service.cpp - High-level activity operations implementation
// ============================================================================

#include "tomtom/services/activity/activity_service.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <map>

namespace tomtom::services::activity
{

    ActivityService::ActivityService(std::shared_ptr<services::FileService> file_service)
        : file_service_(std::move(file_service)) {}

    std::vector<models::ActivityInfo> ActivityService::list()
    {
        std::vector<models::ActivityInfo> activities;

        auto files = file_service_->listFiles();
        for (const auto &file : files)
        {
            spdlog::info("  File ID: 0x{:08X}, Size: {} bytes", file.id.value, file.size);
            // Activity files range from 0x00910000 to 0x0091FFFF
            if ((file.id.value & 0xFFFF0000) == 0x00910000)
            {
                try
                {
                    auto data = file_service_->readFile(file.id);
                    auto info = buildActivityInfo(file.id, data);
                    activities.push_back(info);
                }
                catch (const std::exception &e)
                {
                    spdlog::warn("Failed to read activity file 0x{:08X}: {}",
                                 file.id.value, e.what());
                }
            }
        }

        spdlog::info("Found {} activities", activities.size());
        return activities;
    }

    models::Activity ActivityService::get(uint16_t index)
    {
        return get(indexToFileId(index));
    }

    models::Activity ActivityService::get(FileId file_id)
    {
        spdlog::debug("Reading activity file 0x{:08X}", file_id.value);

        auto data = file_service_->readFile(file_id);
        ActivityParser parser;
        return parser.parse(data);
    }

    size_t ActivityService::count()
    {
        return list().size();
    }

    void ActivityService::remove(uint16_t index)
    {
        remove(indexToFileId(index));
    }

    void ActivityService::remove(FileId file_id)
    {
        spdlog::info("Deleting activity file 0x{:08X}", file_id.value);
        file_service_->deleteFile(file_id);
    }

    size_t ActivityService::removeAll()
    {
        auto activities = list();
        size_t count = activities.size();

        for (const auto &activity : activities)
        {
            try
            {
                remove(activity.file_id);
            }
            catch (const std::exception &e)
            {
                spdlog::warn("Failed to delete activity {}: {}",
                             activity.index, e.what());
            }
        }

        spdlog::info("Deleted {} activities", count);
        return count;
    }

    std::string ActivityService::exportToGPX(const models::Activity &activity)
    {
        auto track_points = buildTrackPoints(activity);
        auto metadata = buildExportMetadata(activity);

        return common::exportToGPX(metadata, track_points);
    }

    std::string ActivityService::exportToTCX(const models::Activity &activity)
    {
        auto track_points = buildTrackPoints(activity);
        auto metadata = buildExportMetadata(activity);

        return common::exportToTCX(metadata, track_points);
    }

    std::string ActivityService::exportToCSV(const models::Activity &activity)
    {
        auto track_points = buildTrackPoints(activity);
        return common::exportToCSV(track_points);
    }

    FileId ActivityService::getNextActivityFileId()
    {
        // Find the highest activity index
        uint16_t max_index = 0;

        for (uint16_t i = 0; i < 0xFFFF; ++i)
        {
            if (exists(i))
            {
                max_index = i;
            }
        }

        return indexToFileId(max_index + 1);
    }

    bool ActivityService::exists(uint16_t index)
    {
        return exists(indexToFileId(index));
    }

    bool ActivityService::exists(FileId file_id)
    {
        try
        {
            auto size = file_service_->getFileSize(file_id);
            return size > 0;
        }
        catch (...)
        {
            return false;
        }
    }

    FileId ActivityService::indexToFileId(uint16_t index) const
    {
        return activityFile(index);
    }

    models::ActivityInfo ActivityService::buildActivityInfo(FileId file_id, const std::vector<uint8_t> &data)
    {
        // Parse the activity to extract info
        ActivityParser parser;
        auto activity = parser.parse(data);

        models::ActivityInfo info;
        info.index = getActivityIndex(file_id);
        info.file_id = file_id;
        info.type = activity.type;
        info.start_time = activity.start_time;
        info.duration_seconds = activity.duration_seconds;
        info.distance_meters = activity.distance_meters;
        info.file_size = static_cast<uint32_t>(data.size());

        return info;
    }

    std::vector<common::TrackPoint> ActivityService::buildTrackPoints(
        const models::Activity &activity)
    {
        std::vector<common::TrackPoint> points;

        auto gps_records = activity.getGPSRecords();
        auto hr_records = activity.getHeartRateRecords();

        // Build map of timestamp -> heart rate for quick lookup
        std::map<uint32_t, uint16_t> hr_map;
        for (const auto *hr : hr_records)
        {
            hr_map[hr->timestamp] = hr->heart_rate;
        }

        // Convert GPS records to track points
        for (const auto *gps : gps_records)
        {
            common::TrackPoint point;
            point.latitude = gps->getLatitudeDegrees();
            point.longitude = gps->getLongitudeDegrees();
            point.altitude = 0; // TODO: Extract from altitude records
            point.timestamp = static_cast<std::time_t>(gps->timestamp);
            point.speed = gps->filtered_speed;
            point.distance = gps->distance;
            point.cadence = gps->cycles;
            point.calories = gps->calories;

            // Find matching heart rate
            auto hr_it = hr_map.find(gps->timestamp);
            point.heart_rate = (hr_it != hr_map.end()) ? hr_it->second : 0;

            points.push_back(point);
        }

        return points;
    }

    common::ActivityMetadata ActivityService::buildExportMetadata(
        const models::Activity &activity)
    {
        common::ActivityMetadata metadata;
        metadata.sport_type = std::string(toString(activity.type));
        metadata.start_time = activity.start_time;
        metadata.duration_seconds = activity.duration_seconds;
        metadata.total_distance = activity.distance_meters;
        metadata.total_calories = activity.calories;
        metadata.avg_heart_rate = activity.heart_rate_avg.value_or(0);
        metadata.max_heart_rate = activity.heart_rate_max.value_or(0);

        return metadata;
    }

} // namespace tomtom::services::activity
