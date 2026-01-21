#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::sdk::models
{

    /**
     * @brief Indoor cycling record binary data structure (Tag 0x40)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 12 bytes
     */
#pragma pack(push, 1)
    struct IndoorCyclingRecordData
    {
        uint32_t timestamp;
        float distance_meters;
        uint16_t calories;
        uint8_t cycling_cadence;
    };
#pragma pack(pop)

    /**
     * @brief Indoor cycling record (Tag 0x40)
     *
     * Contains indoor cycling workout data.
     */
    struct IndoorCyclingRecord : ActivityRecord, IndoorCyclingRecordData
    {
        IndoorCyclingRecord() : ActivityRecord(RecordTag::IndoorCycling), IndoorCyclingRecordData{} {}

        static std::unique_ptr<IndoorCyclingRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<IndoorCyclingRecord>();
            std::memcpy(static_cast<IndoorCyclingRecordData *>(rec.get()), ptr, sizeof(IndoorCyclingRecordData));
            return rec;
        }
    };

}
