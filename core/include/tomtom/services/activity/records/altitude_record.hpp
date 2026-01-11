#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::services::activity::records
{

    /**
     * @brief Altitude record binary data structure (Tag 0x3E)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 7 bytes
     */
#pragma pack(push, 1)
    struct AltitudeRecordData
    {
        int16_t rel_altitude; // Altitude change from workout start (meters)
        float total_climb;    // Total climb in meters (descents ignored)
        uint8_t qualifier;    // Not defined yet
    };
#pragma pack(pop)

    /**
     * @brief Altitude record (Tag 0x3E)
     *
     * Contains altitude and elevation data.
     * Logged when altitude changes significantly or at regular intervals.
     */
    struct AltitudeRecord : ActivityRecord, AltitudeRecordData
    {
        AltitudeRecord() : ActivityRecord(RecordTag::AltitudeUpdate), AltitudeRecordData{} {}

        static std::unique_ptr<AltitudeRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<AltitudeRecord>();
            std::memcpy(static_cast<AltitudeRecordData *>(rec.get()), ptr, sizeof(AltitudeRecordData));
            return rec;
        }

        const char *recordName() const override { return "Altitude"; }
        size_t recordSize() const override { return sizeof(AltitudeRecordData); }

        /**
         * @brief Get altitude in meters
         */
        double getAltitudeMeters() const
        {
            return static_cast<double>(rel_altitude);
        }

        /**
         * @brief Get altitude in feet
         */
        double getAltitudeFeet() const
        {
            return rel_altitude * 3.28084;
        }
    };

}
