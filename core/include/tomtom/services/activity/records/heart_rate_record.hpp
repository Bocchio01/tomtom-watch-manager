// ============================================================================
// heart_rate_record.hpp - Heart rate record (Tag 0x25)
// ============================================================================
#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::services::activity::records
{

    /**
     * @brief Heart rate record binary data structure (Tag 0x25)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 6 bytes
     */
#pragma pack(push, 1)
    struct HeartRateRecordData
    {
        uint32_t timestamp; // GMT epoch seconds (UTC time)
        uint8_t heart_rate; // Heart rate in BPM (0-255)
        uint8_t reserved;   // Reserved byte (usually 0)
    };
#pragma pack(pop)

    /**
     * @brief Heart rate record (Tag 0x25)
     *
     * Contains heart rate in beats per minute.
     * Logged every second when heart rate monitor is active.
     */
    struct HeartRateRecord : ActivityRecord, HeartRateRecordData
    {
        HeartRateRecord() : ActivityRecord(RecordTag::HeartRate), HeartRateRecordData{} {}

        static std::unique_ptr<HeartRateRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<HeartRateRecord>();
            std::memcpy(static_cast<HeartRateRecordData *>(rec.get()), ptr, sizeof(HeartRateRecordData));
            return rec;
        }

        const char *recordName() const override { return "HeartRate"; }
        size_t recordSize() const override { return sizeof(HeartRateRecordData); }

        /**
         * @brief Check if heart rate is valid
         */
        bool isValid() const
        {
            return heart_rate > 0 && heart_rate < 255;
        }
    };

} // namespace tomtom::services::activity::records
