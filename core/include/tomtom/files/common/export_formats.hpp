// ============================================================================
// export_formats.hpp - Export utilities for GPX, TCX, and other formats
// ============================================================================
#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace tomtom::files::common
{

    /**
     * @brief GPS coordinate point
     */
    struct GPSPoint
    {
        double latitude;       // Degrees
        double longitude;      // Degrees
        double altitude;       // Meters (optional, can be 0)
        std::time_t timestamp; // Unix timestamp
    };

    /**
     * @brief Track point with additional metrics
     */
    struct TrackPoint : GPSPoint
    {
        double speed;        // m/s (optional)
        double distance;     // Cumulative distance in meters (optional)
        uint16_t heart_rate; // BPM (optional, 0 if not available)
        uint16_t cadence;    // Steps/min or RPM (optional, 0 if not available)
        uint16_t calories;   // Cumulative calories (optional)
    };

    /**
     * @brief Activity metadata for exports
     */
    struct ActivityMetadata
    {
        std::string sport_type; // "Running", "Cycling", etc.
        std::time_t start_time;
        uint32_t duration_seconds;
        double total_distance; // Meters
        uint16_t total_calories;
        uint16_t avg_heart_rate; // Optional
        uint16_t max_heart_rate; // Optional
    };

    // ========================================================================
    // GPX Export
    // ========================================================================

    /**
     * @brief Export track points to GPX 1.1 format
     * @param metadata Activity metadata
     * @param points Track points
     * @return GPX XML string
     */
    inline std::string exportToGPX(
        const ActivityMetadata &metadata,
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
                      std::gmtime(&metadata.start_time));
        gpx << "    <time>" << time_buf << "</time>\n";
        gpx << "  </metadata>\n";

        // Track
        gpx << "  <trk>\n";
        gpx << "    <name>" << metadata.sport_type << "</name>\n";
        gpx << "    <type>" << metadata.sport_type << "</type>\n";
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

    // ========================================================================
    // TCX Export
    // ========================================================================

    /**
     * @brief Export track points to TCX format (Training Center XML)
     * @param metadata Activity metadata
     * @param points Track points
     * @return TCX XML string
     */
    inline std::string exportToTCX(
        const ActivityMetadata &metadata,
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

        tcx << "  <Activities>\n";
        tcx << "    <Activity Sport=\"" << metadata.sport_type << "\">\n";

        char time_buf[32];
        std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ",
                      std::gmtime(&metadata.start_time));
        tcx << "      <Id>" << time_buf << "</Id>\n";

        // Lap (single lap for entire activity)
        tcx << "      <Lap StartTime=\"" << time_buf << "\">\n";
        tcx << "        <TotalTimeSeconds>" << metadata.duration_seconds << "</TotalTimeSeconds>\n";
        tcx << "        <DistanceMeters>" << std::fixed << std::setprecision(2)
            << metadata.total_distance << "</DistanceMeters>\n";
        tcx << "        <Calories>" << metadata.total_calories << "</Calories>\n";

        if (metadata.avg_heart_rate > 0)
        {
            tcx << "        <AverageHeartRateBpm><Value>"
                << metadata.avg_heart_rate << "</Value></AverageHeartRateBpm>\n";
        }

        if (metadata.max_heart_rate > 0)
        {
            tcx << "        <MaximumHeartRateBpm><Value>"
                << metadata.max_heart_rate << "</Value></MaximumHeartRateBpm>\n";
        }

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

            if (point.distance > 0)
            {
                tcx << "            <DistanceMeters>" << std::fixed << std::setprecision(2)
                    << point.distance << "</DistanceMeters>\n";
            }

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

    // ========================================================================
    // CSV Export
    // ========================================================================

    /**
     * @brief Export track points to CSV format
     * @param points Track points
     * @return CSV string
     */
    inline std::string exportToCSV(const std::vector<TrackPoint> &points)
    {
        std::ostringstream csv;

        // Header
        csv << "timestamp,latitude,longitude,altitude,speed,distance,heart_rate,cadence,calories\n";

        // Data rows
        for (const auto &point : points)
        {
            char time_buf[32];
            std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S",
                          std::gmtime(&point.timestamp));

            csv << time_buf << ","
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

} // namespace tomtom::files::common
