#pragma once

#include <memory>
#include <cstring>
#include <cassert>

#include "../activity_types.hpp"

namespace tomtom::sdk::models
{

    /**
     * @brief Base class for all activity record types
     *
     * Each record in a TomTom activity file starts with a tag byte
     * followed by tag-specific data. This base class provides the
     * common interface for all record types.
     */
    struct ActivityRecord
    {
        RecordTag tag;

        explicit ActivityRecord(RecordTag t) : tag(t) {}
        virtual ~ActivityRecord() = default;

        // Prevent copying (use unique_ptr for polymorphism)
        ActivityRecord(const ActivityRecord &) = delete;
        ActivityRecord &operator=(const ActivityRecord &) = delete;

        /**
         * @brief Get human-readable name of this record type
         */
        const std::string_view recordName() const { return toString(tag); }

        /**
         * @brief Create a record instance from binary data
         * @tparam RecordT Derived record type
         * @tparam DataT Data structure type
         * @param ptr Pointer to binary data
         * @param record_length Length of the record data
         * @return Unique pointer to created record
         */
        template <typename RecordT, typename DataT>
        static std::unique_ptr<RecordT> makeRecordFromBinary(const uint8_t *ptr, size_t record_length)
        {
            static_assert(std::is_standard_layout_v<DataT>);
            static_assert(std::is_trivially_copyable_v<DataT>);
            static_assert(std::is_base_of_v<DataT, RecordT>);

            assert(sizeof(RecordTag) + sizeof(DataT) == record_length);

            auto rec = std::make_unique<RecordT>();
            std::memcpy(static_cast<DataT *>(rec.get()), ptr, sizeof(DataT));
            return rec;
        }
    };

}
