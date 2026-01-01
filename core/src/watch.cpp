#include <spdlog/spdlog.h>

#include "tomtom/defines.hpp"
#include "tomtom/watch.hpp"
#include "tomtom/protocol/definition/protocol.hpp"

namespace tomtom
{
    Watch::Watch(std::shared_ptr<transport::DeviceConnection> conn) : connection(std::move(conn))
    {
        if (!connection)
        {
            throw std::invalid_argument("Connection cannot be null");
        }

        if (!connection->open())
        {
            throw std::runtime_error("Failed to open connection to the watch");
        }

        packet_handler_ = std::make_unique<protocol::runtime::PacketHandler>(connection);

        spdlog::info("Connected to watch: {} (Product ID: 0x{:04X}, Serial: {})",
                     getProductName(), getProductId(), getSerialNumber());
    }

    Watch::~Watch()
    {
        if (connection && connection->isOpen())
        {
            connection->close();
        }
    }

    Watch::Watch(Watch &&other) noexcept : connection(std::move(other.connection))
    {
    }

    Watch &Watch::operator=(Watch &&other) noexcept
    {
        if (this != &other)
        {
            connection = std::move(other.connection);
        }
        return *this;
    }

    std::time_t Watch::getTime()
    {
        spdlog::debug("Requesting time from watch...");

        protocol::definition::GetWatchTimeTx request;

        auto response = packet_handler_->transaction<protocol::definition::GetWatchTimeTx, protocol::definition::GetWatchTimeRx>(request);
        uint32_t raw_time = response.packet.payload.time;

        std::time_t t = static_cast<std::time_t>(TT_BIGENDIAN(raw_time));

        spdlog::debug("Watch Time: {}", std::asctime(std::gmtime(&t)));

        return t;
    }

    std::string Watch::getFirmwareVersion()
    {
        spdlog::debug("Requesting firmware version from watch...");

        protocol::definition::GetFirmwareVersionTx request;

        auto response = packet_handler_->transaction<protocol::definition::GetFirmwareVersionTx, protocol::definition::GetFirmwareVersionRx>(request);

        // The firmware version is raw bytes (no structured payload)
        std::string version(reinterpret_cast<const char *>(response.raw_payload_bytes.data()),
                            response.raw_payload_bytes.size());

        spdlog::debug("Firmware Version: {}", version);

        return version;
    }

    void Watch::listFiles()
    {
        spdlog::info("Listing files on the watch...");

        protocol::definition::FindFirstFileTx find_first_request;
        auto find_first_response = packet_handler_->transaction<protocol::definition::FindFirstFileTx, protocol::definition::FindFirstFileRx>(find_first_request);

        bool end_of_list = find_first_response.packet.payload.end_of_list;

        spdlog::info("File ID: 0x{:08X}, Size: {}, End of List: {}", TT_BIGENDIAN(find_first_response.packet.payload.file_id.value), find_first_response.packet.payload.file_size, end_of_list);

        while (!end_of_list)
        {
            protocol::definition::FindNextFileTx find_next_request;
            auto find_next_response = packet_handler_->transaction<protocol::definition::FindNextFileTx, protocol::definition::FindNextFileRx>(find_next_request);

            end_of_list = find_next_response.packet.payload.end_of_list;

            spdlog::info("File ID: 0x{:08X}, Size: {}, End of List: {}", TT_BIGENDIAN(find_next_response.packet.payload.file_id.value), find_next_response.packet.payload.file_size, end_of_list);
        }

        spdlog::info("Completed file listing.");
    }

    void Watch::readFile(protocol::definition::FileId file_id)
    {
        spdlog::info("Reading file with ID: 0x{:08X}", file_id.value);

        // Open file for reading
        protocol::definition::OpenFileReadTx open_request;
        open_request.payload.file_id = (protocol::definition::FileId)TT_BIGENDIAN(file_id);
        auto open_response = packet_handler_->transaction<protocol::definition::OpenFileReadTx, protocol::definition::OpenFileReadRx>(open_request);
        if (open_response.packet.payload.error != static_cast<uint32_t>(protocol::definition::ProtocolError::SUCCESS))
        {
            spdlog::error("Failed to open file. Error code: {}", open_response.packet.payload.error);
            return;
        }

        // Get file size
        uint32_t file_size = 0;
        protocol::definition::GetFileSizeTx size_request;
        size_request.payload.file_id = (protocol::definition::FileId)TT_BIGENDIAN(file_id);
        auto size_response = packet_handler_->transaction<protocol::definition::GetFileSizeTx, protocol::definition::GetFileSizeRx>(size_request);
        file_size = TT_BIGENDIAN(size_response.packet.payload.file_size);
        if (size_response.packet.payload.error != static_cast<uint32_t>(protocol::definition::ProtocolError::SUCCESS))
        {
            spdlog::error("Failed to get file size. Error code: {}", size_response.packet.payload.error);
            return;
        }
        spdlog::info("File size: {} bytes", file_size);

        // Read file data in chunks
        const uint32_t chunk_size = 242;
        uint32_t bytes_read = 0;
        std::vector<uint8_t> data;
        data.reserve(file_size);
        bool done = false;
        while (!done)
        {
            protocol::definition::ReadFileDataTx read_request;
            read_request.payload.file_id = (protocol::definition::FileId)TT_BIGENDIAN(file_id);
            read_request.payload.length = chunk_size;

            auto read_response = packet_handler_->transaction<protocol::definition::ReadFileDataTx, protocol::definition::ReadFileDataRx>(read_request);

            // Access the variable-length file data safely
            auto file_data_span = read_response.variable_data();
            data.insert(data.end(), file_data_span.begin(), file_data_span.end());

            bytes_read += read_response.packet.payload.read_length;
            spdlog::info("Read {} bytes from file ID 0x{:08X}", read_response.packet.payload.read_length, TT_BIGENDIAN(file_id.value));

            if (read_response.packet.payload.read_length < chunk_size)
            {
                done = true; // Reached end of file
            }
        }
        spdlog::info("Completed reading file ID 0x{:08X}, total bytes read: {}", TT_BIGENDIAN(file_id.value), bytes_read);

        // Close the file
        protocol::definition::CloseFileTx close_request;
        close_request.payload.file_id = file_id;
        auto close_response = packet_handler_->transaction<protocol::definition::CloseFileTx, protocol::definition::CloseFileRx>(close_request);
        if (close_response.packet.payload.error != static_cast<uint32_t>(protocol::definition::ProtocolError::SUCCESS))
        {
            spdlog::error("Failed to close file. Error code: {}", close_response.packet.payload.error);
        }
        else
        {
            spdlog::info("File ID 0x{:08X} closed successfully.", TT_BIGENDIAN(file_id.value));
        }

        // Print the file data (hex + ASCII dump)
        std::ostringstream oss_hex;
        std::ostringstream oss_ascii;

        for (size_t i = 0; i < data.size(); ++i)
        {
            // Hex dump
            oss_hex << std::hex << std::uppercase << std::setw(2)
                    << std::setfill('0') << static_cast<unsigned>(data[i]) << ' ';

            // ASCII dump (printable characters or '.')
            if (std::isprint(data[i]) || data[i] == '\n' || data[i] == '\r' || data[i] == '\t')
                oss_ascii << static_cast<char>(data[i]);
            else
                oss_ascii << '.';
        }

        // Log both
        spdlog::info("Data ({} bytes):", data.size());
        spdlog::info("HEX  : {}", oss_hex.str());
        spdlog::info("ASCII: {}", oss_ascii.str());
    }

}