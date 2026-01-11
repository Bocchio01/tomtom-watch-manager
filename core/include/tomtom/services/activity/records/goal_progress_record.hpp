#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::services::activity::records
{

    /**
     * @brief Goal progress record binary data structure (Tag 0x35)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 5 bytes
     */
#pragma pack(push, 1)
    struct GoalProgressRecordData
    {
        uint8_t percent;
        uint32_t value;
    };
#pragma pack(pop)

    /**
     * @brief Goal progress record (Tag 0x35)
     *
     * Tracks progress towards a goal.
     */
    struct GoalProgressRecord : ActivityRecord, GoalProgressRecordData
    {
        GoalProgressRecord() : ActivityRecord(RecordTag::GoalProgress), GoalProgressRecordData{} {}

        static std::unique_ptr<GoalProgressRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<GoalProgressRecord>();
            std::memcpy(static_cast<GoalProgressRecordData *>(rec.get()), ptr, sizeof(GoalProgressRecordData));
            return rec;
        }

        const char *recordName() const override { return "GoalProgress"; }
        size_t recordSize() const override { return sizeof(GoalProgressRecordData); }
    };

}
