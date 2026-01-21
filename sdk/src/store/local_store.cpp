#include <fstream>
#include <iomanip>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <ctime>
#include <algorithm>

#include "tomtom/sdk/store/local_store.hpp"
#include "tomtom/sdk/utils/time_utils.hpp"
#include "tomtom/sdk/models/activity_types.hpp"

namespace fs = std::filesystem;

namespace tomtom::sdk::store
{
    LocalStore::LocalStore(const fs::path &config_path)
        : configFilePath_(config_path)
    {
        // Set defaults if load fails
        if (fs::exists(config_path))
        {
            loadConfig();
        }
        else
        {
            // Create default storage root in current directory
            config_.storage_root = fs::current_path() / "TomTomData";
            spdlog::info("No config found, using default storage at: {}", config_.storage_root.string());
        }
    }

    void LocalStore::loadConfig()
    {
        spdlog::debug("Loading config from {}", configFilePath_.string());

        try
        {
            std::ifstream file(configFilePath_);
            if (!file.is_open())
            {
                spdlog::warn("Config file not found, using defaults");
                return;
            }

            nlohmann::json j;
            file >> j;

            // Parse storage root
            if (j.contains("storage_root"))
            {
                config_.storage_root = j["storage_root"].get<std::string>();
            }

            // Parse directory layout
            if (j.contains("layout"))
            {
                std::string layout = j["layout"].get<std::string>();
                if (layout == "Flat")
                    config_.layout = DirectoryLayout::Flat;
                else if (layout == "ByDate")
                    config_.layout = DirectoryLayout::ByDate;
                else if (layout == "BySport")
                    config_.layout = DirectoryLayout::BySport;
                else if (layout == "ByDevice")
                    config_.layout = DirectoryLayout::ByDevice;
            }

            // Parse export formats
            if (j.contains("export_formats") && j["export_formats"].is_array())
            {
                config_.export_formats = j["export_formats"].get<std::vector<std::string>>();
            }

            // Parse archive option
            if (j.contains("archive_raw_data"))
            {
                config_.archive_raw_data = j["archive_raw_data"].get<bool>();
            }

            // Parse log level
            if (j.contains("log_level"))
            {
                config_.log_level = j["log_level"].get<std::string>();
            }

            spdlog::info("Configuration loaded successfully");
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to load config: {}", e.what());
            spdlog::info("Using default configuration");
        }
    }

    void LocalStore::saveConfig()
    {
        spdlog::debug("Saving config to {}", configFilePath_.string());

        try
        {
            ensureDirectory(configFilePath_.parent_path());

            nlohmann::json j;
            j["storage_root"] = config_.storage_root.string();

            // Convert layout enum to string
            switch (config_.layout)
            {
            case DirectoryLayout::Flat:
                j["layout"] = "Flat";
                break;
            case DirectoryLayout::ByDate:
                j["layout"] = "ByDate";
                break;
            case DirectoryLayout::BySport:
                j["layout"] = "BySport";
                break;
            case DirectoryLayout::ByDevice:
                j["layout"] = "ByDevice";
                break;
            }

            j["export_formats"] = config_.export_formats;
            j["archive_raw_data"] = config_.archive_raw_data;
            j["log_level"] = config_.log_level;

            std::ofstream file(configFilePath_);
            if (!file.is_open())
            {
                throw std::runtime_error("Failed to open config file for writing");
            }

            // Write formatted JSON with 2-space indentation
            file << j.dump(2) << std::endl;

            spdlog::info("Configuration saved successfully");
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to save config: {}", e.what());
            throw;
        }
    }

    // ========================================================================
    // Path Logic
    // ========================================================================

    fs::path LocalStore::getSystemRoot() const
    {
        // Hidden folder for raw data and logs
        return config_.storage_root / ".tomtom";
    }

    fs::path LocalStore::getUserRoot() const
    {
        return config_.storage_root;
    }

    fs::path LocalStore::getLogDirectory() const
    {
        fs::path logDir = getSystemRoot() / "logs";
        ensureDirectory(logDir);
        return logDir;
    }

    std::string LocalStore::buildActivityFilename(const models::Activity &activity) const
    {
        // Format: YYYY-MM-DD_HH-MM-SS_Sport
        std::string timeStr = utils::formatForFilename(activity.start_time);
        std::string sportStr = std::string(models::toString(activity.type));

        // Remove spaces from sport name (e.g., "Trail Running" -> "TrailRunning")
        sportStr.erase(std::remove(sportStr.begin(), sportStr.end(), ' '), sportStr.end());

        return timeStr + "_" + sportStr;
    }

    fs::path LocalStore::buildActivityPath(const models::Activity &activity, const std::string &extension) const
    {
        std::tm *tm_info = std::gmtime(&activity.start_time);
        std::string year = std::to_string(tm_info->tm_year + 1900);

        std::ostringstream month_ss;
        month_ss << std::setw(2) << std::setfill('0') << (tm_info->tm_mon + 1);
        std::string month = month_ss.str();

        std::string sport = std::string(models::toString(activity.type));
        sport.erase(std::remove(sport.begin(), sport.end(), ' '), sport.end());

        fs::path dir = getUserRoot();

        // Directory Layout Strategy
        switch (config_.layout)
        {
        case DirectoryLayout::ByDate:
            // Structure: /2023/05/Running_...
            dir /= year;
            dir /= month;
            break;

        case DirectoryLayout::BySport:
            // Structure: /Running/2023/Running_...
            dir /= sport;
            dir /= year;
            break;

        case DirectoryLayout::ByDevice:
            // Structure: /12345ABC/Running/Running_...
            dir /= std::to_string(activity.product_id); // Or serial if available in Activity struct
            dir /= sport;
            break;

        case DirectoryLayout::Flat:
        default:
            // Structure: /Running_...
            break;
        }

        std::string filename = buildActivityFilename(activity) + "." + extension;
        return dir / filename;
    }

    // ========================================================================
    // Persistence Actions
    // ========================================================================

    bool LocalStore::hasRawActivity(const models::Activity &activity) const
    {
        // Raw files are stored in .tomtom/archive/YYYY/filename.ttbin
        // regardless of User Layout preference, to keep the archive stable.

        std::tm *tm_info = std::gmtime(&activity.start_time);
        std::string year = std::to_string(tm_info->tm_year + 1900);

        fs::path archiveDir = getSystemRoot() / "archive" / year;
        fs::path filePath = archiveDir / (buildActivityFilename(activity) + ".ttbin");

        return fs::exists(filePath);
    }

    void LocalStore::saveRawActivity(const models::Activity &activity, const std::vector<uint8_t> &data)
    {
        if (!config_.archive_raw_data)
            return;

        std::tm *tm_info = std::gmtime(&activity.start_time);
        std::string year = std::to_string(tm_info->tm_year + 1900);

        fs::path archiveDir = getSystemRoot() / "archive" / year;
        ensureDirectory(archiveDir);

        fs::path filePath = archiveDir / (buildActivityFilename(activity) + ".ttbin");

        spdlog::debug("Archiving raw data to: {}", filePath.string());

        std::ofstream file(filePath, std::ios::binary);
        if (!file.write(reinterpret_cast<const char *>(data.data()), data.size()))
        {
            throw std::runtime_error("Failed to write raw activity file");
        }
    }

    fs::path LocalStore::saveExportedActivity(
        const models::Activity &activity,
        const std::string &content,
        const std::string &extension)
    {
        fs::path path = buildActivityPath(activity, extension);
        ensureDirectory(path.parent_path());

        spdlog::info("Saving {} export to: {}", extension, path.string());

        std::ofstream file(path);
        if (!file)
        {
            throw std::runtime_error("Failed to open export file for writing: " + path.string());
        }

        file << content;
        return path;
    }

    void LocalStore::ensureDirectory(const fs::path &path) const
    {
        if (!path.empty() && !fs::exists(path))
        {
            try
            {
                fs::create_directories(path);
            }
            catch (const std::exception &e)
            {
                spdlog::error("Failed to create directory {}: {}", path.string(), e.what());
                throw;
            }
        }
    }
}
