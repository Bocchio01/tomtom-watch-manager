#pragma once

#include <vector>
#include <cstdint>
#include <stdexcept>

#include "preferences_model.hpp"

namespace tomtom::services::preferences
{

    /**
     * @brief Exception thrown when preferences parsing fails
     */
    class PreferencesParseError : public std::runtime_error
    {
    public:
        explicit PreferencesParseError(const std::string &message)
            : std::runtime_error("Preferences parse error: " + message) {}
    };

    /**
     * @brief Parser for TomTom preferences XML file (0x00F20000)
     *
     * Parses the XML-formatted preferences file into a structured format.
     * The file contains watch settings, authentication credentials, and
     * various configuration options.
     */
    class PreferencesParser
    {
    public:
        /**
         * @brief Parse preferences from XML data
         * @param xml_data Raw XML data from preferences file
         * @return Parsed preferences structure
         * @throws PreferencesParseError if parsing fails
         */
        models::Preferences parse(const std::vector<uint8_t> &xml_data);

        /**
         * @brief Parse preferences from XML string
         * @param xml_string XML string
         * @return Parsed preferences structure
         * @throws PreferencesParseError if parsing fails
         */
        models::Preferences parse(const std::string &xml_string);
    };

}
