#pragma once

#include <tomtom/services/activity/activity_models.hpp>
#include <string>

namespace tomtom::sdk
{

    /**
     * @brief Static class for converting activities to standard export formats
     *
     * Directly converts Core Activity models to GPX, TCX, or CSV strings
     * without intermediate data structures.
     */
    class ActivityExporter
    {
    public:
        /**
         * @brief Supported export formats
         */
        enum class Format
        {
            GPX, ///< GPS Exchange Format (XML)
            TCX, ///< Training Center XML Format
            CSV  ///< Comma-Separated Values
        };

        /**
         * @brief Export activity to specified format
         * @param activity The activity to convert
         * @param format Output format
         * @return Formatted string
         */
        static std::string exportFile(
            const tomtom::services::activity::models::Activity &activity,
            Format format);

        /**
         * @brief Get file extension for a format
         * @param format Export format
         * @return File extension (e.g., ".gpx")
         */
        static std::string getExtension(Format format);

        /**
         * @brief Parse format from string
         * @param format_str Format string ("gpx", "tcx", "csv")
         * @return Export format enum
         * @throws std::invalid_argument if format is unknown
         */
        static Format parseFormat(const std::string &format_str);

    private:
        /**
         * @brief Convert activity to GPX format
         * @param activity The activity to convert
         * @return GPX XML string
         */
        static std::string toGPX(const tomtom::services::activity::models::Activity &activity);

        /**
         * @brief Convert activity to TCX format
         * @param activity The activity to convert
         * @return TCX XML string
         */
        static std::string toTCX(const tomtom::services::activity::models::Activity &activity);

        /**
         * @brief Convert activity to CSV format
         * @param activity The activity to convert
         * @return CSV string
         */
        static std::string toCSV(const tomtom::services::activity::models::Activity &activity);
    };

} // namespace tomtom::sdk
