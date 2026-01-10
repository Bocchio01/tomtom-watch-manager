#include "tomtom/sdk/quickfix_manager.hpp"
#include "tomtom/sdk/networking/http_client.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <chrono>

namespace tomtom::sdk
{

    QuickFixManager::QuickFixManager(
        std::shared_ptr<tomtom::Watch> watch,
        const std::filesystem::path &cache_dir)
        : watch_(std::move(watch)), cache_dir_(cache_dir)
    {
        if (!watch_)
        {
            throw std::invalid_argument("Watch cannot be null");
        }
        ensureCacheDir();
    }

    void QuickFixManager::updateFromUrl(
        const std::string &url,
        int days,
        bool reset_gps,
        ProgressCallback progress)
    {
        if (!validateUrl(url))
        {
            throw std::invalid_argument("Invalid URL: " + url);
        }

        if (!validateDays(days))
        {
            throw std::invalid_argument("Days must be 3 or 7");
        }

        spdlog::info("Updating GPS QuickFix from URL ({} days)", days);

        // Step 1: Build URL (lib operation)
        std::string final_url = buildUrl(url, days);
        spdlog::debug("Ephemeris URL: {}", final_url);

        // Step 2: Download data (lib operation - network)
        spdlog::info("Downloading GPS QuickFix data...");
        auto result = networking::downloadFile(final_url, progress);

        if (!result.success())
        {
            throw std::runtime_error("Failed to download ephemeris: " + result.error_message);
        }

        if (result.data.empty())
        {
            throw std::runtime_error("Downloaded data is empty");
        }

        spdlog::info("Downloaded {} bytes", result.data.size());

        // Optional: Cache the data
        try
        {
            ensureCacheDir();
            std::string cache_filename = generateCacheFilename(days);
            std::filesystem::path cache_path = cache_dir_ / cache_filename;

            std::ofstream cache_file(cache_path, std::ios::binary);
            if (cache_file)
            {
                cache_file.write(reinterpret_cast<const char *>(result.data.data()), result.data.size());
                spdlog::debug("Cached ephemeris data to: {}", cache_path.string());
            }
        }
        catch (const std::exception &e)
        {
            spdlog::warn("Failed to cache ephemeris data: {}", e.what());
            // Continue anyway - caching is optional
        }

        // Step 3: Write to watch (core operation - device)
        watch_->gpsQuickFix().updateEphemeris(result.data, reset_gps);

        spdlog::info("GPS QuickFix update completed successfully");
    }

    void QuickFixManager::updateFromFile(
        const std::filesystem::path &file_path,
        bool reset_gps)
    {
        spdlog::info("Updating GPS QuickFix from file: {}", file_path.string());

        // Step 1: Read file (lib operation - filesystem)
        std::ifstream file(file_path, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Failed to open file: " + file_path.string());
        }

        std::vector<uint8_t> data(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());

        if (data.empty())
        {
            throw std::runtime_error("File is empty");
        }

        spdlog::info("Read {} bytes from file", data.size());

        // Step 2: Write to watch (core operation - device)
        watch_->gpsQuickFix().updateEphemeris(data, reset_gps);

        spdlog::info("GPS QuickFix update completed successfully");
    }

    std::filesystem::path QuickFixManager::downloadAndCache(
        const std::string &url,
        int days,
        ProgressCallback progress)
    {
        if (!validateUrl(url))
        {
            throw std::invalid_argument("Invalid URL: " + url);
        }

        if (!validateDays(days))
        {
            throw std::invalid_argument("Days must be 3 or 7");
        }

        spdlog::info("Downloading GPS QuickFix data ({} days)", days);

        // Build URL
        std::string final_url = buildUrl(url, days);
        spdlog::debug("Ephemeris URL: {}", final_url);

        // Download
        auto result = networking::downloadFile(final_url, progress);

        if (!result.success())
        {
            throw std::runtime_error("Failed to download ephemeris: " + result.error_message);
        }

        if (result.data.empty())
        {
            throw std::runtime_error("Downloaded data is empty");
        }

        spdlog::info("Downloaded {} bytes", result.data.size());

        // Cache
        ensureCacheDir();
        std::string cache_filename = generateCacheFilename(days);
        std::filesystem::path cache_path = cache_dir_ / cache_filename;

        std::ofstream cache_file(cache_path, std::ios::binary);
        if (!cache_file)
        {
            throw std::runtime_error("Failed to create cache file: " + cache_path.string());
        }

        cache_file.write(reinterpret_cast<const char *>(result.data.data()), result.data.size());
        cache_file.close();

        spdlog::info("Cached ephemeris data to: {}", cache_path.string());
        return cache_path;
    }

    bool QuickFixManager::updateFromCache(bool reset_gps)
    {
        auto cached_file = getMostRecentCachedFile();

        if (cached_file.empty())
        {
            spdlog::warn("No cached ephemeris files found");
            return false;
        }

        spdlog::info("Using cached ephemeris file: {}", cached_file.string());
        updateFromFile(cached_file, reset_gps);
        return true;
    }

    void QuickFixManager::setCacheDir(const std::filesystem::path &dir)
    {
        cache_dir_ = dir;
        ensureCacheDir();
    }

    void QuickFixManager::clearCache()
    {
        if (!std::filesystem::exists(cache_dir_))
        {
            return;
        }

        size_t count = 0;
        for (const auto &entry : std::filesystem::directory_iterator(cache_dir_))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".eph")
            {
                std::filesystem::remove(entry.path());
                count++;
            }
        }

        spdlog::info("Cleared {} cached ephemeris files", count);
    }

    std::filesystem::path QuickFixManager::getMostRecentCachedFile() const
    {
        if (!std::filesystem::exists(cache_dir_))
        {
            return {};
        }

        std::filesystem::path most_recent;
        std::filesystem::file_time_type most_recent_time;

        for (const auto &entry : std::filesystem::directory_iterator(cache_dir_))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".eph")
            {
                auto ftime = std::filesystem::last_write_time(entry);
                if (most_recent.empty() || ftime > most_recent_time)
                {
                    most_recent = entry.path();
                    most_recent_time = ftime;
                }
            }
        }

        return most_recent;
    }

    std::string QuickFixManager::buildUrl(const std::string &url_template, int days)
    {
        std::string url = url_template;
        std::string days_str = std::to_string(days);

        // Replace {DAYS} placeholder
        size_t pos = url.find("{DAYS}");
        if (pos != std::string::npos)
        {
            url.replace(pos, 6, days_str);
        }

        return url;
    }

    bool QuickFixManager::validateUrl(const std::string &url)
    {
        if (url.empty())
        {
            return false;
        }

        // Check if URL starts with http:// or https://
        if (url.find("http://") != 0 && url.find("https://") != 0)
        {
            return false;
        }

        return true;
    }

    bool QuickFixManager::validateDays(int days)
    {
        return days == 3 || days == 7;
    }

    std::string QuickFixManager::generateCacheFilename(int days) const
    {
        // Format: ephemeris_7day_YYYYMMDD_HHMMSS.eph
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::tm *tm = std::localtime(&time_t);

        std::ostringstream filename;
        filename << "ephemeris_" << days << "day_"
                 << std::put_time(tm, "%Y%m%d_%H%M%S")
                 << ".eph";

        return filename.str();
    }

    void QuickFixManager::ensureCacheDir()
    {
        if (!std::filesystem::exists(cache_dir_))
        {
            std::filesystem::create_directories(cache_dir_);
            spdlog::info("Created cache directory: {}", cache_dir_.string());
        }
    }

} // namespace tomtom::sdk
