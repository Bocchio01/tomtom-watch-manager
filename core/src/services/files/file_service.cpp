#include <algorithm>
#include <stdexcept>
#include <spdlog/spdlog.h>

#include "tomtom/defines.hpp"
#include "tomtom/services/files/files.hpp"

namespace tomtom::services::files
{
    FileService::FileService(std::shared_ptr<protocol::runtime::PacketHandler> packet_handler)
        : packet_handler_(std::move(packet_handler))
    {
        if (!packet_handler_)
        {
            throw std::invalid_argument("PacketHandler cannot be null");
        }
    }

    std::vector<FileEntry> FileService::listFiles()
    {
        spdlog::debug("Listing files on the watch...");
        std::vector<FileEntry> files;

        protocol::definition::FindFirstFileTx find_first_request;
        auto find_first_response = packet_handler_->transaction<protocol::definition::FindFirstFileTx, protocol::definition::FindFirstFileRx>(find_first_request);

        bool end_of_list = find_first_response.packet.payload.end_of_list;

        if (!end_of_list)
        {
            FileId id;
            id.value = TT_BIGENDIAN(find_first_response.packet.payload.file_id);
            files.push_back({id, find_first_response.packet.payload.file_size});
        }

        while (!end_of_list)
        {
            protocol::definition::FindNextFileTx find_next_request;
            auto find_next_response = packet_handler_->transaction<protocol::definition::FindNextFileTx, protocol::definition::FindNextFileRx>(find_next_request);

            end_of_list = find_next_response.packet.payload.end_of_list;

            if (!end_of_list)
            {
                FileId id;
                id.value = TT_BIGENDIAN(find_next_response.packet.payload.file_id);
                files.push_back({id, find_next_response.packet.payload.file_size});
            }
        }

        spdlog::debug("Found {} files.", files.size());
        return files;
    }

    std::vector<uint8_t> FileService::readFile(FileId file_id)
    {
        spdlog::debug("Reading file with ID: 0x{:08X}", file_id.value);

        // 1. Open File
        openFile(file_id, FileOpenMode::Read);

        // 2. Get Size
        uint32_t file_size = 0;
        try
        {
            file_size = getFileSize(file_id);
        }
        catch (...)
        {
            // Try to close before re-throwing
            closeFile(file_id, false);
            throw;
        }

        // 3. Read Data
        const uint32_t chunk_size = 242;
        std::vector<uint8_t> data;
        data.reserve(file_size);
        bool done = false;

        while (!done)
        {
            protocol::definition::ReadFileDataTx read_request;
            read_request.payload.file_id = (FileId)TT_BIGENDIAN(file_id);
            read_request.payload.length = chunk_size;

            auto read_response = packet_handler_->transaction<protocol::definition::ReadFileDataTx, protocol::definition::ReadFileDataRx>(read_request);

            // Append data
            auto file_data_span = read_response.variable_data();
            data.insert(data.end(), file_data_span.begin(), file_data_span.end());

            // Check completion
            if (read_response.packet.payload.read_length < chunk_size)
            {
                done = true;
            }

            // Safety break for empty reads to prevent infinite loops
            if (read_response.packet.payload.read_length == 0 && file_data_span.size() == 0)
            {
                done = true;
            }
        }

        // 4. Close File
        closeFile(file_id);

        spdlog::debug("Read {} bytes from file 0x{:08X}", data.size(), file_id.value);
        return data;
    }

    void FileService::writeFile(FileId file_id, const std::vector<uint8_t> &data)
    {
        spdlog::debug("Writing {} bytes to file ID: 0x{:08X}", data.size(), file_id.value);

        openFile(file_id, FileOpenMode::Write);

        const size_t MAX_CHUNK_SIZE = 246;
        size_t total_written = 0;

        while (total_written < data.size())
        {
            size_t remaining = data.size() - total_written;
            size_t chunk_size = std::min(remaining, MAX_CHUNK_SIZE);

            protocol::definition::WriteFileDataTx write_request;
            write_request.payload.file_id = (FileId)TT_BIGENDIAN(file_id);

            std::vector<uint8_t> chunk_data(
                data.begin() + total_written,
                data.begin() + total_written + chunk_size);

            packet_handler_->transaction<protocol::definition::WriteFileDataTx, protocol::definition::WriteFileDataRx>(write_request, chunk_data);

            total_written += chunk_size;
        }

        closeFile(file_id);

        spdlog::debug("Wrote {} bytes to file 0x{:08X}", data.size(), file_id.value);
    }

    void FileService::deleteFile(FileId file_id)
    {
        protocol::definition::DeleteFileTx request;
        request.payload.file_id = (FileId)TT_BIGENDIAN(file_id);

        auto response = packet_handler_->transaction<protocol::definition::DeleteFileTx, protocol::definition::DeleteFileRx>(request);

        if (response.packet.payload.error != static_cast<uint32_t>(protocol::definition::ProtocolError::SUCCESS))
        {
            throw std::runtime_error("Failed to delete file (Error: " + std::to_string(response.packet.payload.error) + ")");
        }

        spdlog::debug("Deleted file 0x{:08X}", file_id.value);
    }

    uint32_t FileService::getFileSize(FileId file_id)
    {
        protocol::definition::GetFileSizeTx request;
        request.payload.file_id = (FileId)TT_BIGENDIAN(file_id);

        auto response = packet_handler_->transaction<protocol::definition::GetFileSizeTx, protocol::definition::GetFileSizeRx>(request);

        if (response.packet.payload.error != static_cast<uint32_t>(protocol::definition::ProtocolError::SUCCESS))
        {
            throw std::runtime_error("Failed to get file size (Error: " + std::to_string(response.packet.payload.error) + ")");
        }

        return TT_BIGENDIAN(response.packet.payload.file_size);
    }

    bool FileService::fileExists(FileId file_id)
    {
        try
        {
            return getFileSize(file_id) > 0;
        }
        catch (const std::exception &)
        {
            return false;
        }
    }

    void FileService::openFile(FileId file_id, FileOpenMode mode)
    {
        if (mode == FileOpenMode::Read)
        {
            protocol::definition::OpenFileReadTx open_request;
            open_request.payload.file_id = (FileId)TT_BIGENDIAN(file_id);
            auto open_response = packet_handler_->transaction<protocol::definition::OpenFileReadTx, protocol::definition::OpenFileReadRx>(open_request);

            if (open_response.packet.payload.error != static_cast<uint32_t>(protocol::definition::ProtocolError::SUCCESS))
            {
                throw std::runtime_error("Failed to open file for reading (Error: " + std::to_string(open_response.packet.payload.error) + ")");
            }
        }
        else // FileOpenMode::Write
        {
            protocol::definition::OpenFileWriteTx open_request;
            open_request.payload.file_id = (FileId)TT_BIGENDIAN(file_id);
            auto open_response = packet_handler_->transaction<protocol::definition::OpenFileWriteTx, protocol::definition::OpenFileWriteRx>(open_request);

            if (open_response.packet.payload.error != static_cast<uint32_t>(protocol::definition::ProtocolError::SUCCESS))
            {
                throw std::runtime_error("Failed to open file for writing (Error: " + std::to_string(open_response.packet.payload.error) + ")");
            }
        }
    }

    void FileService::closeFile(FileId file_id, bool check_error)
    {
        protocol::definition::CloseFileTx close_request;
        close_request.payload.file_id = (FileId)TT_BIGENDIAN(file_id);
        auto close_response = packet_handler_->transaction<protocol::definition::CloseFileTx, protocol::definition::CloseFileRx>(close_request);

        if (check_error && close_response.packet.payload.error != static_cast<uint32_t>(protocol::definition::ProtocolError::SUCCESS))
        {
            throw std::runtime_error("Failed to close file (Error: " + std::to_string(close_response.packet.payload.error) + ")");
        }
    }
}
