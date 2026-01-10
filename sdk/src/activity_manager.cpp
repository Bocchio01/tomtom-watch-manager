#include "tomtom/sdk/activity_manager.hpp"
#include "tomtom/sdk/activity_converter.hpp"
#include <tomtom/services/activity/activity_parser.hpp>
#include <spdlog/spdlog.h>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace tomtom::sdk
{

    ActivityManager::ActivityManager(
        std::shared_ptr<tomtom::Watch> watch,
        const std::filesystem::path &storage_dir)
        : watch_(std::move(watch)), storage_dir_(storage_dir)
    {
        if (!watch_)
        {
            throw std::invalid_argument("Watch cannot be null");
        }
        ensureStorageDir();
    }

    std::filesystem::path ActivityManager::downloadAndSave(
        uint16_t index,
        ExportFormat format,
        ProgressCallback progress)
    {
        spdlog::info("Downloading activity {} from watch", index);

        if (progress)
        {
            progress(0, 3, "Downloading from watch...");
        }

        // Step 1: Download from watch (core operation)
        auto activity = watch_->activities().get(index);

        if (progress)
        {
            progress(1, 3, "Converting to " + std::string(ActivityConverter::getExtension(format)));
        }

        // Step 2: Convert to requested format (lib operation)
        std::string content;
        switch (format)
        {
        case ExportFormat::GPX:
            content = ActivityConverter::toGPX(activity);
            break;
        case ExportFormat::TCX:
            content = ActivityConverter::toTCX(activity);
            break;
        case ExportFormat::CSV:
            content = ActivityConverter::toCSV(activity);
            break;
        default:
            throw std::invalid_argument("Unknown export format");
        }

        if (progress)
        {
            progress(2, 3, "Saving to disk...");
        }

        // Step 3: Save to disk (lib operation)
        ensureStorageDir();

        std::string filename = generateFilename(activity, format);
        std::filesystem::path file_path = storage_dir_ / filename;

        std::ofstream file(file_path);
        if (!file)
        {
            throw std::runtime_error("Failed to create file: " + file_path.string());
        }

        file << content;
        file.close();

        spdlog::info("Saved activity to: {}", file_path.string());

        if (progress)
        {
            progress(3, 3, "Complete");
        }

        return file_path;
    }

    size_t ActivityManager::downloadAll(
        ExportFormat format,
        ProgressCallback progress)
    {
        auto activities = watch_->activities().list();
        size_t total = activities.size();

        spdlog::info("Downloading {} activities from watch", total);

        size_t completed = 0;
        for (const auto &info : activities)
        {
            try
            {
                if (progress)
                {
                    std::string msg = "Activity " + std::to_string(completed + 1) + "/" + std::to_string(total);
                    progress(completed, total, msg);
                }

                downloadAndSave(info.index, format, nullptr);
                completed++;
            }
            catch (const std::exception &e)
            {
                spdlog::warn("Failed to download activity {}: {}", info.index, e.what());
            }
        }

        if (progress)
        {
            progress(total, total, "Complete");
        }

        spdlog::info("Downloaded {} of {} activities", completed, total);
        return completed;
    }

    std::filesystem::path ActivityManager::convertFile(
        const std::filesystem::path &source_path,
        ExportFormat target_format)
    {
        spdlog::info("Converting {} to {}", source_path.string(),
                     ActivityConverter::getExtension(target_format));

        // Read the source file
        std::ifstream file(source_path, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Failed to open file: " + source_path.string());
        }

        std::vector<uint8_t> data(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());

        // Parse the activity
        tomtom::services::activity::ActivityParser parser;
        auto activity = parser.parse(data);

        // Convert to target format
        std::string content;
        switch (target_format)
        {
        case ExportFormat::GPX:
            content = ActivityConverter::toGPX(activity);
            break;
        case ExportFormat::TCX:
            content = ActivityConverter::toTCX(activity);
            break;
        case ExportFormat::CSV:
            content = ActivityConverter::toCSV(activity);
            break;
        default:
            throw std::invalid_argument("Unknown export format");
        }

        // Generate output path
        std::filesystem::path output_path = source_path;
        output_path.replace_extension(ActivityConverter::getExtension(target_format));

        // Save converted file
        std::ofstream output(output_path);
        if (!output)
        {
            throw std::runtime_error("Failed to create file: " + output_path.string());
        }

        output << content;
        output.close();

        spdlog::info("Saved converted file to: {}", output_path.string());
        return output_path;
    }

    std::vector<std::filesystem::path> ActivityManager::listLocal() const
    {
        std::vector<std::filesystem::path> files;

        if (!std::filesystem::exists(storage_dir_))
        {
            return files;
        }

        for (const auto &entry : std::filesystem::directory_iterator(storage_dir_))
        {
            if (entry.is_regular_file())
            {
                auto ext = entry.path().extension().string();
                if (ext == ".gpx" || ext == ".tcx" || ext == ".csv" || ext == ".ttbin")
                {
                    files.push_back(entry.path());
                }
            }
        }

        return files;
    }

    void ActivityManager::setStorageDir(const std::filesystem::path &dir)
    {
        storage_dir_ = dir;
        ensureStorageDir();
    }

    size_t ActivityManager::getWatchActivityCount() const
    {
        return watch_->activities().count();
    }

    std::vector<tomtom::services::activity::models::ActivityInfo> ActivityManager::listOnWatch() const
    {
        return watch_->activities().list();
    }

    std::string ActivityManager::generateFilename(
        const tomtom::services::activity::models::Activity &activity,
        ExportFormat format) const
    {
        // Format: YYYY-MM-DD_HHMMSS_ActivityType.ext
        std::time_t time = activity.start_time;
        std::tm *tm = std::localtime(&time);

        std::ostringstream filename;
        filename << std::put_time(tm, "%Y-%m-%d_%H%M%S")
                 << "_" << tomtom::services::activity::toString(activity.type)
                 << ActivityConverter::getExtension(format);

        return filename.str();
    }

    void ActivityManager::ensureStorageDir()
    {
        if (!std::filesystem::exists(storage_dir_))
        {
            std::filesystem::create_directories(storage_dir_);
            spdlog::info("Created storage directory: {}", storage_dir_.string());
        }
    }

} // namespace tomtom::sdk
