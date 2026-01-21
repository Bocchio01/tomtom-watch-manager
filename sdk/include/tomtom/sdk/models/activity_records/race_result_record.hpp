#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::sdk::models
{

    /**
     * @brief Race result record binary data structure (Tag 0x3D)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 18 bytes
     */
#pragma pack(push, 1)
    struct RaceResultRecordData
    {
        float distance;    // Meters
        uint32_t duration; // Seconds
        uint16_t calories;
    };
#pragma pack(pop)

    /**
     * @brief Race result record (Tag 0x3D)
     *
     * Stores race/competition results and statistics.
     */
    struct RaceResultRecord : ActivityRecord, RaceResultRecordData
    {
        RaceResultRecord() : ActivityRecord(RecordTag::RaceResult), RaceResultRecordData{} {}

        static std::unique_ptr<RaceResultRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<RaceResultRecord>();
            std::memcpy(static_cast<RaceResultRecordData *>(rec.get()), ptr, sizeof(RaceResultRecordData));
            return rec;
        }
    };

}
