#pragma once

#include "activity_converter.hpp"

namespace tomtom::sdk::converters
{
    /**
     * @brief JSON converter for activity data
     *
     * Converts TomTom activities to JSON format for easy parsing
     * and integration with web applications and data analysis tools.
     *
     * Features:
     * - Complete activity metadata
     * - All GPS track points
     * - Heart rate, altitude, and cadence data
     * - Human-readable and machine-parsable
     */
    class JsonConverter : public ActivityConverter
    {
    public:
        /**
         * @brief Convert activity to JSON format
         *
         * @param activity Activity to convert
         * @return JSON string
         */
        std::string convert(const models::Activity &activity) override;

        std::string getExtension() const override { return "json"; }
        std::string getMimeType() const override { return "application/json"; }

    private:
        std::string escapeJson(const std::string &str) const;
        std::string formatTimestamp(std::time_t time) const;
    };

} // namespace tomtom::sdk::converters
