#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::services::activity::records
{

    /**
     * @brief Lap record binary data structure (Tag 0x2F)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 10 bytes
     */
#pragma pack(push, 1)
    struct LapRecordData
    {
        uint32_t total_time;     // Seconds since activity start
        float total_distance;    // Total distance in meters
        uint16_t total_calories; // Total calories
    };
#pragma pack(pop)

    /**
     * @brief Lap marker record (Tag 0x2F)
     *
     * Logged when user manually marks a lap or auto-lap triggers.
     */
    struct LapRecord : ActivityRecord, LapRecordData
    {
        LapRecord() : ActivityRecord(RecordTag::Lap), LapRecordData{} {}

        static std::unique_ptr<LapRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<LapRecord>();
            std::memcpy(static_cast<LapRecordData *>(rec.get()), ptr, sizeof(LapRecordData));
            return rec;
        }

        const char *recordName() const override { return "Lap"; }
        size_t recordSize() const override { return sizeof(LapRecordData); }
    };

} // namespace tomtom::services::activity::records
