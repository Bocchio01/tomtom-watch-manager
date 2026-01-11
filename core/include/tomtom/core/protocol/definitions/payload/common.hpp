#pragma once

#include <cstdint>

#pragma pack(push, 1)

namespace tomtom::core::protocol::definitions
{

    /**
     * @brief Standard response structure for file operations
     *
     * @details
     * Most file operations return this structure with:
     * - reserved fields (purpose unknown, typically echoed)
     * - file_id for operation context
     * - error code indicating success/failure
     */
    struct FileOperationResponse
    {
        uint32_t reserved_1;
        uint32_t file_id;
        uint32_t reserved_2;
        uint32_t reserved_3;
        uint32_t error;
    };

    static_assert(sizeof(FileOperationResponse) == 20, "FileOperationResponse must be 20 bytes");

    /**
     * @brief Response for file size queries
     *
     * Similar to FileOperationResponse but includes file size.
     */
    struct FileSizeResponse
    {
        uint32_t reserved_1;
        uint32_t file_id;
        uint32_t reserved_2;
        uint32_t file_size;
        uint32_t error;
    };

    static_assert(sizeof(FileSizeResponse) == 20, "FileSizeResponse must be 20 bytes");

    /**
     * @brief Response for find operations (first/next file)
     */
    struct FindResponse
    {
        uint32_t reserved_1;
        uint32_t file_id;
        uint32_t reserved_2;
        uint32_t file_size;
        uint32_t end_of_list;
    };

    static_assert(sizeof(FindResponse) == 20, "FindResponse must be 20 bytes");

}

#pragma pack(pop)
