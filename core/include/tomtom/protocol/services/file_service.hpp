#pragma once

#include <memory>
#include <vector>
#include <cstdint>

#include "tomtom/protocol/runtime/packet_handler.hpp"
#include "tomtom/protocol/definition/protocol.hpp"

namespace tomtom::protocol::services
{
    /**
     * @brief File opening mode.
     */
    enum class FileOpenMode
    {
        Read,
        Write
    };

    /**
     * @brief Represents a file entry on the watch.
     */
    struct FileEntry
    {
        definition::FileId id;
        uint32_t size;
    };

    /**
     * @brief Service for file operations on the watch.
     *
     * Provides methods to list, read, write, and delete files
     * stored on the watch.
     */
    class FileService
    {
    public:
        explicit FileService(std::shared_ptr<runtime::PacketHandler> packet_handler);

        /**
         * @brief Lists all files stored on the watch.
         * @return A vector containing metadata for all found files.
         * @throws std::runtime_error if the operation fails.
         */
        std::vector<FileEntry> listFiles();

        /**
         * @brief Reads a specific file from the watch.
         * @param file_id The ID of the file to read.
         * @return A vector containing the raw bytes of the file.
         * @throws std::runtime_error if the file cannot be opened or read.
         */
        std::vector<uint8_t> readFile(definition::FileId file_id);

        /**
         * @brief Writes data to a specific file on the watch.
         * @param file_id The ID of the file to write.
         * @param data The binary data to write.
         * @throws std::runtime_error if the write operation fails.
         */
        void writeFile(definition::FileId file_id, const std::vector<uint8_t> &data);

        /**
         * @brief Deletes a file from the watch.
         * @param file_id The ID of the file to delete.
         * @throws std::runtime_error if the delete operation fails.
         */
        void deleteFile(definition::FileId file_id);

        /**
         * @brief Gets the size of a specific file.
         * @param file_id The ID of the file.
         * @return The size of the file in bytes.
         * @throws std::runtime_error if the operation fails.
         */
        uint32_t getFileSize(definition::FileId file_id);

    private:
        /**
         * @brief Opens a file with the specified mode.
         * @param file_id The ID of the file to open.
         * @param mode The mode to open the file (Read or Write).
         * @throws std::runtime_error if the file cannot be opened.
         */
        void openFile(definition::FileId file_id, FileOpenMode mode);

        /**
         * @brief Closes an opened file.
         * @param file_id The ID of the file to close.
         * @param check_error If true, throws on error. If false, silently ignores errors.
         * @throws std::runtime_error if check_error is true and the operation fails.
         */
        void closeFile(definition::FileId file_id, bool check_error = true);

        std::shared_ptr<runtime::PacketHandler> packet_handler_;
    };
}
