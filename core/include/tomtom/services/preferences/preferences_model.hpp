#pragma once

#include <string>
#include <optional>
#include <chrono>
#include <cstdint>

namespace tomtom::services::preferences::models
{

    /**
     * @brief Watch preferences structure
     *
     * Contains watch configuration including name, authentication,
     * and various settings. Stored in XML format in file 0x00F20000.
     */
    struct Preferences
    {
        // ====================================================================
        // Version Information
        // ====================================================================
        std::string version;                              // Preferences file version
        std::chrono::system_clock::time_point modified{}; // Last modified timestamp (Unix epoch)

        // ====================================================================
        // Watch Identity
        // ====================================================================
        std::string watch_name;    // User-defined watch name
        std::string ephemeris_url; // Configuration URL

        // ====================================================================
        // Authentication (MySports/TomTom Account)
        // ====================================================================
        std::optional<std::string> auth_token;   // Authentication token
        std::optional<std::string> token_secret; // Token secret
        std::optional<std::string> user_id;      // User ID

        // ====================================================================
        // GPS/Location Settings
        // ====================================================================
        std::chrono::system_clock::time_point ephemeris_modified{}; // GPS ephemeris last update timestamp

        // ====================================================================
        // Display Settings
        // ====================================================================
        std::optional<std::string> language; // Display language
        std::optional<std::string> units;    // Metric/Imperial
    };

}
