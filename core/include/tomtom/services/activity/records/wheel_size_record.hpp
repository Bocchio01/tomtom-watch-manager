// ============================================================================
// wheel_size_record.hpp - Wheel size record (Tag 0x2B)
// ============================================================================
#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::services::activity::records
{

    /**
     * @brief Wheel size record binary data structure (Tag 0x2B)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 4 bytes
     */
#pragma pack(push, 1)
    struct WheelSizeRecordData
    {
        uint32_t wheel_size; // Millimeters
    };
#pragma pack(pop)

    /**
     * @brief Wheel size record (Tag 0x2B)
     *
     * Specifies the bicycle wheel size.
     */
    struct WheelSizeRecord : ActivityRecord, WheelSizeRecordData
    {
        WheelSizeRecord() : ActivityRecord(RecordTag::WheelSize), WheelSizeRecordData{} {}

        static std::unique_ptr<WheelSizeRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<WheelSizeRecord>();
            std::memcpy(static_cast<WheelSizeRecordData *>(rec.get()), ptr, sizeof(WheelSizeRecordData));
            return rec;
        }

        const char *recordName() const override { return "WheelSize"; }
        size_t recordSize() const override { return sizeof(WheelSizeRecordData); }

        double getWheelSizeMeters() const
        {
            return wheel_size / 1000.0;
        }
    };

} // namespace tomtom::services::activity::records
