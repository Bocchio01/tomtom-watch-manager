#pragma once

#include <memory>
#include <vector>
#include <filesystem>
#include <optional>

#include "tomtom/sdk/store/config.hpp"
#include "tomtom/sdk/models/activity_model.hpp"
#include "tomtom/sdk/utils/files_identifier.hpp"

namespace tomtom::sdk::store
{
    /**
     * @brief Manages local file persistence, directory structures, and configuration.
     */
    class LocalStore
    {
    public:
        explicit LocalStore(const std::filesystem::path &config_path = "config.json");

        // ====================================================================
        // Configuration
        // ====================================================================

        /**
         * @brief Reload configuration from disk
         */
        void loadConfig();

        /**
         * @brief Save current configuration to disk
         */
        void saveConfig();

        /**
         * @brief Get read-only access to config
         */
        const SdkConfig &getConfig() const { return config_; }

        /**
         * @brief Update config (does not auto-save)
         */
        void setConfig(const SdkConfig &config) { config_ = config; }

        // ====================================================================
        // Activity Management
        // ====================================================================

        /**
         * @brief Check if a raw activity file already exists in the archive
         * Used to skip downloading existing files.
         */
        bool hasRawActivity(const models::Activity &activity) const;

        /**
         * @brief Save the raw binary TTBIN file to the protected archive folder
         * This usually goes into .tomtom/archive/...
         */
        void saveRawActivity(const models::Activity &activity, const std::vector<uint8_t> &data);

        /**
         * @brief Save an exported activity (GPX, JSON, etc) to the user folder
         * This uses the configured DirectoryLayout.
         */
        std::filesystem::path saveExportedActivity(
            const models::Activity &activity,
            const std::string &content,
            const std::string &extension);

        // ====================================================================
        // System Paths
        // ====================================================================

        /**
         * @brief Get directory for logs
         */
        std::filesystem::path getLogDirectory() const;

        /**
         * @brief Get the configuration file path
         */
        std::filesystem::path getConfigPath() const { return configFilePath_; }

    private:
        std::filesystem::path configFilePath_;
        SdkConfig config_;

        // Helpers
        std::filesystem::path getSystemRoot() const; // For internal usage (.tomtom)
        std::filesystem::path getUserRoot() const;   // For user exports

        // Strategy pattern for naming
        std::filesystem::path buildActivityPath(const models::Activity &activity, const std::string &extension) const;
        std::string buildActivityFilename(const models::Activity &activity) const;

        void ensureDirectory(const std::filesystem::path &path) const;
    };
}
