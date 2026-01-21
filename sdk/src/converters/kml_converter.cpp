#include "tomtom/sdk/converters/kml_converter.hpp"
#include "tomtom/sdk/models/activity_records/gps_record.hpp"
#include "tomtom/sdk/models/activity_records/altitude_record.hpp"

#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <ctime>
#include <map>

namespace tomtom::sdk::converters
{

    std::string KmlConverter::convert(const models::Activity &activity)
    {
        std::ostringstream kml;

        // Collect GPS records
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

        // Create altitude tracker (altitude records track relative changes)
        int16_t currentAltitude = 0;
        for (const auto &record : activity.records)
        {
            if (record->tag == models::RecordTag::AltitudeUpdate)
            {
                auto *altRec = dynamic_cast<models::AltitudeRecord *>(record.get());
                if (altRec)
                {
                    currentAltitude = altRec->rel_altitude;
                }
            }
        }

        // KML header
        kml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        kml << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n";
        kml << "  <Document>\n";
        kml << "    <name>" << escapeXml(getActivityName(activity)) << "</name>\n";

        // Description with activity stats
        kml << "    <description><![CDATA[\n";
        kml << "      <h3>" << toString(activity.type) << "</h3>\n";
        kml << "      <p>Start Time: " << formatTimestamp(activity.start_time) << "</p>\n";
        kml << "      <p>Duration: " << (activity.duration_seconds / 60) << " minutes</p>\n";
        kml << "      <p>Distance: " << std::fixed << std::setprecision(2)
            << (activity.distance_meters / 1000.0) << " km</p>\n";
        kml << "      <p>Calories: " << activity.calories << " kcal</p>\n";
        kml << "    ]]></description>\n\n";

        // Style for the track line
        std::string color = getActivityColor(activity);
        kml << "    <Style id=\"trackStyle\">\n";
        kml << "      <LineStyle>\n";
        kml << "        <color>" << color << "</color>\n";
        kml << "        <width>3</width>\n";
        kml << "      </LineStyle>\n";
        kml << "    </Style>\n\n";

        // Start point placemark
        const auto *startPoint = gpsRecords.front();
        kml << "    <Placemark>\n";
        kml << "      <name>Start</name>\n";
        kml << "      <description>Activity start</description>\n";
        kml << "      <Point>\n";
        kml << "        <coordinates>" << std::fixed << std::setprecision(7)
            << startPoint->getLongitudeDegrees() << ","
            << startPoint->getLatitudeDegrees() << ",0</coordinates>\n";
        kml << "      </Point>\n";
        kml << "    </Placemark>\n\n";

        // End point placemark
        const auto *endPoint = gpsRecords.back();
        kml << "    <Placemark>\n";
        kml << "      <name>End</name>\n";
        kml << "      <description>Activity end</description>\n";
        kml << "      <Point>\n";
        kml << "        <coordinates>" << std::fixed << std::setprecision(7)
            << endPoint->getLongitudeDegrees() << ","
            << endPoint->getLatitudeDegrees() << ",0</coordinates>\n";
        kml << "      </Point>\n";
        kml << "    </Placemark>\n\n";

        // Track as LineString
        kml << "    <Placemark>\n";
        kml << "      <name>Track</name>\n";
        kml << "      <styleUrl>#trackStyle</styleUrl>\n";
        kml << "      <LineString>\n";
        kml << "        <extrude>1</extrude>\n";
        kml << "        <tessellate>1</tessellate>\n";
        kml << "        <altitudeMode>absolute</altitudeMode>\n";
        kml << "        <coordinates>\n";

        // Generate coordinates (lon,lat,alt format for KML)
        for (const auto *gpsRec : gpsRecords)
        {
            kml << "          " << std::fixed << std::setprecision(7)
                << gpsRec->getLongitudeDegrees() << ","
                << gpsRec->getLatitudeDegrees();

            // Add altitude if available
            if (currentAltitude != 0)
            {
                kml << "," << std::setprecision(1) << (currentAltitude / 100.0);
            }
            else
            {
                kml << ",0";
            }
            kml << "\n";
        }

        kml << "        </coordinates>\n";
        kml << "      </LineString>\n";
        kml << "    </Placemark>\n";
        kml << "  </Document>\n";
        kml << "</kml>\n";

        return kml.str();
    }

    std::string KmlConverter::escapeXml(const std::string &str) const
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

    std::string KmlConverter::formatTimestamp(std::time_t time) const
    {
        std::tm tm;
#ifdef _WIN32
        localtime_s(&tm, &time);
#else
        localtime_r(&time, &tm);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    std::string KmlConverter::getActivityName(const models::Activity &activity) const
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

    std::string KmlConverter::getActivityColor(const models::Activity &activity) const
    {
        // KML color format: aabbggrr (alpha, blue, green, red in hex)
        switch (activity.type)
        {
        case models::ActivityType::Running:
            return "ff0000ff"; // Red
        case models::ActivityType::Cycling:
            return "ffff0000"; // Blue
        case models::ActivityType::Swimming:
            return "ffffff00"; // Cyan
        case models::ActivityType::Treadmill:
            return "ff00ffff"; // Yellow
        case models::ActivityType::Freestyle:
            return "ffff00ff"; // Magenta
        case models::ActivityType::Hiking:
            return "ff00ff00"; // Green
        case models::ActivityType::IndoorCycling:
            return "ffff8800"; // Light Blue
        default:
            return "ffffffff"; // White
        }
    }

} // namespace tomtom::sdk::converters
