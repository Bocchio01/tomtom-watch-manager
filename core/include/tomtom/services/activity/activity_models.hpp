#pragma once

#include <vector>
#include <ctime>
#include <map>

#include "activity_types.hpp"
#include "records/activity_record.hpp"

namespace tomtom::services::activity::models
{

    /**
     * @brief Complete activity/workout session data
     *
     * Represents a parsed TomTom .ttbin activity file with all
     * header information, summary, and detailed records.
     */
    struct Activity
    {
        // ====================================================================
        // Header Information
        // ====================================================================
        uint8_t format_version;                       // File format version
        uint8_t firmware_version[6];                  // Firmware version [major, minor, patch]
        uint16_t product_id;                          // Watch product ID
        std::time_t start_time;                       // Activity start time (UTC)
        std::time_t watch_time;                       // Watch's current time when saved
        int32_t local_time_offset;                    // Offset from UTC in seconds
        std::map<RecordTag, uint16_t> record_lengths; // Record type lengths

        // ====================================================================
        // All Records from Activity File
        // ====================================================================
        std::vector<std::unique_ptr<records::ActivityRecord>> records;

        // ====================================================================
        // Summary Information (from Summary record if present)
        // ====================================================================
        ActivityType type;
        uint32_t duration_seconds;
        float distance_meters;
        uint16_t calories;
    };

}
