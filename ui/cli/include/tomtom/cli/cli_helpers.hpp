#pragma once

#include <memory>
#include <string>
#include <vector>
#include <tomtom/sdk/manager.hpp> // The central manager we designed

namespace tomtom::cli
{

    /**
     * @brief Global CLI context shared across commands
     */
    struct CliContext
    {
        // Configuration
        std::string config_path;
        std::string activity_store_override;
        std::string device_filter;
        bool debug_mode = false;
        bool packet_logging = false;
        bool unsafe_mode = false;
        bool auto_yes = false;
        bool eph7days = false;

        // The Central Manager
        std::unique_ptr<sdk::Manager> manager;

        // Helper: Lazy initialization of the manager and connection
        sdk::Manager &getManager();

        // Helper to ask for confirmation
        bool confirmAction(const std::string &prompt);
    };

    /**
     * @brief Progress indicator helper
     */
    class ProgressIndicator
    {
    public:
        ProgressIndicator(const std::string &task_name);
        ~ProgressIndicator();

        void update(size_t current, size_t total, const std::string &message = "");
        void updateStage(const std::string &stage, int progress, const std::string &message = "");
        void complete(const std::string &message = "Done");
        void error(const std::string &message);

    private:
        std::string task_name_;
        bool completed_ = false;
        size_t last_percent_ = 0;
    };

    /**
     * @brief Formatting and Color helpers
     */
    namespace Color
    {
        std::string red(const std::string &text);
        std::string green(const std::string &text);
        std::string yellow(const std::string &text);
        std::string blue(const std::string &text);
        std::string bold(const std::string &text);
        std::string dim(const std::string &text);
    }

    namespace Format
    {
        std::string formatBytes(size_t bytes);
        std::string formatDuration(uint32_t seconds);
        std::string formatTime(std::time_t time);
        std::string formatDate(std::time_t time);
        std::string formatDateTime(std::time_t time);
    }

} // namespace tomtom::cli
