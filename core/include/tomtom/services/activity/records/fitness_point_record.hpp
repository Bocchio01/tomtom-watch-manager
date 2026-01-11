#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::services::activity::records
{

    /**
     * @brief Fitness point record binary data structure (Tag 0x4A)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 10 bytes
     */
#pragma pack(push, 1)
    struct FitnessPointRecordData
    {
        uint32_t timestamp;
        uint16_t points1;
        uint16_t points2;
    };
#pragma pack(pop)

    /**
     * @brief Fitness point record (Tag 0x4A)
     *
     * Contains fitness point data.
     */
    struct FitnessPointRecord : ActivityRecord, FitnessPointRecordData
    {
        FitnessPointRecord() : ActivityRecord(RecordTag::FitnessPoint), FitnessPointRecordData{} {}

        static std::unique_ptr<FitnessPointRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<FitnessPointRecord>();
            std::memcpy(static_cast<FitnessPointRecordData *>(rec.get()), ptr, sizeof(FitnessPointRecordData));
            return rec;
        }

        const char *recordName() const override { return "FitnessPoint"; }
        size_t recordSize() const override { return sizeof(FitnessPointRecordData); }
    };

}
