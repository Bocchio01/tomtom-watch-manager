#include "tomtom/sdk/converters/gpx_converter.hpp"
#include "tomtom/sdk/models/activity_records/gps_record.hpp"
#include "tomtom/sdk/models/activity_records/heart_rate_record.hpp"
#include "tomtom/sdk/models/activity_records/altitude_record.hpp"

#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <ctime>
#include <map>

namespace tomtom::sdk::converters
{

    std::string GpxConverter::convert(const models::Activity &activity)
    {
        std::ostringstream gpx;

        // XML header and GPX root element
        gpx << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        gpx << "<gpx version=\"1.1\" creator=\"TomTom Watch Manager\"\n";
        gpx << "  xmlns=\"http://www.topografix.com/GPX/1/1\"\n";
        gpx << "  xmlns:gpxtpx=\"http://www.garmin.com/xmlschemas/TrackPointExtension/v1\"\n";
        gpx << "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n";
        gpx << "  xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1\n";
        gpx << "    http://www.topografix.com/GPX/1/1/gpx.xsd\n";
        gpx << "    http://www.garmin.com/xmlschemas/TrackPointExtension/v1\n";
        gpx << "    http://www.garmin.com/xmlschemas/TrackPointExtensionv1.xsd\">\n\n";

        // Metadata
        gpx << "  <metadata>\n";
        gpx << "    <name>" << escapeXml(getActivityName(activity)) << "</name>\n";
        gpx << "    <time>" << formatTimestamp(activity.start_time) << "</time>\n";
        gpx << "  </metadata>\n\n";

        // Track
        gpx << "  <trk>\n";
        gpx << "    <name>" << escapeXml(getActivityName(activity)) << "</name>\n";
        gpx << "    <type>" << toString(activity.type) << "</type>\n";
        gpx << "    <trkseg>\n";

        // Collect all GPS records with their timestamps for synchronization
        std::vector<const models::GPSRecord *> gpsRecords;
        for (const auto &record : activity.records)
        {
            if (record->tag == models::RecordTag::GPS)
            {
                auto *gpsRec = dynamic_cast<models::GPSRecord *>(record.get());
                if (gpsRec && gpsRec->timestamp != 0xFFFFFFFF)
                {
                    gpsRecords.push_back(gpsRec);
                }
            }
        }

        if (gpsRecords.empty())
        {
            throw std::runtime_error("Activity has no GPS data");
        }

        // Create maps for heart rate and altitude data by timestamp
        std::map<uint32_t, uint8_t> heartRateMap;
        int16_t currentAltitude = 0;

        for (const auto &record : activity.records)
        {
            if (record->tag == models::RecordTag::HeartRate)
            {
                auto *hrRec = dynamic_cast<models::HeartRateRecord *>(record.get());
                if (hrRec)
                {
                    heartRateMap[hrRec->timestamp] = hrRec->heart_rate;
                }
            }
            else if (record->tag == models::RecordTag::AltitudeUpdate)
            {
                auto *altRec = dynamic_cast<models::AltitudeRecord *>(record.get());
                if (altRec)
                {
                    currentAltitude = altRec->rel_altitude;
                }
            }
        }

        // Generate track points
        for (const auto *gpsRec : gpsRecords)
        {
            gpx << "      <trkpt lat=\"" << std::fixed << std::setprecision(7)
                << gpsRec->getLatitudeDegrees() << "\" lon=\""
                << gpsRec->getLongitudeDegrees() << "\">\n";

            // Elevation (altitude) - use current altitude if available
            if (currentAltitude != 0)
            {
                gpx << "        <ele>" << std::fixed << std::setprecision(1)
                    << (currentAltitude / 100.0) << "</ele>\n";
            }

            // Timestamp
            gpx << "        <time>" << formatTimestamp(gpsRec->timestamp) << "</time>\n";

            // Extensions for heart rate, cadence, etc.
            bool hasExtensions = false;
            auto hrIt = heartRateMap.find(gpsRec->timestamp);

            if (hrIt != heartRateMap.end() || gpsRec->cycles > 0)
            {
                if (!hasExtensions)
                {
                    gpx << "        <extensions>\n";
                    gpx << "          <gpxtpx:TrackPointExtension>\n";
                    hasExtensions = true;
                }

                if (hrIt != heartRateMap.end())
                {
                    gpx << "            <gpxtpx:hr>" << static_cast<int>(hrIt->second) << "</gpxtpx:hr>\n";
                }

                if (gpsRec->cycles > 0)
                {
                    gpx << "            <gpxtpx:cad>" << static_cast<int>(gpsRec->cycles) << "</gpxtpx:cad>\n";
                }
            }

            if (hasExtensions)
            {
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

    std::string GpxConverter::escapeXml(const std::string &str) const
    {
        std::string escaped;
        escaped.reserve(str.size());

        for (char c : str)
        {
            switch (c)
            {
            case '&':
                escaped += "&amp;";
                break;
            case '<':
                escaped += "&lt;";
                break;
            case '>':
                escaped += "&gt;";
                break;
            case '"':
                escaped += "&quot;";
                break;
            case '\'':
                escaped += "&apos;";
                break;
            default:
                escaped += c;
            }
        }

        return escaped;
    }

    std::string GpxConverter::formatTimestamp(std::time_t time) const
    {
        std::tm tm;
#ifdef _WIN32
        gmtime_s(&tm, &time);
#else
        gmtime_r(&time, &tm);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        return oss.str();
    }

    std::string GpxConverter::getActivityName(const models::Activity &activity) const
    {
        std::ostringstream name;
        name << toString(activity.type) << " ";

        std::tm tm;
#ifdef _WIN32
        localtime_s(&tm, &activity.start_time);
#else
        localtime_r(&activity.start_time, &tm);
#endif

        name << std::put_time(&tm, "%Y-%m-%d %H:%M");
        return name.str();
    }

} // namespace tomtom::sdk::converters
