#include "tomtom/sdk/converters/json_converter.hpp"
#include "tomtom/sdk/models/activity_records/gps_record.hpp"
#include "tomtom/sdk/models/activity_records/heart_rate_record.hpp"
#include "tomtom/sdk/models/activity_records/altitude_record.hpp"

#include <sstream>
#include <iomanip>
#include <ctime>
#include <map>

namespace tomtom::sdk::converters
{

    std::string JsonConverter::convert(const models::Activity &activity)
    {
        std::ostringstream json;

        json << "{\n";
        json << "  \"activity\": {\n";

        // Metadata
        json << "    \"type\": \"" << toString(activity.type) << "\",\n";
        json << "    \"startTime\": \"" << formatTimestamp(activity.start_time) << "\",\n";
        json << "    \"duration\": " << activity.duration_seconds << ",\n";
        json << "    \"distance\": " << std::fixed << std::setprecision(2) << activity.distance_meters << ",\n";
        json << "    \"calories\": " << activity.calories << ",\n";
        json << "    \"formatVersion\": " << activity.format_version << ",\n";
        json << "    \"productId\": " << activity.product_id << ",\n";
        json << "    \"localTimeOffset\": " << activity.local_time_offset << ",\n";

        // Collect all GPS records
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

        // Create maps for heart rate and altitude
        std::map<uint32_t, uint8_t> heartRateMap;
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
        }

        // Track points
        json << "    \"trackPoints\": [\n";
        for (size_t i = 0; i < gpsRecords.size(); ++i)
        {
            const auto *gpsRec = gpsRecords[i];

            json << "      {\n";
            json << "        \"timestamp\": " << gpsRec->timestamp << ",\n";
            json << "        \"time\": \"" << formatTimestamp(gpsRec->timestamp) << "\",\n";
            json << "        \"latitude\": " << std::fixed << std::setprecision(7) << gpsRec->getLatitudeDegrees() << ",\n";
            json << "        \"longitude\": " << std::setprecision(7) << gpsRec->getLongitudeDegrees() << ",\n";
            json << "        \"speed\": " << std::setprecision(2) << gpsRec->getSpeedMps() << ",\n";
            json << "        \"heading\": " << std::setprecision(1) << gpsRec->getHeadingDegrees() << ",\n";
            json << "        \"distance\": " << std::setprecision(2) << gpsRec->distance << ",\n";
            json << "        \"calories\": " << gpsRec->calories << ",\n";
            json << "        \"cadence\": " << static_cast<int>(gpsRec->cycles);

            // Add heart rate if available
            auto hrIt = heartRateMap.find(gpsRec->timestamp);
            if (hrIt != heartRateMap.end())
            {
                json << ",\n        \"heartRate\": " << static_cast<int>(hrIt->second);
            }

            json << "\n      }";

            if (i < gpsRecords.size() - 1)
            {
                json << ",";
            }
            json << "\n";
        }
        json << "    ]\n";

        json << "  }\n";
        json << "}\n";

        return json.str();
    }

    std::string JsonConverter::escapeJson(const std::string &str) const
    {
        std::string escaped;
        escaped.reserve(str.size());

        for (char c : str)
        {
            switch (c)
            {
            case '"':
                escaped += "\\\"";
                break;
            case '\\':
                escaped += "\\\\";
                break;
            case '\b':
                escaped += "\\b";
                break;
            case '\f':
                escaped += "\\f";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                escaped += c;
            }
        }

        return escaped;
    }

    std::string JsonConverter::formatTimestamp(std::time_t time) const
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

} // namespace tomtom::sdk::converters
