#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::sdk::models
{

    /**
     * @brief Interval finish record binary data structure (Tag 0x3B)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 15 bytes
     */
#pragma pack(push, 1)
    struct IntervalFinishRecordData
    {
        uint8_t type;            // 1 = warmup, 2 = work, 3 = rest, 4 = cooldown
        uint32_t total_time;     // Seconds
        float total_distance;    // Meters
        uint16_t total_calories; // Calories
    };
#pragma pack(pop)

    /**
     * @brief Interval finish record (Tag 0x3B)
     *
     * Logged when an interval phase finishes.
     */
    struct IntervalFinishRecord : ActivityRecord, IntervalFinishRecordData
    {
        IntervalFinishRecord() : ActivityRecord(RecordTag::IntervalFinish), IntervalFinishRecordData{} {}

        static std::unique_ptr<IntervalFinishRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<IntervalFinishRecord>();
            std::memcpy(static_cast<IntervalFinishRecordData *>(rec.get()), ptr, sizeof(IntervalFinishRecordData));
            return rec;
        }

        IntervalType getType() const
        {
            return static_cast<IntervalType>(type);
        }
    };

}
