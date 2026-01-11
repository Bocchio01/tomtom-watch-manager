#pragma once

#include <functional>
#include <string>
#include <cstdarg>
#include <cstdio>
#include <vector>

namespace tomtom::core::utils
{

    /**
     * @brief Log levels for logging utility
     */
    enum class LogLevel
    {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

    /**
     * @brief Log callback type
     * @param level Log level
     * @param message Log message
     */
    using LogCallback = std::function<void(LogLevel level, const std::string &message)>;

    /**
     * @brief Simple logging utility with callback support
     *
     * Allows setting a custom log callback to handle log messages.
     * Supports formatted logging with variable arguments.
     */
    class Logger
    {
    public:
        /**
         * @brief Set log callback
         * @param callback Log callback function
         */
        static void set_callback(LogCallback callback)
        {
            get_callback() = callback;
        }

        /**
         * @brief Log a formatted message
         * @param level Log level
         * @param fmt Format string
         * @param ... Variable arguments
         */
        static void log(LogLevel level, const char *fmt, ...)
        {
            auto &cb = get_callback();
            if (!cb)
                return;

            // Simple formatting buffer
            char buffer[1024];
            va_list args;
            va_start(args, fmt);
            std::vsnprintf(buffer, sizeof(buffer), fmt, args);
            va_end(args);

            cb(level, std::string(buffer));
        }

    private:
        /**
         * @brief Get static log callback reference
         * @return Reference to log callback
         */
        static LogCallback &get_callback()
        {
            static LogCallback cb;
            return cb;
        }
    };

}

#define LOG_CORE_TRACE(...) tomtom::core::utils::Logger::log(tomtom::core::utils::LogLevel::Trace, __VA_ARGS__)
#define LOG_CORE_DEBUG(...) tomtom::core::utils::Logger::log(tomtom::core::utils::LogLevel::Debug, __VA_ARGS__)
#define LOG_CORE_INFO(...) tomtom::core::utils::Logger::log(tomtom::core::utils::LogLevel::Info, __VA_ARGS__)
#define LOG_CORE_WARNING(...) tomtom::core::utils::Logger::log(tomtom::core::utils::LogLevel::Warning, __VA_ARGS__)
#define LOG_CORE_ERROR(...) tomtom::core::utils::Logger::log(tomtom::core::utils::LogLevel::Error, __VA_ARGS__)
#define LOG_CORE_CRITICAL(...) tomtom::core::utils::Logger::log(tomtom::core::utils::LogLevel::Critical, __VA_ARGS__)