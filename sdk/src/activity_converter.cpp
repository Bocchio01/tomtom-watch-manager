#include "tomtom/sdk/activity_converter.hpp"
#include "tomtom/sdk/export_formats.hpp"
#include <tomtom/services/activity/activity_types.hpp>
#include <algorithm>
#include <map>
#include <stdexcept>

namespace tomtom::sdk
{

    namespace
    {
        // Helper to convert Activity records to common::TrackPoint format
        std::vector<tomtom::services::common::TrackPoint> buildTrackPoints(
            const tomtom::services::activity::models::Activity &activity)
        {
            using namespace tomtom::services;

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

        // Helper to build activity metadata for export
        tomtom::services::common::ActivityMetadata buildExportMetadata(
            const tomtom::services::activity::models::Activity &activity)
        {
            using namespace tomtom::services;

            common::ActivityMetadata metadata;
            metadata.sport_type = std::string(activity::toString(activity.type));
            metadata.start_time = activity.start_time;
            metadata.duration_seconds = activity.duration_seconds;
            metadata.total_distance = activity.distance_meters;
            metadata.total_calories = activity.calories;

            return metadata;
        }
    }

    std::string ActivityConverter::toGPX(const tomtom::services::activity::models::Activity &activity)
    {
        auto track_points = buildTrackPoints(activity);
        auto metadata = buildExportMetadata(activity);

        return tomtom::services::common::exportToGPX(metadata, track_points);
    }

    std::string ActivityConverter::toTCX(const tomtom::services::activity::models::Activity &activity)
    {
        auto track_points = buildTrackPoints(activity);
        auto metadata = buildExportMetadata(activity);

        return tomtom::services::common::exportToTCX(metadata, track_points);
    }

    std::string ActivityConverter::toCSV(const tomtom::services::activity::models::Activity &activity)
    {
        auto track_points = buildTrackPoints(activity);
        return tomtom::services::common::exportToCSV(track_points);
    }

    std::string ActivityConverter::getExtension(ExportFormat format)
    {
        switch (format)
        {
        case ExportFormat::GPX:
            return ".gpx";
        case ExportFormat::TCX:
            return ".tcx";
        case ExportFormat::CSV:
            return ".csv";
        default:
            throw std::invalid_argument("Unknown export format");
        }
    }

    ExportFormat ActivityConverter::parseFormat(const std::string &format_str)
    {
        std::string lower = format_str;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower == "gpx")
            return ExportFormat::GPX;
        if (lower == "tcx")
            return ExportFormat::TCX;
        if (lower == "csv")
            return ExportFormat::CSV;

        throw std::invalid_argument("Unknown format: " + format_str);
    }

} // namespace tomtom::sdk
