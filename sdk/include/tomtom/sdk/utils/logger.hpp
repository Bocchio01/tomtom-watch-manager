#pragma once

#include <spdlog/spdlog.h>
#include <memory>

namespace tomtom::sdk::utils
{
    /**
     * @brief Get the SDK logger instance
     */
    inline std::shared_ptr<spdlog::logger> getLogger()
    {
        auto logger = spdlog::get("tomtom_sdk");
        if (!logger)
        {
            logger = spdlog::default_logger()->clone("tomtom_sdk");
        }
        return logger;
    }

    /**
     * @brief Initialize SDK logging
     */
    inline void initializeLogging(spdlog::level::level_enum level = spdlog::level::info)
    {
        auto logger = getLogger();
        logger->set_level(level);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
    }

}
