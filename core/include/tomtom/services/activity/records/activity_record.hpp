#pragma once

#include <memory>

#include "../activity_types.hpp"

namespace tomtom::services::activity::records
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
        virtual const char *recordName() const = 0;

        /**
         * @brief Get the size of this record in bytes (excluding tag)
         */
        virtual size_t recordSize() const = 0;
    };

}
