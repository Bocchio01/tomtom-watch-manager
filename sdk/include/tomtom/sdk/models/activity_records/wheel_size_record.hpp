#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::sdk::models
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

        double getWheelSizeMeters() const
        {
            return wheel_size / 1000.0;
        }
    };

}
