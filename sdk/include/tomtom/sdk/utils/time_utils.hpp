#pragma once

#include <ctime>
#include <string>
#include <cstdint>

namespace tomtom::sdk::utils
{
    /**
     * @brief TomTom epoch (Jan 1, 2010)
     */
    constexpr time_t TOMTOM_EPOCH = 1262304000; // Unix timestamp for 2010-01-01 00:00:00 UTC

    /**
     * @brief Convert TomTom timestamp to Unix timestamp
     *
     * TomTom timestamps are seconds since Jan 1, 2010
     */
    inline std::time_t tomtomToUnix(uint32_t tomtom_time)
    {
        return TOMTOM_EPOCH + static_cast<std::time_t>(tomtom_time);
    }

    /**
     * @brief Convert Unix timestamp to TomTom timestamp
     */
    inline uint32_t unixToTomTom(std::time_t unix_time)
    {
        return static_cast<uint32_t>(unix_time - TOMTOM_EPOCH);
    }

    /**
     * @brief Format time_t as ISO 8601 string
     */
    std::string formatISO8601(std::time_t time)
    {
        char buffer[20];
        std::tm *tm_info = std::gmtime(&time);
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", tm_info);
        return std::string(buffer);
    }

    /**
     * @brief Format time_t for filenames (YYYY-MM-DD_HH-MM-SS)
     */
    std::string formatForFilename(std::time_t time)
    {
        char buffer[20];
        std::tm *tm_info = std::gmtime(&time);
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", tm_info);
        return std::string(buffer);
    }

    /**
     * @brief Get local time offset in seconds
     */
    int32_t getLocalTimeOffset()
    {
        std::time_t now = std::time(nullptr);
        std::tm *local_tm = std::localtime(&now);
        std::tm *gmt_tm = std::gmtime(&now);

        return static_cast<int32_t>(std::difftime(std::mktime(local_tm), std::mktime(gmt_tm)));
    }

}
