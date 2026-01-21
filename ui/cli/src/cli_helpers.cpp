#include <tomtom/cli/cli_helpers.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace tomtom::cli
{

    sdk::Manager &CliContext::getManager()
    {
        if (manager)
            return *manager;

        // 1. Initialize Manager
        manager = std::make_unique<sdk::Manager>();

        // 2. Configure Overrides
        if (!activity_store_override.empty())
        {
            auto config = manager->store().getConfig();
            config.storage_root = activity_store_override;
            manager->store().setConfig(config);
        }

        // 3. Connect to Watch
        std::cout << "Connecting to device..." << std::flush;
        try
        {
            std::shared_ptr<core::Watch> watch;

            if (!device_filter.empty())
            {
                // Try to connect to specific device by serial
                watch = manager->connectToWatch(device_filter);
            }
            else
            {
                // Connect to first available watch
                watch = manager->connectToWatch();
            }

            if (!watch)
            {
                throw std::runtime_error("No watch found");
            }

            std::cout << " Connected.\n";

            if (packet_logging)
            {
                // TODO: Enable packet logging if needed
                // manager->enablePacketLogging(true);
            }
        }
        catch (const std::exception &e)
        {
            throw std::runtime_error("Connection failed: " + std::string(e.what()));
        }

        return *manager;
    }

    bool CliContext::confirmAction(const std::string &prompt)
    {
        if (auto_yes)
            return true;

        std::cout << prompt << " [y/N]: " << std::flush;
        std::string response;
        std::getline(std::cin, response);

        return response == "y" || response == "Y" || response == "yes" || response == "YES";
    }

    // ========================================================================
    // ProgressIndicator
    // ========================================================================

    ProgressIndicator::ProgressIndicator(const std::string &task_name)
        : task_name_(task_name)
    {
        std::cout << task_name_ << "..." << std::flush;
    }

    ProgressIndicator::~ProgressIndicator()
    {
        if (!completed_)
        {
            std::cout << "\n";
        }
    }

    void ProgressIndicator::update(size_t current, size_t total, const std::string &message)
    {
        if (total == 0)
        {
            std::cout << "\r" << task_name_ << "... " << current << " " << message << std::flush;
            return;
        }

        size_t percent = (current * 100) / total;
        if (percent != last_percent_ || !message.empty())
        {
            std::cout << "\r" << task_name_ << "... " << percent << "% ";
            if (!message.empty())
                std::cout << "(" << message << ")";
            std::cout << std::flush;
            last_percent_ = percent;
        }
    }

    void ProgressIndicator::updateStage(const std::string &stage, int progress, const std::string &message)
    {
        std::cout << "\r" << task_name_ << " [" << stage << "] " << progress << "% ";
        if (!message.empty())
            std::cout << message;
        std::cout << std::flush;
    }

    void ProgressIndicator::complete(const std::string &message)
    {
        completed_ = true;
        std::cout << "\r" << task_name_ << "... " << Color::green("✓") << " " << message << "\n";
    }

    void ProgressIndicator::error(const std::string &message)
    {
        completed_ = true;
        std::cout << "\r" << task_name_ << "... " << Color::red("✗") << " " << message << "\n";
    }

    // ========================================================================
    // Color helpers
    // ========================================================================

    namespace Color
    {
        bool supportsColor()
        {
#ifdef _WIN32
            // Windows 10+ supports ANSI colors
            HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD mode;
            if (!GetConsoleMode(hOut, &mode))
                return false;
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            return SetConsoleMode(hOut, mode);
#else
            return isatty(STDOUT_FILENO);
#endif
        }

        std::string red(const std::string &text)
        {
            return supportsColor() ? "\033[31m" + text + "\033[0m" : text;
        }

        std::string green(const std::string &text)
        {
            return supportsColor() ? "\033[32m" + text + "\033[0m" : text;
        }

        std::string yellow(const std::string &text)
        {
            return supportsColor() ? "\033[33m" + text + "\033[0m" : text;
        }

        std::string blue(const std::string &text)
        {
            return supportsColor() ? "\033[34m" + text + "\033[0m" : text;
        }

        std::string bold(const std::string &text)
        {
            return supportsColor() ? "\033[1m" + text + "\033[0m" : text;
        }

        std::string dim(const std::string &text)
        {
            return supportsColor() ? "\033[2m" + text + "\033[0m" : text;
        }
    }

    // ========================================================================
    // Format helpers
    // ========================================================================

    namespace Format
    {
        std::string formatBytes(size_t bytes)
        {
            const char *units[] = {"B", "KB", "MB", "GB"};
            int unit = 0;
            double size = static_cast<double>(bytes);

            while (size >= 1024.0 && unit < 3)
            {
                size /= 1024.0;
                ++unit;
            }

            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2) << size << " " << units[unit];
            return ss.str();
        }

        std::string formatDuration(uint32_t seconds)
        {
            uint32_t hours = seconds / 3600;
            uint32_t minutes = (seconds % 3600) / 60;
            uint32_t secs = seconds % 60;

            std::ostringstream ss;
            if (hours > 0)
                ss << hours << "h " << minutes << "m " << secs << "s";
            else if (minutes > 0)
                ss << minutes << "m " << secs << "s";
            else
                ss << secs << "s";

            return ss.str();
        }

        std::string formatTime(std::time_t time)
        {
            char buffer[32];
            std::strftime(buffer, sizeof(buffer), "%H:%M:%S", std::localtime(&time));
            return buffer;
        }

        std::string formatDate(std::time_t time)
        {
            char buffer[32];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", std::localtime(&time));
            return buffer;
        }

        std::string formatDateTime(std::time_t time)
        {
            char buffer[64];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
            return buffer;
        }
    }

} // namespace tomtom::cli
