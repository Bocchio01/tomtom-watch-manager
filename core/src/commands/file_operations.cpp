#include <algorithm>
#include <stdexcept>

#include "tomtom/core/utils/endianness.hpp"
#include "tomtom/core/commands/file_operations.hpp"

namespace tomtom::core::commands
{
    FileOperations::FileOperations(std::shared_ptr<runtime::PacketHandler> packet_handler)
        : packet_handler_(std::move(packet_handler))
    {
        if (!packet_handler_)
        {
            throw std::invalid_argument("PacketHandler cannot be null");
        }
    }

    std::vector<FileEntry> FileOperations::listFiles()
    {
        std::vector<FileEntry> files;

        definitions::FindFirstFileTx find_first_request;
        auto find_first_response = packet_handler_->transaction<definitions::FindFirstFileTx, definitions::FindFirstFileRx>(find_first_request);

        bool end_of_list = find_first_response.packet.payload.end_of_list;

        if (!end_of_list)
        {
            definitions::FileId id;
            id.value = tomtom::core::utils::hostToBigEndian(find_first_response.packet.payload.file_id);
            files.push_back({id, find_first_response.packet.payload.file_size});
        }

        while (!end_of_list)
        {
            definitions::FindNextFileTx find_next_request;
            auto find_next_response = packet_handler_->transaction<definitions::FindNextFileTx, definitions::FindNextFileRx>(find_next_request);

            end_of_list = find_next_response.packet.payload.end_of_list;

            if (!end_of_list)
            {
                definitions::FileId id;
                id.value = tomtom::core::utils::hostToBigEndian(find_next_response.packet.payload.file_id);
                files.push_back({id, find_next_response.packet.payload.file_size});
            }
        }

        return files;
    }

    std::vector<uint8_t> FileOperations::readFile(definitions::FileId file_id)
    {
        uint32_t file_size = 0;

        try
        {
            openFile(file_id, FileOpenMode::Read);
            file_size = getFileSize(file_id);
        }
        catch (...)
        {
            closeFile(file_id, false);
            throw;
        }

        const uint32_t chunk_size = 242; // TODO: modify this depending on the ProductID
        std::vector<uint8_t> data;
        data.reserve(file_size);
        bool done = false;

        while (!done)
        {
            definitions::ReadFileDataTx read_request;
            read_request.payload.file_id = (definitions::FileId)tomtom::core::utils::hostToBigEndian(file_id);
            read_request.payload.length = chunk_size;

            auto read_response = packet_handler_->transaction<definitions::ReadFileDataTx, definitions::ReadFileDataRx>(read_request);

            auto file_data_span = read_response.variable_data();
            data.insert(data.end(), file_data_span.begin(), file_data_span.end());

            if (read_response.packet.payload.read_length < chunk_size)
            {
                done = true;
            }

            if (read_response.packet.payload.read_length == 0 && file_data_span.size() == 0)
            {
                done = true;
            }
        }

        closeFile(file_id);

        return data;
    }

    void FileOperations::writeFile(definitions::FileId file_id, const std::vector<uint8_t> &data)
    {
        size_t total_written = 0;
        const size_t MAX_CHUNK_SIZE = 246;

        openFile(file_id, FileOpenMode::Write);

        while (total_written < data.size())
        {
            size_t remaining = data.size() - total_written;
            size_t chunk_size = std::min(remaining, MAX_CHUNK_SIZE);

            definitions::WriteFileDataTx write_request;
            write_request.payload.file_id = (definitions::FileId)tomtom::core::utils::hostToBigEndian(file_id);

            std::vector<uint8_t> chunk_data(
                data.begin() + total_written,
                data.begin() + total_written + chunk_size);

            packet_handler_->transaction<definitions::WriteFileDataTx, definitions::WriteFileDataRx>(write_request, chunk_data);

            total_written += chunk_size;
        }

        closeFile(file_id);
    }

    void FileOperations::deleteFile(definitions::FileId file_id)
    {
        definitions::DeleteFileTx request;
        request.payload.file_id = (definitions::FileId)tomtom::core::utils::hostToBigEndian(file_id);

        auto response = packet_handler_->transaction<definitions::DeleteFileTx, definitions::DeleteFileRx>(request);

        if (response.packet.payload.error != static_cast<uint32_t>(definitions::ProtocolError::SUCCESS))
        {
            throw std::runtime_error("Failed to delete file (Error: " + std::to_string(response.packet.payload.error) + ")");
        }
    }

    uint32_t FileOperations::getFileSize(definitions::FileId file_id)
    {
        definitions::GetFileSizeTx request;
        request.payload.file_id = (definitions::FileId)tomtom::core::utils::hostToBigEndian(file_id);

        auto response = packet_handler_->transaction<definitions::GetFileSizeTx, definitions::GetFileSizeRx>(request);

        if (response.packet.payload.error != static_cast<uint32_t>(definitions::ProtocolError::SUCCESS))
        {
            throw std::runtime_error("Failed to get file size (Error: " + std::to_string(response.packet.payload.error) + ")");
        }

        return tomtom::core::utils::hostToBigEndian(response.packet.payload.file_size);
    }

    void FileOperations::openFile(definitions::FileId file_id, FileOpenMode mode)
    {
        if (mode == FileOpenMode::Read)
        {
            definitions::OpenFileReadTx open_request;
            open_request.payload.file_id = (definitions::FileId)tomtom::core::utils::hostToBigEndian(file_id);
            auto open_response = packet_handler_->transaction<definitions::OpenFileReadTx, definitions::OpenFileReadRx>(open_request);

            if (open_response.packet.payload.error != static_cast<uint32_t>(definitions::ProtocolError::SUCCESS))
            {
                throw std::runtime_error("Failed to open file for reading (Error: " + std::to_string(open_response.packet.payload.error) + ")");
            }
        }
        else // FileOpenMode::Write
        {
            definitions::OpenFileWriteTx open_request;
            open_request.payload.file_id = (definitions::FileId)tomtom::core::utils::hostToBigEndian(file_id);
            auto open_response = packet_handler_->transaction<definitions::OpenFileWriteTx, definitions::OpenFileWriteRx>(open_request);

            if (open_response.packet.payload.error != static_cast<uint32_t>(definitions::ProtocolError::SUCCESS))
            {
                throw std::runtime_error("Failed to open file for writing (Error: " + std::to_string(open_response.packet.payload.error) + ")");
            }
        }
    }

    void FileOperations::closeFile(definitions::FileId file_id, bool check_error)
    {
        definitions::CloseFileTx close_request;
        close_request.payload.file_id = (definitions::FileId)tomtom::core::utils::hostToBigEndian(file_id);
        auto close_response = packet_handler_->transaction<definitions::CloseFileTx, definitions::CloseFileRx>(close_request);

        if (check_error && close_response.packet.payload.error != static_cast<uint32_t>(definitions::ProtocolError::SUCCESS))
        {
            throw std::runtime_error("Failed to close file (Error: " + std::to_string(close_response.packet.payload.error) + ")");
        }
    }
}
