// ============================================================================
// cycling_cadence_record.hpp - Cycling cadence record (Tag 0x31)
// ============================================================================
#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::services::activity::records
{

    /**
     * @brief Cycling cadence record binary data structure (Tag 0x31)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 10 bytes
     */
#pragma pack(push, 1)
    struct CyclingCadenceRecordData
    {
        uint32_t wheel_revolutions;      // Total wheel revolutions
        uint16_t wheel_revolutions_time; // Time for wheel revolutions
        uint16_t crank_revolutions;      // Total crank revolutions
        uint16_t crank_revolutions_time; // Time for crank revolutions
    };
#pragma pack(pop)

    /**
     * @brief Cycling cadence record (Tag 0x31)
     *
     * Contains cycling-specific cadence data from sensors.
     */
    struct CyclingCadenceRecord : ActivityRecord, CyclingCadenceRecordData
    {
        CyclingCadenceRecord() : ActivityRecord(RecordTag::CyclingCadence), CyclingCadenceRecordData{} {}

        static std::unique_ptr<CyclingCadenceRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<CyclingCadenceRecord>();
            std::memcpy(static_cast<CyclingCadenceRecordData *>(rec.get()), ptr, sizeof(CyclingCadenceRecordData));
            return rec;
        }

        const char *recordName() const override { return "CyclingCadence"; }
        size_t recordSize() const override { return sizeof(CyclingCadenceRecordData); }
    };

} // namespace tomtom::services::activity::records
