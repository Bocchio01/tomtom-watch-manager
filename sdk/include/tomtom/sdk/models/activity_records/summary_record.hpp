#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::sdk::models
{

    /**
     * @brief Summary record binary data structure (Tag 0x27)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 38 bytes
     */
#pragma pack(push, 1)
    struct SummaryRecordData
    {
        uint8_t activity_type;     // Type of activity (see ActivityType enum)
        float distance_meters;     // Total distance in meters
        uint32_t duration_seconds; // Total duration in seconds
        uint16_t calories;         // Total calories burned
        uint16_t reserved1;        // Reserved
        uint32_t duration2;        // Duplicate of duration in seconds (for compatibility?)
    };
#pragma pack(pop)

    /**
     * @brief Activity summary record (Tag 0x27)
     *
     * Contains overall statistics for the activity including
     * total distance, duration, calories, and heart rate metrics.
     */
    struct SummaryRecord : ActivityRecord, SummaryRecordData
    {
        SummaryRecord() : ActivityRecord(RecordTag::Summary), SummaryRecordData{} {}

        static std::unique_ptr<SummaryRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<SummaryRecord>();
            std::memcpy(static_cast<SummaryRecordData *>(rec.get()), ptr, sizeof(SummaryRecordData));
            return rec;
        }

        // Helper to get activity type as enum
        ActivityType getActivityType() const
        {
            return static_cast<ActivityType>(activity_type);
        }

        // ====================================================================
        // Convenience Methods
        // ====================================================================

        /**
         * @brief Get distance in kilometers
         */
        double getDistanceKm() const
        {
            return distance_meters / 1000.0;
        }

        /**
         * @brief Get distance in miles
         */
        double getDistanceMiles() const
        {
            return distance_meters / 1609.344;
        }

        /**
         * @brief Get average pace in minutes per kilometer
         */
        double getAvgPaceMinPerKm() const
        {
            if (distance_meters <= 0)
                return 0;
            return (duration_seconds / 60.0) / (distance_meters / 1000.0);
        }

        /**
         * @brief Get average pace in minutes per mile
         */
        double getAvgPaceMinPerMile() const
        {
            if (distance_meters <= 0)
                return 0;
            return (duration_seconds / 60.0) / (distance_meters / 1609.344);
        }

        /**
         * @brief Get average speed in km/h
         */
        double getAvgSpeedKmh() const
        {
            if (duration_seconds <= 0)
                return 0;
            return (distance_meters / 1000.0) / (duration_seconds / 3600.0);
        }

        /**
         * @brief Get average speed in mph
         */
        double getAvgSpeedMph() const
        {
            if (duration_seconds <= 0)
                return 0;
            return (distance_meters / 1609.344) / (duration_seconds / 3600.0);
        }
    };

}
