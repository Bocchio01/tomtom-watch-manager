#include <spdlog/spdlog.h>
#include <chrono>
#include <ctime>

#include "tomtom/sdk/services/device_service.hpp"
#include "tomtom/sdk/utils/binary_writer.hpp"
#include "tomtom/sdk/utils/files_identifier.hpp"

namespace tomtom::sdk::services
{
    // Note: This ID is often 0x00000002 or specific to the model.
    // For this implementation, we will assume a standard Time File ID exists.
    // This may need adjustment based on specific watch models.
    constexpr utils::FileId TIME_CONFIG_FILE = utils::FileId(0x00000002);

    DeviceService::DeviceService(std::shared_ptr<core::Watch> watch)
        : watch_(std::move(watch))
    {
    }

    void DeviceService::syncTime()
    {
        spdlog::info("Syncing watch time with system time");

        // Create time payload
        // Structure depends on specific FW, usually:
        // Year(2) Month(1) Day(1) Hour(1) Min(1) Sec(1) + padding
        utils::BinaryWriter writer;

        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

#ifdef _WIN32
        // Use localtime_s on Windows (thread-safe)
        std::tm tm_buf;
        localtime_s(&tm_buf, &now_c);
        std::tm *tm = &tm_buf;
#else
        // Use localtime_r on Unix (thread-safe)
        std::tm tm_buf;
        localtime_r(&now_c, &tm_buf);
        std::tm *tm = &tm_buf;
#endif

        writer.writeU16(static_cast<uint16_t>(tm->tm_year + 1900));
        writer.writeU8(static_cast<uint8_t>(tm->tm_mon + 1));
        writer.writeU8(static_cast<uint8_t>(tm->tm_mday));
        writer.writeU8(static_cast<uint8_t>(tm->tm_hour));
        writer.writeU8(static_cast<uint8_t>(tm->tm_min));
        writer.writeU8(static_cast<uint8_t>(tm->tm_sec));

        // Sometimes a millisecond or timezone offset is required here
        writer.writePadding(2);

        // Write to generic "Set Time" file or similar mechanism
        // Note: If your core::Watch exposes a specific setTime command, use that instead.
        try
        {
            watch_->files().writeFile(TIME_CONFIG_FILE.value, writer.toVector());
            spdlog::info("Time synced successfully: {}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}",
                         tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                         tm->tm_hour, tm->tm_min, tm->tm_sec);
        }
        catch (const std::exception &e)
        {
            std::string errorMsg = "Failed to sync time: " + std::string(e.what());
            spdlog::error("{}", errorMsg);
            throw std::runtime_error(errorMsg);
        }
    }

    void DeviceService::setWatchName(const std::string &name)
    {
        spdlog::info("Setting watch name to: {}", name);

        // Note: Implementation depends on specific watch model and firmware.
        // This may involve writing to a specific preferences field or a dedicated file.
        // For now, we'll log a warning that this is not yet fully implemented.

        spdlog::warn("setWatchName is not yet fully implemented for all models");

        // TODO: Implement actual name setting based on watch model
        // This might involve:
        // 1. Writing to preferences file with a specific field
        // 2. Writing to a Bluetooth name configuration file
        // 3. Using a specific command protocol

        throw std::runtime_error("setWatchName is not yet implemented");
    }

    DeviceStats DeviceService::getStats()
    {
        spdlog::debug("Getting device statistics");

        DeviceStats stats;

        // Get basic info from watch
        try
        {
            // Note: These would typically come from reading specific files or
            // from the device info obtained during connection
            stats.serial_number = "Unknown"; // TODO: Get from device info
            stats.product_name = "TomTom Watch";
            stats.battery_level = -1; // -1 indicates unavailable

            spdlog::debug("Retrieved device stats (partially mocked)");
        }
        catch (const std::exception &e)
        {
            spdlog::warn("Failed to get complete device stats: {}", e.what());
        }

        return stats;
    }
}
