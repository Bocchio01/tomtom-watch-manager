// ============================================================================
// heart_rate_recovery_record.hpp - Heart rate recovery record (Tag 0x3F)
// ============================================================================
#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::services::activity::records
{

    /**
     * @brief Heart rate recovery record binary data structure (Tag 0x3F)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 8 bytes
     */
#pragma pack(push, 1)
    struct HeartRateRecoveryRecordData
    {
        uint32_t status;     // 1 = poor, 2 = decent, 3 = good, 4 = excellent
        uint32_t heart_rate; // Heart rate recovery in bpm
    };
#pragma pack(pop)

    /**
     * @brief Heart rate recovery record (Tag 0x3F)
     *
     * Contains heart rate recovery information after exercise.
     */
    struct HeartRateRecoveryRecord : ActivityRecord, HeartRateRecoveryRecordData
    {
        HeartRateRecoveryRecord() : ActivityRecord(RecordTag::HeartRateRecovery), HeartRateRecoveryRecordData{} {}

        static std::unique_ptr<HeartRateRecoveryRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<HeartRateRecoveryRecord>();
            std::memcpy(static_cast<HeartRateRecoveryRecordData *>(rec.get()), ptr, sizeof(HeartRateRecoveryRecordData));
            return rec;
        }

        const char *recordName() const override { return "HeartRateRecovery"; }
        size_t recordSize() const override { return sizeof(HeartRateRecoveryRecordData); }

        RecoveryStatus getStatus() const
        {
            return static_cast<RecoveryStatus>(status);
        }
    };

} // namespace tomtom::services::activity::records
