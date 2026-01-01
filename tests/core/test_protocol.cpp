#include <iostream>
#include <cstring>
#include <spdlog/spdlog.h>

#include "tomtom/interface/protocol/protocol.hpp"
#include "../test_utils.hpp"

using namespace tomtom::interface::protocol;

// Test counter
int tests_passed = 0;
int tests_failed = 0;

// =============================================================================
// Interface Types Tests
// =============================================================================

TEST(test_packet_direction_enum)
{
    ASSERT_EQ(static_cast<uint8_t>(0x09), static_cast<uint8_t>(PacketDirection::TX), "TX direction should be 0x09");
    ASSERT_EQ(static_cast<uint8_t>(0x01), static_cast<uint8_t>(PacketDirection::RX), "RX direction should be 0x01");
    ASSERT_EQ(static_cast<uint8_t>(0xFF), static_cast<uint8_t>(PacketDirection::UNKNOWN), "UNKNOWN direction should be 0xFF");
}

TEST(test_message_type_enum)
{
    ASSERT_EQ(static_cast<uint8_t>(0x02), static_cast<uint8_t>(MessageType::OPEN_FILE_WRITE), "OPEN_FILE_WRITE should be 0x02");
    ASSERT_EQ(static_cast<uint8_t>(0x06), static_cast<uint8_t>(MessageType::OPEN_FILE_READ), "OPEN_FILE_READ should be 0x06");
    ASSERT_EQ(static_cast<uint8_t>(0x0C), static_cast<uint8_t>(MessageType::CLOSE_FILE), "CLOSE_FILE should be 0x0C");
    ASSERT_EQ(static_cast<uint8_t>(0x14), static_cast<uint8_t>(MessageType::GET_WATCH_TIME), "GET_WATCH_TIME should be 0x14");
}

TEST(test_interface_error_enum)
{
    ASSERT_EQ(static_cast<uint32_t>(0x00000000), static_cast<uint32_t>(ProtocolError::SUCCESS), "SUCCESS should be 0x00000000");
    ASSERT_EQ(static_cast<uint32_t>(0x00000001), static_cast<uint32_t>(ProtocolError::UNKNOWN_ERROR), "UNKNOWN_ERROR should be 0x00000001");
    ASSERT_EQ(static_cast<uint32_t>(0x00000002), static_cast<uint32_t>(ProtocolError::FILE_NOT_FOUND), "FILE_NOT_FOUND should be 0x00000002");
}

TEST(test_file_id_struct)
{
    FileId file_id1(0x12345678);
    FileId file_id2(0x12345678);
    FileId file_id3(0xABCDEF00);

    ASSERT_EQ(0x12345678u, file_id1.value, "FileId value should be set correctly");
    ASSERT_TRUE(file_id1 == file_id2, "Equal FileIds should compare equal");
    ASSERT_TRUE(file_id1 != file_id3, "Different FileIds should compare not equal");
    ASSERT_EQ(0x12345678u, static_cast<uint32_t>(file_id1), "FileId should convert to uint32_t");
}

// =============================================================================
// Packet Header Tests
// =============================================================================

TEST(test_packet_header_size)
{
    ASSERT_EQ(4u, sizeof(PacketHeader), "PacketHeader should be exactly 4 bytes");
}

TEST(test_packet_header_layout)
{
    PacketHeader header;
    header.direction = PacketDirection::TX;
    header.length = 10;
    header.counter = 42;
    header.type = MessageType::OPEN_FILE_READ;

    // Verify the header is packed correctly
    ASSERT_EQ(PacketDirection::TX, header.direction, "Direction should be set");
    ASSERT_EQ(10u, header.length, "Length should be set");
    ASSERT_EQ(42u, header.counter, "Counter should be set");
    ASSERT_EQ(MessageType::OPEN_FILE_READ, header.type, "Type should be set");
}

// =============================================================================
// Payload Traits Tests
// =============================================================================

TEST(test_open_file_read_tx_traits)
{
    using Traits = PayloadTraits<OpenFileReadTxPayload>;
    ASSERT_EQ(MessageType::OPEN_FILE_READ, Traits::type, "OpenFileReadTxPayload type should be OPEN_FILE_READ");
    ASSERT_EQ(PacketDirection::TX, Traits::direction, "OpenFileReadTxPayload direction should be TX");
}

TEST(test_open_file_read_rx_traits)
{
    using Traits = PayloadTraits<OpenFileReadRxPayload>;
    ASSERT_EQ(MessageType::OPEN_FILE_READ, Traits::type, "OpenFileReadRxPayload type should be OPEN_FILE_READ");
    ASSERT_EQ(PacketDirection::RX, Traits::direction, "OpenFileReadRxPayload direction should be RX");
}

TEST(test_get_watch_time_tx_traits)
{
    using Traits = PayloadTraits<GetWatchTimeTxPayload>;
    ASSERT_EQ(MessageType::GET_WATCH_TIME, Traits::type, "GetWatchTimeTxPayload type should be GET_WATCH_TIME");
    ASSERT_EQ(PacketDirection::TX, Traits::direction, "GetWatchTimeTxPayload direction should be TX");
}

// =============================================================================
// Packet Tests
// =============================================================================

TEST(test_open_file_read_tx_packet_auto_header)
{
    OpenFileReadTx packet;
    packet.payload.file_id = FileId(0x12345678);

    ASSERT_EQ(PacketDirection::TX, packet.header.direction, "Packet direction should be auto-filled");
    ASSERT_EQ(MessageType::OPEN_FILE_READ, packet.header.type, "Packet type should be auto-filled");
    ASSERT_EQ(0u, packet.header.counter, "Packet counter should be initialized to 0");

    // Length should be counter (1) + type (1) + payload (4)
    ASSERT_EQ(6u, packet.header.length, "Packet length should be auto-calculated");
}

TEST(test_open_file_read_tx_packet_with_payload)
{
    OpenFileReadTxPayload payload;
    payload.file_id = FileId(0xABCDEF00);

    OpenFileReadTx packet(payload);

    ASSERT_EQ(0xABCDEF00u, packet.payload.file_id.value, "Payload should be set from constructor");
    ASSERT_EQ(PacketDirection::TX, packet.header.direction, "Header should be auto-filled");
}

TEST(test_packet_size_method)
{
    OpenFileReadTx packet;
    size_t expected_size = sizeof(PacketHeader) + sizeof(OpenFileReadTxPayload);
    ASSERT_EQ(expected_size, packet.size(), "Packet size() should return total packet size");
}

TEST(test_get_watch_time_tx_packet)
{
    GetWatchTimeTx packet;

    ASSERT_EQ(PacketDirection::TX, packet.header.direction, "GetWatchTimeTx direction should be TX");
    ASSERT_EQ(MessageType::GET_WATCH_TIME, packet.header.type, "GetWatchTimeTx type should be GET_WATCH_TIME");

    // Empty payload: length = counter (1) + type (1) + 0
    ASSERT_EQ(2u, packet.header.length, "Empty payload length should be 2");
}

TEST(test_read_file_data_tx_packet)
{
    ReadFileDataTx packet;
    packet.payload.file_id = FileId(0x00000013);
    packet.payload.length = 1024;

    ASSERT_EQ(PacketDirection::TX, packet.header.direction, "ReadFileDataTx direction should be TX");
    ASSERT_EQ(MessageType::READ_FILE_DATA_REQUEST, packet.header.type, "ReadFileDataTx type should be READ_FILE_DATA_REQUEST");
    ASSERT_EQ(0x00000013u, packet.payload.file_id.value, "File ID should be set");
    ASSERT_EQ(1024u, packet.payload.length, "Length should be set");
}

// =============================================================================
// Response Payload Tests
// =============================================================================

TEST(test_file_operation_response_size)
{
    ASSERT_EQ(20u, sizeof(FileOperationResponse), "FileOperationResponse should be 20 bytes");
}

TEST(test_file_size_response_size)
{
    ASSERT_EQ(20u, sizeof(FileSizeResponse), "FileSizeResponse should be 20 bytes");
}

TEST(test_find_response_size)
{
    ASSERT_EQ(20u, sizeof(FindResponse), "FindResponse should be 20 bytes");
}

TEST(test_file_operation_response_usage)
{
    OpenFileReadRx response_packet;
    response_packet.payload.file_id = FileId(0x12345678);
    response_packet.payload.error = static_cast<uint32_t>(ProtocolError::SUCCESS);

    ASSERT_EQ(0x12345678u, response_packet.payload.file_id.value, "Response file ID should be set");
    ASSERT_EQ(static_cast<uint32_t>(ProtocolError::SUCCESS), response_packet.payload.error, "Response error should be set");
}

// =============================================================================
// Packet Type Aliases Tests
// =============================================================================

TEST(test_tx_packet_aliases_exist)
{
    // This test just verifies that all TX packet aliases compile and can be instantiated
    OpenFileReadTx p1;
    OpenFileWriteTx p2;
    CloseFileTx p3;
    DeleteFileTx p4;
    GetFileSizeTx p5;
    ReadFileDataTx p6;
    WriteFileDataTx p7;
    FindFirstFileTx p8;
    FindNextFileTx p9;
    GetWatchTimeTx p10;
    GetFirmwareVersionTx p11;
    GetBleVersionTx p12;
    GetProductIdTx p13;
    FormatWatchTx p14;
    ResetDeviceTx p15;
    ResetGpsTx p16;

    ASSERT_TRUE(true, "All TX packet types should compile");
}

TEST(test_rx_packet_aliases_exist)
{
    // This test just verifies that all RX packet aliases compile and can be instantiated
    OpenFileReadRx p1;
    OpenFileWriteRx p2;
    CloseFileRx p3;
    DeleteFileRx p4;
    GetFileSizeRx p5;
    ReadFileDataRx p6;
    WriteFileDataRx p7;
    FindFirstFileRx p8;
    FindNextFileRx p9;
    GetWatchTimeRx p10;
    GetFirmwareVersionRx p11;
    GetBleVersionRx p12;
    GetProductIdRx p13;
    FormatWatchRx p14;
    ResetGpsRx p15;

    ASSERT_TRUE(true, "All RX packet types should compile");
}

// =============================================================================
// Main Test Runner
// =============================================================================

int main()
{
    spdlog::set_level(spdlog::level::info);
    spdlog::info("=================================================");
    spdlog::info("Running Interface Structure Unit Tests");
    spdlog::info("=================================================");

    // Interface types tests
    RUN_TEST(test_packet_direction_enum);
    RUN_TEST(test_message_type_enum);
    RUN_TEST(test_interface_error_enum);
    RUN_TEST(test_file_id_struct);

    // Packet header tests
    RUN_TEST(test_packet_header_size);
    RUN_TEST(test_packet_header_layout);

    // Payload traits tests
    RUN_TEST(test_open_file_read_tx_traits);
    RUN_TEST(test_open_file_read_rx_traits);
    RUN_TEST(test_get_watch_time_tx_traits);

    // Packet tests
    RUN_TEST(test_open_file_read_tx_packet_auto_header);
    RUN_TEST(test_open_file_read_tx_packet_with_payload);
    RUN_TEST(test_packet_size_method);
    RUN_TEST(test_get_watch_time_tx_packet);
    RUN_TEST(test_read_file_data_tx_packet);

    // Response payload tests
    RUN_TEST(test_file_operation_response_size);
    RUN_TEST(test_file_size_response_size);
    RUN_TEST(test_find_response_size);
    RUN_TEST(test_file_operation_response_usage);

    // Packet type aliases tests
    RUN_TEST(test_tx_packet_aliases_exist);
    RUN_TEST(test_rx_packet_aliases_exist);

    // Print results
    spdlog::info("=================================================");
    spdlog::info("Test Results:");
    spdlog::info("  Passed: {}", tests_passed);
    spdlog::info("  Failed: {}", tests_failed);
    spdlog::info("=================================================");

    if (tests_failed > 0)
    {
        spdlog::error("Some tests failed!");
        return 1;
    }

    spdlog::info("All tests passed!");
    return 0;
}
