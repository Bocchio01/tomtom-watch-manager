#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::services::activity::records
{

    /**
     * @brief Interval start record binary data structure (Tag 0x3A)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 1 byte
     */
#pragma pack(push, 1)
    struct IntervalStartRecordData
    {
        uint8_t type; // 1 = warmup, 2 = work, 3 = rest, 4 = cooldown, 5 = finished
    };
#pragma pack(pop)

    /**
     * @brief Interval start record (Tag 0x3A)
     *
     * Logged when an interval phase starts.
     */
    struct IntervalStartRecord : ActivityRecord, IntervalStartRecordData
    {
        IntervalStartRecord() : ActivityRecord(RecordTag::IntervalStart), IntervalStartRecordData{} {}

        static std::unique_ptr<IntervalStartRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<IntervalStartRecord>();
            std::memcpy(static_cast<IntervalStartRecordData *>(rec.get()), ptr, sizeof(IntervalStartRecordData));
            return rec;
        }

        const char *recordName() const override { return "IntervalStart"; }
        size_t recordSize() const override { return sizeof(IntervalStartRecordData); }

        IntervalType getType() const
        {
            return static_cast<IntervalType>(type);
        }
    };

}
