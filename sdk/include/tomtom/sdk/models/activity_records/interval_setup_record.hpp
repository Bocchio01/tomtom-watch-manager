#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::sdk::models
{

    /**
     * @brief Interval setup record binary data structure (Tag 0x39)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 21 bytes
     */
#pragma pack(push, 1)
    struct IntervalSetupRecordData
    {
        uint8_t warm_type; // 0 = distance, 1 = time
        uint32_t warm;     // Meters or seconds
        uint8_t work_type; // 0 = distance, 1 = time
        uint32_t work;     // Meters or seconds
        uint8_t rest_type; // 0 = distance, 1 = time
        uint32_t rest;     // Meters or seconds
        uint8_t cool_type; // 0 = distance, 1 = time
        uint32_t cool;     // Meters or seconds
        uint8_t sets;
    };
#pragma pack(pop)

    /**
     * @brief Interval setup record (Tag 0x39)
     *
     * Configures interval training parameters.
     */
    struct IntervalSetupRecord : ActivityRecord, IntervalSetupRecordData
    {
        IntervalSetupRecord() : ActivityRecord(RecordTag::IntervalSetup), IntervalSetupRecordData{} {}

        static std::unique_ptr<IntervalSetupRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<IntervalSetupRecord>();
            std::memcpy(static_cast<IntervalSetupRecordData *>(rec.get()), ptr, sizeof(IntervalSetupRecordData));
            return rec;
        }
    };

}
