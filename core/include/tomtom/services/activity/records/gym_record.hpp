// ============================================================================
// gym_record.hpp - Gym record (Tag 0x41)
// ============================================================================
#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::services::activity::records
{

    /**
     * @brief Gym record binary data structure (Tag 0x41)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 10 bytes
     */
#pragma pack(push, 1)
    struct GymRecordData
    {
        uint32_t timestamp;
        uint16_t total_calories;
        uint32_t total_cycles;
    };
#pragma pack(pop)

    /**
     * @brief Gym record (Tag 0x41)
     *
     * Contains gym workout data.
     */
    struct GymRecord : ActivityRecord, GymRecordData
    {
        GymRecord() : ActivityRecord(RecordTag::Gym), GymRecordData{} {}

        static std::unique_ptr<GymRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<GymRecord>();
            std::memcpy(static_cast<GymRecordData *>(rec.get()), ptr, sizeof(GymRecordData));
            return rec;
        }

        const char *recordName() const override { return "Gym"; }
        size_t recordSize() const override { return sizeof(GymRecordData); }
    };

} // namespace tomtom::services::activity::records
