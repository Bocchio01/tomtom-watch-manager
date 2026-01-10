#include <iostream>
#include <spdlog/spdlog.h>

#include "tomtom/protocol/definition/protocol.hpp"
#include "tomtom/manager.hpp"
#include "tomtom/watch.hpp"
#include "../test_utils.hpp"

using namespace tomtom;

// Test counter
int tests_passed = 0;
int tests_failed = 0;

// =============================================================================
// Manager Tests (Require Hardware)
// =============================================================================
Manager manager;
auto watch = manager.connectToWatch();

TEST(test_get_time)
{
    auto time = watch->info().getTime();
    spdlog::info("Watch time (Unix timestamp): {}", std::asctime(std::gmtime(&time)));
}

TEST(test_get_firmware_version)
{
    auto version = watch->info().getFirmwareVersion();
    spdlog::info("Firmware version: {}", version);
}

TEST(test_list_files)
{
    auto files = watch->files().listFiles();
    spdlog::info("Files on watch:");
    for (const auto &file : files)
    {
        spdlog::info("  File ID: 0x{:08X}, Size: {} bytes", file.id.value, file.size);
    }
}

TEST(test_read_files)
{
    services::FileId test_file_id(0x00F20000);
    auto data = watch->files().readFile(test_file_id);
    spdlog::info("Read {} bytes from file ID 0x{:08X}", data.size(), test_file_id.value);

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
    // spdlog::info("HEX  : {}", oss_hex.str());
    spdlog::info("ASCII: {}", oss_ascii.str());
}

// =============================================================================
// Main Test Runner
// =============================================================================

int main()
{
    spdlog::set_level(spdlog::level::info);
    spdlog::info("=================================================");
    spdlog::info("Running Manager Integration Tests");
    spdlog::info("Note: These tests require a connected watch");
    spdlog::info("=================================================");

    // Run tests
    RUN_TEST(test_get_time);
    RUN_TEST(test_get_firmware_version);
    // RUN_TEST(test_list_files);
    RUN_TEST(test_read_files);

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