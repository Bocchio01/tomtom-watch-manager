#include <spdlog/spdlog.h>
#include <sstream>
#include <algorithm>
#include <charconv>
#include <limits>

#include "tomtom/sdk/parsers/preferences_parser.hpp"

using namespace tomtom::sdk::models;

namespace tomtom::sdk::parsers
{

    // Simple XML parsing helper functions
    namespace
    {
        /**
         * @brief Extract text content of a given XML tag
         * @param xml XML string
         * @param tag Tag name
         * @return Text content, or empty string if not found
         */
        std::string extractText(const std::string &xml, const std::string &tag)
        {
            std::string open_tag = "<" + tag + ">";
            std::string close_tag = "</" + tag + ">";

            size_t start = xml.find(open_tag);
            if (start == std::string::npos)
            {
                return "";
            }

            start += open_tag.length();
            size_t end = xml.find(close_tag, start);
            if (end == std::string::npos)
            {
                return "";
            }

            return xml.substr(start, end - start);
        }

        /**
         * @brief Parse Unix timestamp from XML and convert to time_point
         * @param xml XML string
         * @param tag Tag name
         * @return time_point, or epoch if parsing fails
         */
        [[maybe_unused]] std::chrono::system_clock::time_point extractTimestamp(const std::string &xml, const std::string &tag)
        {
            const std::string text = extractText(xml, tag);
            if (text.empty())
                return {};

            uint64_t seconds = 0;
            const auto result = std::from_chars(text.data(), text.data() + text.size(), seconds);

            if (result.ec != std::errc{} || result.ptr != text.data() + text.size())
            {
                return {};
            }

            return std::chrono::system_clock::time_point{std::chrono::seconds{seconds}};
        }
    }

    Preferences PreferencesParser::parse(const std::vector<uint8_t> &xml_data)
    {
        // Convert to string
        std::string xml_string(xml_data.begin(), xml_data.end());
        return parse(xml_string);
    }

    Preferences PreferencesParser::parse(const std::string &xml_string)
    {
        Preferences prefs;

        try
        {
            // Extract basic fields
            prefs.version = extractText(xml_string, "version");
            prefs.watch_name = extractText(xml_string, "name");
            prefs.ephemeris_url = extractText(xml_string, "ephemeris_url");

            // Extract timestamps
            prefs.modified = extractTimestamp(xml_string, "modified");
            prefs.ephemeris_modified = extractTimestamp(xml_string, "ephemeris_modified");

            // Extract optional authentication fields
            std::string auth_token = extractText(xml_string, "auth_token");
            if (!auth_token.empty())
            {
                prefs.auth_token = auth_token;
            }

            std::string token_secret = extractText(xml_string, "token_secret");
            if (!token_secret.empty())
            {
                prefs.token_secret = token_secret;
            }

            std::string user_id = extractText(xml_string, "user_id");
            if (!user_id.empty())
            {
                prefs.user_id = user_id;
            }

            // Extract optional settings
            std::string language = extractText(xml_string, "language");
            if (!language.empty())
            {
                prefs.language = language;
            }

            std::string units = extractText(xml_string, "units");
            if (!units.empty())
            {
                prefs.units = units;
            }

            spdlog::debug("Parsed preferences: name='{}', version='{}'",
                          prefs.watch_name, prefs.version);
        }
        catch (const std::exception &e)
        {
            throw PreferencesParseError(std::string("Failed to parse XML: ") + e.what());
        }

        return prefs;
    }

}
