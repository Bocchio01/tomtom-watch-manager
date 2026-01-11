#include "tomtom/sdk/activity_exporter.hpp"
#include <tomtom/services/activity/activity_models.hpp>
#include <tomtom/services/activity/records/gps_record.hpp>
#include <tomtom/services/activity/records/heart_rate_record.hpp>
#include <tomtom/services/activity/activity_types.hpp>
#include <algorithm>
#include <map>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace tomtom::sdk
{

    namespace
    {
        // Internal track point structure
        struct TrackPoint
        {
            double latitude;
            double longitude;
            double altitude;
            std::time_t timestamp;
            double speed;
            double distance;
            uint16_t heart_rate;
            uint16_t cadence;
            uint16_t calories;
        };

        // Helper to build track points directly from Activity records
        std::vector<TrackPoint> buildTrackPoints(
            const tomtom::services::activity::models::Activity &activity)
        {
            using namespace tomtom::services;

            std::vector<TrackPoint> points;

            // Extract GPS and heart rate records from activity
            std::vector<const activity::records::GPSRecord *> gps_records;
            std::vector<const activity::records::HeartRateRecord *> hr_records;

            for (const auto &record : activity.records)
            {
                if (auto *gps = dynamic_cast<activity::records::GPSRecord *>(record.get()))
                {
                    gps_records.push_back(gps);
                }
                else if (auto *hr = dynamic_cast<activity::records::HeartRateRecord *>(record.get()))
                {
                    hr_records.push_back(hr);
                }
            }

            // Build map of timestamp -> heart rate for quick lookup
            std::map<uint32_t, uint16_t> hr_map;
            for (const auto *hr : hr_records)
            {
                hr_map[hr->timestamp] = hr->heart_rate;
            }

            // Convert GPS records to track points
            for (const auto *gps : gps_records)
            {
                TrackPoint point;
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

        // Convert activity to GPX format
        std::string convertToGPX(
            const tomtom::services::activity::models::Activity &activity,
            const std::vector<TrackPoint> &points)
        {
            std::ostringstream gpx;

            // GPX header
            gpx << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
            gpx << "<gpx version=\"1.1\" creator=\"TomTom Watch Manager\"\n";
            gpx << "  xmlns=\"http://www.topografix.com/GPX/1/1\"\n";
            gpx << "  xmlns:gpxtpx=\"http://www.garmin.com/xmlschemas/TrackPointExtension/v1\"\n";
            gpx << "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n";
            gpx << "  xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 ";
            gpx << "http://www.topografix.com/GPX/1/1/gpx.xsd\">\n";

            // Metadata
            gpx << "  <metadata>\n";
            char time_buf[32];
            std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ",
                          std::gmtime(&activity.start_time));
            gpx << "    <time>" << time_buf << "</time>\n";
            gpx << "  </metadata>\n";

            // Track
            std::string sport_type = std::string(tomtom::services::activity::toString(activity.type));
            gpx << "  <trk>\n";
            gpx << "    <name>" << sport_type << "</name>\n";
            gpx << "    <type>" << sport_type << "</type>\n";
            gpx << "    <trkseg>\n";

            // Track points
            for (const auto &point : points)
            {
                gpx << "      <trkpt lat=\"" << std::fixed << std::setprecision(7)
                    << point.latitude << "\" lon=\"" << point.longitude << "\">\n";

                if (point.altitude != 0)
                {
                    gpx << "        <ele>" << std::fixed << std::setprecision(1)
                        << point.altitude << "</ele>\n";
                }

                std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ",
                              std::gmtime(&point.timestamp));
                gpx << "        <time>" << time_buf << "</time>\n";

                // Extensions (heart rate, cadence, speed)
                if (point.heart_rate > 0 || point.cadence > 0 || point.speed > 0)
                {
                    gpx << "        <extensions>\n";
                    gpx << "          <gpxtpx:TrackPointExtension>\n";

                    if (point.heart_rate > 0)
                    {
                        gpx << "            <gpxtpx:hr>" << point.heart_rate << "</gpxtpx:hr>\n";
                    }

                    if (point.cadence > 0)
                    {
                        gpx << "            <gpxtpx:cad>" << point.cadence << "</gpxtpx:cad>\n";
                    }

                    if (point.speed > 0)
                    {
                        gpx << "            <gpxtpx:speed>" << std::fixed << std::setprecision(2)
                            << point.speed << "</gpxtpx:speed>\n";
                    }

                    gpx << "          </gpxtpx:TrackPointExtension>\n";
                    gpx << "        </extensions>\n";
                }

                gpx << "      </trkpt>\n";
            }

            gpx << "    </trkseg>\n";
            gpx << "  </trk>\n";
            gpx << "</gpx>\n";

            return gpx.str();
        }

        // Convert activity to TCX format
        std::string convertToTCX(
            const tomtom::services::activity::models::Activity &activity,
            const std::vector<TrackPoint> &points)
        {
            std::ostringstream tcx;

            // TCX header
            tcx << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
            tcx << "<TrainingCenterDatabase\n";
            tcx << "  xmlns=\"http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2\"\n";
            tcx << "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n";
            tcx << "  xsi:schemaLocation=\"http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2 ";
            tcx << "http://www.garmin.com/xmlschemas/TrainingCenterDatabasev2.xsd\">\n";

            std::string sport_type = std::string(tomtom::services::activity::toString(activity.type));
            tcx << "  <Activities>\n";
            tcx << "    <Activity Sport=\"" << sport_type << "\">\n";

            char time_buf[32];
            std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ",
                          std::gmtime(&activity.start_time));
            tcx << "      <Id>" << time_buf << "</Id>\n";

            // Lap (single lap for entire activity)
            tcx << "      <Lap StartTime=\"" << time_buf << "\">\n";
            tcx << "        <TotalTimeSeconds>" << activity.duration_seconds << "</TotalTimeSeconds>\n";
            tcx << "        <DistanceMeters>" << std::fixed << std::setprecision(2)
                << activity.distance_meters << "</DistanceMeters>\n";
            tcx << "        <Calories>" << activity.calories << "</Calories>\n";
            tcx << "        <Intensity>Active</Intensity>\n";
            tcx << "        <TriggerMethod>Manual</TriggerMethod>\n";

            // Track points
            tcx << "        <Track>\n";

            for (const auto &point : points)
            {
                tcx << "          <Trackpoint>\n";

                std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ",
                              std::gmtime(&point.timestamp));
                tcx << "            <Time>" << time_buf << "</Time>\n";

                tcx << "            <Position>\n";
                tcx << "              <LatitudeDegrees>" << std::fixed << std::setprecision(7)
                    << point.latitude << "</LatitudeDegrees>\n";
                tcx << "              <LongitudeDegrees>" << point.longitude << "</LongitudeDegrees>\n";
                tcx << "            </Position>\n";

                if (point.altitude != 0)
                {
                    tcx << "            <AltitudeMeters>" << std::fixed << std::setprecision(1)
                        << point.altitude << "</AltitudeMeters>\n";
                }

                tcx << "            <DistanceMeters>" << std::fixed << std::setprecision(2)
                    << point.distance << "</DistanceMeters>\n";

                if (point.heart_rate > 0)
                {
                    tcx << "            <HeartRateBpm><Value>"
                        << point.heart_rate << "</Value></HeartRateBpm>\n";
                }

                if (point.cadence > 0)
                {
                    tcx << "            <Cadence>" << point.cadence << "</Cadence>\n";
                }

                tcx << "          </Trackpoint>\n";
            }

            tcx << "        </Track>\n";
            tcx << "      </Lap>\n";
            tcx << "    </Activity>\n";
            tcx << "  </Activities>\n";
            tcx << "</TrainingCenterDatabase>\n";

            return tcx.str();
        }

        // Convert activity to CSV format
        std::string convertToCSV(const std::vector<TrackPoint> &points)
        {
            std::ostringstream csv;

            // CSV header
            csv << "timestamp,latitude,longitude,altitude,speed,distance,heart_rate,cadence,calories\n";

            // Data rows
            for (const auto &point : points)
            {
                csv << point.timestamp << ","
                    << std::fixed << std::setprecision(7) << point.latitude << ","
                    << point.longitude << ","
                    << std::setprecision(1) << point.altitude << ","
                    << std::setprecision(2) << point.speed << ","
                    << point.distance << ","
                    << point.heart_rate << ","
                    << point.cadence << ","
                    << point.calories << "\n";
            }

            return csv.str();
        }
    }

    std::string ActivityExporter::exportFile(
        const tomtom::services::activity::models::Activity &activity,
        Format format)
    {
        switch (format)
        {
        case Format::GPX:
            return toGPX(activity);
        case Format::TCX:
            return toTCX(activity);
        case Format::CSV:
            return toCSV(activity);
        default:
            throw std::invalid_argument("Unknown export format");
        }
    }

    std::string ActivityExporter::toGPX(const tomtom::services::activity::models::Activity &activity)
    {
        auto track_points = buildTrackPoints(activity);
        return convertToGPX(activity, track_points);
    }

    std::string ActivityExporter::toTCX(const tomtom::services::activity::models::Activity &activity)
    {
        auto track_points = buildTrackPoints(activity);
        return convertToTCX(activity, track_points);
    }

    std::string ActivityExporter::toCSV(const tomtom::services::activity::models::Activity &activity)
    {
        auto track_points = buildTrackPoints(activity);
        return convertToCSV(track_points);
    }

    std::string ActivityExporter::getExtension(Format format)
    {
        switch (format)
        {
        case Format::GPX:
            return ".gpx";
        case Format::TCX:
            return ".tcx";
        case Format::CSV:
            return ".csv";
        default:
            throw std::invalid_argument("Unknown export format");
        }
    }

    ActivityExporter::Format ActivityExporter::parseFormat(const std::string &format_str)
    {
        std::string lower = format_str;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower == "gpx")
            return Format::GPX;
        if (lower == "tcx")
            return Format::TCX;
        if (lower == "csv")
            return Format::CSV;

        throw std::invalid_argument("Unknown format: " + format_str);
    }

} // namespace tomtom::sdk
