#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "tomtom/sdk/models/activity_types.hpp"

namespace tomtom::sdk::store
{
    enum class DirectoryLayout
    {
        Flat,    // /Running_2023-01-01.gpx
        ByDate,  // /2023/01/Running_2023-01-01.gpx
        BySport, // /Running/2023/Running_2023-01-01.gpx
        ByDevice // /SerialNo/2023/Running_...
    };

    struct SdkConfig
    {
        // Root directory for all data
        std::filesystem::path storage_root = "TomTomData";

        // Layout preference
        DirectoryLayout layout = DirectoryLayout::ByDate;

        // Enabled export formats (e.g., "gpx", "kml", "json")
        std::vector<std::string> export_formats = {"gpx"};

        // Keep raw .ttbin files?
        bool archive_raw_data = true;

        // Logging
        std::string log_level = "info";
    };
}
