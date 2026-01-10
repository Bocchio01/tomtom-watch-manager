#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::services::activity::records
{

    /**
     * @brief Race setup record binary data structure (Tag 0x3C)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 36 bytes
     */
#pragma pack(push, 1)
    struct RaceSetupRecordData
    {
        uint8_t race_id[16]; // Only used for web services race, 0 otherwise
        float distance;      // Meters
        uint32_t duration;   // Seconds
        char name[16];       // Always null-terminated
    };
#pragma pack(pop)

    /**
     * @brief Race setup record (Tag 0x3C)
     *
     * Configures race parameters.
     */
    struct RaceSetupRecord : ActivityRecord, RaceSetupRecordData
    {
        RaceSetupRecord() : ActivityRecord(RecordTag::RaceSetup), RaceSetupRecordData{} {}

        static std::unique_ptr<RaceSetupRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<RaceSetupRecord>();
            std::memcpy(static_cast<RaceSetupRecordData *>(rec.get()), ptr, sizeof(RaceSetupRecordData));
            return rec;
        }

        const char *recordName() const override { return "RaceSetup"; }
        size_t recordSize() const override { return sizeof(RaceSetupRecordData); }
    };

} // namespace tomtom::services::activity::records
