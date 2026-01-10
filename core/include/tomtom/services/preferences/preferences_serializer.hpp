#pragma once

#include "preferences_model.hpp"
#include <vector>
#include <string>

namespace tomtom::services::preferences
{

    /**
     * @brief Serializer for TomTom preferences to XML format
     *
     * Converts a preferences structure back to XML format suitable
     * for writing to the watch.
     */
    class PreferencesSerializer
    {
    public:
        /**
         * @brief Serialize preferences to XML data
         * @param prefs Preferences structure
         * @return XML data as byte vector
         */
        std::vector<uint8_t> serialize(const models::Preferences &prefs);

        /**
         * @brief Serialize preferences to XML string
         * @param prefs Preferences structure
         * @return XML string
         */
        std::string serializeToString(const models::Preferences &prefs);

    private:
        /**
         * @brief Build XML string from preferences
         */
        std::string buildXML(const models::Preferences &prefs);
    };

} // namespace tomtom::services::preferences
