#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::sdk::models
{

    /**
     * @brief Treadmill record binary data structure (Tag 0x32)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 16 bytes
     */
#pragma pack(push, 1)
    struct TreadmillRecordData
    {
        uint32_t timestamp; // UTC time
        float distance;     // Meters
        uint16_t calories;
        uint32_t steps;
        uint16_t step_length; // Centimeters
    };
#pragma pack(pop)

    /**
     * @brief Treadmill record (Tag 0x32)
     *
     * Contains treadmill-specific activity data.
     */
    struct TreadmillRecord : ActivityRecord, TreadmillRecordData
    {
        TreadmillRecord() : ActivityRecord(RecordTag::Treadmill), TreadmillRecordData{} {}

        static std::unique_ptr<TreadmillRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<TreadmillRecord>();
            std::memcpy(static_cast<TreadmillRecordData *>(rec.get()), ptr, sizeof(TreadmillRecordData));
            return rec;
        }
    };

}
