#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::services::activity::records
{

    /**
     * @brief GPS record binary data structure (Tag 0x22)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 29 bytes
     */
#pragma pack(push, 1)
    struct GPSRecordData
    {
        int32_t latitude;     // Latitude * 1E7 degrees (-180E7 to 180E7)
        int32_t longitude;    // Longitude * 1E7 degrees (-180E7 to 180E7)
        uint16_t heading;     // Heading * 1E2 degrees (0 to 360E2)
        uint16_t speed;       // Speed * 1E2 m/s
        uint32_t timestamp;   // GMT epoch seconds
        uint16_t calories;    // Cumulative calories
        float filtered_speed; // Filtered speed in m/s
        float distance;       // Cumulative distance in meters
        uint8_t cycles;       // Cadence-related (steps or pedal strokes)
    };
#pragma pack(pop)

    /**
     * @brief GPS position record (Tag 0x22)
     *
     * Contains GPS coordinates, speed, heading, and other metrics.
     * Logged every second when activity is active and GPS is available.
     *
     * Uses multiple inheritance to avoid field duplication:
     * - ActivityRecord provides polymorphic interface
     * - GPSRecordData provides actual data fields
     */
    struct GPSRecord : ActivityRecord, GPSRecordData
    {
        GPSRecord() : ActivityRecord(RecordTag::GPS), GPSRecordData{} {}

        static std::unique_ptr<GPSRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<GPSRecord>();
            std::memcpy(static_cast<GPSRecordData *>(rec.get()), ptr, sizeof(GPSRecordData));
            if (rec->timestamp == 0xFFFFFFFF)
            {
                // Invalid GPS record (no fix)
                return nullptr;
            }
            return rec;
        }

        const char *recordName() const override { return "GPS"; }
        size_t recordSize() const override { return sizeof(GPSRecordData); }

        // ====================================================================
        // Convenience Methods
        // ====================================================================

        /**
         * @brief Get latitude in degrees
         */
        double getLatitudeDegrees() const
        {
            return latitude / 1e7;
        }

        /**
         * @brief Get longitude in degrees
         */
        double getLongitudeDegrees() const
        {
            return longitude / 1e7;
        }

        /**
         * @brief Get heading in degrees (0-360)
         */
        double getHeadingDegrees() const
        {
            return heading / 1e2;
        }

        /**
         * @brief Get speed in meters per second
         */
        double getSpeedMps() const
        {
            return speed / 1e2;
        }

        /**
         * @brief Get speed in kilometers per hour
         */
        double getSpeedKmh() const
        {
            return getSpeedMps() * 3.6;
        }

        /**
         * @brief Get speed in miles per hour
         */
        double getSpeedMph() const
        {
            return getSpeedMps() * 2.23694;
        }

        /**
         * @brief Get distance in kilometers
         */
        double getDistanceKm() const
        {
            return distance / 1000.0;
        }

        /**
         * @brief Get distance in miles
         */
        double getDistanceMiles() const
        {
            return distance / 1609.344;
        }
    };

} // namespace tomtom::services::activity::records
