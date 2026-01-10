#pragma once

#include <tomtom/services/activity/activity_models.hpp>
#include <string>

namespace tomtom::sdk
{

    /**
     * @brief Supported export formats for activities
     */
    enum class ExportFormat
    {
        GPX, ///< GPS Exchange Format (XML)
        TCX, ///< Training Center XML Format
        CSV  ///< Comma-Separated Values
    };

    /**
     * @brief Activity format converter
     *
     * Converts activity data from the watch's internal format
     * to standard interchange formats (GPX, TCX, CSV).
     *
     * This is application-level logic - the watch doesn't know
     * about these formats, they're for user convenience.
     */
    class ActivityConverter
    {
    public:
        /**
         * @brief Convert activity to GPX format
         * @param activity The activity to convert
         * @return GPX XML string
         *
         * GPX is the most widely supported format for GPS data,
         * compatible with Garmin Connect, Strava, etc.
         */
        static std::string toGPX(const tomtom::services::activity::models::Activity &activity);

        /**
         * @brief Convert activity to TCX format
         * @param activity The activity to convert
         * @return TCX XML string
         *
         * TCX (Training Center XML) is Garmin's format with
         * enhanced support for training data.
         */
        static std::string toTCX(const tomtom::services::activity::models::Activity &activity);

        /**
         * @brief Convert activity to CSV format
         * @param activity The activity to convert
         * @return CSV string
         *
         * CSV format is useful for data analysis in spreadsheets
         * or custom tools.
         */
        static std::string toCSV(const tomtom::services::activity::models::Activity &activity);

        /**
         * @brief Get file extension for a format
         * @param format Export format
         * @return File extension (e.g., ".gpx")
         */
        static std::string getExtension(ExportFormat format);

        /**
         * @brief Parse format from string
         * @param format_str Format string ("gpx", "tcx", "csv")
         * @return Export format enum
         * @throws std::invalid_argument if format is unknown
         */
        static ExportFormat parseFormat(const std::string &format_str);
    };

} // namespace tomtom::sdk
