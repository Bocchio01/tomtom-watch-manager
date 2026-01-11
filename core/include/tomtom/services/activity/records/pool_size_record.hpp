#pragma once

#include "activity_record.hpp"
#include <cstdint>
#include <cstring>

namespace tomtom::services::activity::records
{

    /**
     * @brief Pool size record binary data structure (Tag 0x2A)
     *
     * Packed binary layout exactly matching the file format.
     * Total size: 4 bytes
     */
#pragma pack(push, 1)
    struct PoolSizeRecordData
    {
        int32_t pool_size; // Centimeters
    };
#pragma pack(pop)

    /**
     * @brief Pool size record (Tag 0x2A)
     *
     * Specifies the swimming pool size.
     */
    struct PoolSizeRecord : ActivityRecord, PoolSizeRecordData
    {
        PoolSizeRecord() : ActivityRecord(RecordTag::PoolSize), PoolSizeRecordData{} {}

        static std::unique_ptr<PoolSizeRecord> fromBinary(const uint8_t *ptr)
        {
            auto rec = std::make_unique<PoolSizeRecord>();
            std::memcpy(static_cast<PoolSizeRecordData *>(rec.get()), ptr, sizeof(PoolSizeRecordData));
            return rec;
        }

        const char *recordName() const override { return "PoolSize"; }
        size_t recordSize() const override { return sizeof(PoolSizeRecordData); }

        double getPoolSizeMeters() const
        {
            return pool_size / 100.0;
        }
    };

}
