#pragma once

#include <string>
#include <memory>

#include "tomtom/sdk/models/activity_model.hpp"

namespace tomtom::sdk::converters
{
    /**
     * @brief Base interface for activity converters
     *
     * Defines the interface for converting Activity objects to different
     * export formats (GPX, KML, JSON, etc.).
     */
    class ActivityConverter
    {
    public:
        virtual ~ActivityConverter() = default;

        /**
         * @brief Convert activity to the target format
         *
         * @param activity Activity to convert
         * @return Converted data as string
         * @throws std::runtime_error if conversion fails
         */
        virtual std::string convert(const models::Activity &activity) = 0;

        /**
         * @brief Get the file extension for this format (e.g., "gpx", "kml")
         *
         * @return File extension without dot
         */
        virtual std::string getExtension() const = 0;

        /**
         * @brief Get the MIME type for this format
         *
         * @return MIME type string
         */
        virtual std::string getMimeType() const = 0;
    };

} // namespace tomtom::sdk::converters
