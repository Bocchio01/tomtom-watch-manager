#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::services::activity::records
{

    /**
     * @brief Status record binary data structure (Tag 0x21)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 6 bytes
     */
#pragma pack(push, 1)
    struct StatusRecordData
    {
        uint8_t status;     // 0 = ready, 1 = active, 2 = paused, 3 = stopped
        uint8_t activity;   // Activity type code
        uint32_t timestamp; // GMT epoch seconds (UTC time)
    };
#pragma pack(pop)

    /**
     * @brief Activity status record (Tag 0x21)
     *
     * Logged when activity status changes (start, pause, resume, stop).
     */
    struct StatusRecord : ActivityRecord, StatusRecordData
    {
        StatusRecord() : ActivityRecord(RecordTag::Status), StatusRecordData{} {}

        static std::unique_ptr<StatusRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<StatusRecord>();
            std::memcpy(static_cast<StatusRecordData *>(rec.get()), ptr, sizeof(StatusRecordData));
            return rec;
        }

        const char *recordName() const override { return "Status"; }
        size_t recordSize() const override { return sizeof(StatusRecordData); }

        ActivityStatus getStatus() const
        {
            return static_cast<ActivityStatus>(status);
        }

        ActivityType getActivity() const
        {
            return static_cast<ActivityType>(activity);
        }
    };

}
