#pragma once

#include "activity_converter.hpp"

namespace tomtom::sdk::converters
{
    /**
     * @brief KML (Keyhole Markup Language) converter
     *
     * Converts TomTom activities to KML format for use with Google Earth
     * and other mapping applications.
     *
     * Features:
     * - LineString track with GPS coordinates
     * - Placemarks for start/end points
     * - Activity metadata in description
     * - Color-coded based on activity type
     *
     * @see https://developers.google.com/kml/documentation
     */
    class KmlConverter : public ActivityConverter
    {
    public:
        /**
         * @brief Convert activity to KML format
         *
         * @param activity Activity to convert
         * @return KML XML string
         * @throws std::runtime_error if activity has no GPS data
         */
        std::string convert(const models::Activity &activity) override;

        std::string getExtension() const override { return "kml"; }
        std::string getMimeType() const override { return "application/vnd.google-earth.kml+xml"; }

    private:
        std::string escapeXml(const std::string &str) const;
        std::string formatTimestamp(std::time_t time) const;
        std::string getActivityName(const models::Activity &activity) const;
        std::string getActivityColor(const models::Activity &activity) const;
    };

} // namespace tomtom::sdk::converters
