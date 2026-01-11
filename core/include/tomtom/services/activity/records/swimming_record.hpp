#pragma once

#include "activity_record.hpp"
#include "../activity_types.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::services::activity::records
{

    /**
     * @brief Swimming record binary data structure (Tag 0x34)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 22 bytes
     */
#pragma pack(push, 1)
    struct SwimmingRecordData
    {
        uint32_t timestamp;   // UTC time
        float total_distance; // Meters
        uint8_t frequency;
        uint8_t stroke_type;
        uint32_t strokes; // Since the last report
        uint32_t completed_laps;
        uint16_t total_calories;
    };
#pragma pack(pop)

    /**
     * @brief Swimming stroke record (Tag 0x34)
     *
     * Contains swimming stroke data including type, count, and duration.
     * Logged for each swimming interval.
     */
    struct SwimmingRecord : ActivityRecord, SwimmingRecordData
    {
        SwimmingRecord() : ActivityRecord(RecordTag::Swim), SwimmingRecordData{} {}

        static std::unique_ptr<SwimmingRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<SwimmingRecord>();
            std::memcpy(static_cast<SwimmingRecordData *>(rec.get()), ptr, sizeof(SwimmingRecordData));
            return rec;
        }

        const char *recordName() const override { return "Swimming"; }
        size_t recordSize() const override { return sizeof(SwimmingRecordData); }

        SwimStroke getStrokeType() const
        {
            return static_cast<SwimStroke>(stroke_type);
        }
    };

}
