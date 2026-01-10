#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::services::activity::records
{

    /**
     * @brief Training setup record binary data structure (Tag 0x2D)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 9 bytes
     */
#pragma pack(push, 1)
    struct TrainingSetupRecordData
    {
        uint8_t type;    // 0-12 (see TrainingType enum)
        float value_min; // Meters, seconds, calories, secs/km, km/h, bpm (min for zones)
        float max;       // Secs/km, km/h, bpm (only used for zones)
    };
#pragma pack(pop)

    /**
     * @brief Training setup record (Tag 0x2D)
     *
     * Configures training mode parameters.
     */
    struct TrainingSetupRecord : ActivityRecord, TrainingSetupRecordData
    {
        TrainingSetupRecord() : ActivityRecord(RecordTag::TrainingSetup), TrainingSetupRecordData{} {}

        static std::unique_ptr<TrainingSetupRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<TrainingSetupRecord>();
            std::memcpy(static_cast<TrainingSetupRecordData *>(rec.get()), ptr, sizeof(TrainingSetupRecordData));
            return rec;
        }

        const char *recordName() const override { return "TrainingSetup"; }
        size_t recordSize() const override { return sizeof(TrainingSetupRecordData); }

        TrainingType getType() const
        {
            return static_cast<TrainingType>(type);
        }
    };

} // namespace tomtom::services::activity::records
