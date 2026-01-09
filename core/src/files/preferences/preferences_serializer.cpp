// ============================================================================
// preferences_serializer.cpp - Preferences XML serializer implementation
// ============================================================================

#include "tomtom/files/preferences/preferences_serializer.hpp"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cstdint>

namespace tomtom::files::preferences
{

    std::vector<uint8_t> PreferencesSerializer::serialize(const models::Preferences &prefs)
    {
        std::string xml = serializeToString(prefs);
        return std::vector<uint8_t>(xml.begin(), xml.end());
    }

    std::string PreferencesSerializer::serializeToString(const models::Preferences &prefs)
    {
        return buildXML(prefs);
    }

    std::string PreferencesSerializer::buildXML(const models::Preferences &prefs)
    {
        std::ostringstream xml;

        xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        xml << "<preferences>\n";

        // Version
        if (!prefs.version.empty())
        {
            xml << "  <version>" << prefs.version << "</version>\n";
        }
        else
        {
            xml << "  <version>1.0</version>\n";
        }

        // Modified timestamp (current time if not set)
        auto modified = prefs.modified;
        if (modified == std::chrono::system_clock::time_point{})
        {
            modified = std::chrono::system_clock::now();
        }
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
                                 modified.time_since_epoch())
                                 .count();
        xml << "  <modified>" << seconds << "</modified>\n";

        // Watch name
        xml << "  <name>" << prefs.watch_name << "</name>\n";

        // Config URL
        if (!prefs.config_url.empty())
        {
            xml << "  <config_url>" << prefs.config_url << "</config_url>\n";
        }

        // Authentication
        if (prefs.auth_token.has_value() && !prefs.auth_token->empty())
        {
            xml << "  <auth_token>" << *prefs.auth_token << "</auth_token>\n";
        }

        if (prefs.token_secret.has_value() && !prefs.token_secret->empty())
        {
            xml << "  <token_secret>" << *prefs.token_secret << "</token_secret>\n";
        }

        if (prefs.user_id.has_value() && !prefs.user_id->empty())
        {
            xml << "  <user_id>" << *prefs.user_id << "</user_id>\n";
        }

        // GPS ephemeris
        if (prefs.ephemeris_modified != std::chrono::system_clock::time_point{})
        {
            const auto ephemeris_seconds = std::chrono::duration_cast<std::chrono::seconds>(
                                               prefs.ephemeris_modified.time_since_epoch())
                                               .count();
            xml << "  <ephemeris_modified>" << ephemeris_seconds << "</ephemeris_modified>\n";
        }

        // Language
        if (prefs.language.has_value() && !prefs.language->empty())
        {
            xml << "  <language>" << *prefs.language << "</language>\n";
        }

        // Units
        if (prefs.units.has_value() && !prefs.units->empty())
        {
            xml << "  <units>" << *prefs.units << "</units>\n";
        }

        xml << "</preferences>\n";

        return xml.str();
    }

} // namespace tomtom::files::preferences
