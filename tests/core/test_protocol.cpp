#include <iostream>
#include <spdlog/spdlog.h>

#include "tomtom/manager.hpp"
#include "tomtom/protocol/protocol_handler.hpp"
#include "tomtom/protocol/protocol_utils.hpp"

using namespace tomtom;
using namespace tomtom::protocol;

int main()
{
    spdlog::set_level(spdlog::level::info);

    tomtom::Manager manager;

    auto watch = manager.connectToWatch();

    if (!watch)
    {
        spdlog::error("No watch connected.\n");
        return 1;
    }

    ProtocolHandler protocol(watch->connection);
    try
    {
        // Example 1: Get current time
        auto time_payload = protocol.sendRequest(MessageType::GET_CURRENT_TIME);
        uint32_t time = utils::parseCurrentTimeResponse(time_payload);
        std::cout << "Current time: " << static_cast<std::time_t>(time) << "\n";

        // Example 2: Get firmware version
        auto fw_payload = protocol.sendRequest(MessageType::GET_FIRMWARE_VERSION);
        std::string version = utils::parseFirmwareVersionResponse(fw_payload);
        std::cout << "Firmware: " << version << "\n";

        // Example 3: Get product ID
        auto pid_payload = protocol.sendRequest(MessageType::GET_PRODUCT_ID);
        uint32_t product_id = utils::parseProductIdResponse(pid_payload);
        std::cout << "Product ID: 0x" << std::hex << product_id << "\n";

        // Example 4: Open file for reading
        uint32_t file_id = 0x00910000;
        protocol.sendCommand(MessageType::OPEN_FILE_READ,
                             utils::buildFilePayload(file_id));

        // Example 5: Get file size
        auto size_response = protocol.sendRequest(
            MessageType::GET_FILE_SIZE,
            utils::buildFilePayload(file_id));
        FileInfo file_info = utils::parseFileSizeResponse(size_response);
        std::cout << "File size: " << file_info.size << " bytes\n";

        // Example 6: Read file data (242 bytes per chunk for 256-byte packets)
        std::vector<uint8_t> file_data;
        size_t chunk_size = protocol.maxReadChunkSize(); // 242 bytes

        for (size_t offset = 0; offset < file_info.size; offset += chunk_size)
        {
            size_t to_read = std::min(chunk_size, file_info.size - offset);

            auto read_response = protocol.sendRequest(
                MessageType::READ_FILE_DATA_REQUEST,
                utils::buildReadPayload(file_id, to_read));

            auto chunk = utils::parseReadFileDataResponse(read_response);
            file_data.insert(file_data.end(), chunk.begin(), chunk.end());
        }

        // Example 7: Close file
        protocol.sendCommand(MessageType::CLOSE_FILE,
                             utils::buildFilePayload(file_id));

        // Example 8: Enumerate files
        auto find_response = protocol.sendRequest(
            MessageType::FIND_FIRST_FILE,
            utils::buildFindFirstPayload());

        FileInfo first_file = utils::parseFindFileResponse(find_response);
        std::cout << "First file: 0x" << std::hex << first_file.file_id
                  << " (" << std::dec << first_file.size << " bytes)\n";

        // Continue with FIND_NEXT_FILE...
        while (!first_file.end_of_list)
        {
            auto next_response = protocol.sendRequest(MessageType::FIND_NEXT_FILE);
            first_file = utils::parseFindFileResponse(next_response);
            if (!first_file.end_of_list)
            {
                std::cout << "File: 0x" << std::hex << first_file.file_id
                          << " (" << std::dec << first_file.size << " bytes)\n";
            }
        }

        protocol.sendCommand(MessageType::FIND_CLOSE);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Protocol error: " << e.what() << "\n";
        return 1;
    }

    watch->connection->close();
    return 0;
}
