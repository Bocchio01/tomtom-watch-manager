#pragma once

#include "activity_converter.hpp"

namespace tomtom::sdk::converters
{
    /**
     * @brief GPX (GPS Exchange Format) converter
     *
     * Converts TomTom activities to GPX 1.1 format.
     * GPX is the de-facto standard for GPS data exchange.
     *
     * Features:
     * - Track points with GPS coordinates, elevation, timestamps
     * - Heart rate, cadence, speed as extensions
     * - Activity metadata (name, type, distance, duration)
     *
     * @see https://www.topografix.com/gpx.asp
     */
    class GpxConverter : public ActivityConverter
    {
    public:
        /**
         * @brief Convert activity to GPX 1.1 format
         *
         * @param activity Activity to convert
         * @return GPX XML string
         * @throws std::runtime_error if activity has no GPS data
         */
        std::string convert(const models::Activity &activity) override;

        std::string getExtension() const override { return "gpx"; }
        std::string getMimeType() const override { return "application/gpx+xml"; }

    private:
        std::string escapeXml(const std::string &str) const;
        std::string formatTimestamp(std::time_t time) const;
        std::string getActivityName(const models::Activity &activity) const;
    };

} // namespace tomtom::sdk::converters
