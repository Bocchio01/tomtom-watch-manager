#include "tomtom/sdk/activity_manager.hpp"
#include "tomtom/sdk/activity_exporter.hpp"
#include <tomtom/services/activity/activity_parser.hpp>
#include <spdlog/spdlog.h>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace tomtom::sdk
{

    ActivityManager::ActivityManager(std::shared_ptr<tomtom::Watch> watch)
        : watch_(std::move(watch))
    {
        if (!watch_)
        {
            throw std::invalid_argument("Watch cannot be null");
        }
    }

    size_t ActivityManager::downloadNewActivities(
        const std::filesystem::path &destination,
        const std::vector<ActivityExporter::Format> &export_formats,
        ProgressCallback progress)
    {
        // Ensure destination directory exists
        std::filesystem::create_directories(destination);

        // Get list of activities on watch
        auto activities = watch_->activities().list();
        size_t total = activities.size();

        spdlog::info("Downloading {} activities from watch", total);

        size_t completed = 0;
        for (size_t i = 0; i < activities.size(); ++i)
        {
            try
            {
                if (progress)
                {
                    std::string msg = "Downloading activity " + std::to_string(i + 1) + "/" + std::to_string(total);
                    progress(i, total, msg);
                }

                // Download the activity
                downloadActivity(static_cast<uint16_t>(i), destination, export_formats, nullptr);
                completed++;
            }
            catch (const std::exception &e)
            {
                spdlog::warn("Failed to download activity {}: {}", i, e.what());
            }
        }

        if (progress)
        {
            progress(total, total, "Download complete");
        }

        spdlog::info("Successfully downloaded {} of {} activities", completed, total);
        return completed;
    }

    std::vector<std::filesystem::path> ActivityManager::downloadActivity(
        uint16_t index,
        const std::filesystem::path &destination,
        const std::vector<ActivityExporter::Format> &export_formats,
        ProgressCallback progress)
    {
        std::vector<std::filesystem::path> saved_files;

        spdlog::info("Downloading activity {} from watch", index);

        if (progress)
        {
            progress(0, 1 + export_formats.size(), "Downloading from watch...");
        }

        // Step 1: Download from watch
        auto activity = watch_->activities().get(index);

        if (progress)
        {
            progress(1, 1 + export_formats.size(), "Activity downloaded");
        }

        // Step 2: Export to requested formats
        size_t current_step = 1;
        for (const auto &format : export_formats)
        {
            if (progress)
            {
                std::string ext = ActivityExporter::getExtension(format);
                progress(current_step, 1 + export_formats.size(), "Exporting to " + ext + "...");
            }

            std::string content = ActivityExporter::exportFile(activity, format);
            std::string export_filename = generateFilename(activity, ActivityExporter::getExtension(format));
            std::filesystem::path export_path = destination / export_filename;

            std::ofstream export_file(export_path);
            if (!export_file)
            {
                throw std::runtime_error("Failed to create file: " + export_path.string());
            }
            export_file << content;
            export_file.close();

            saved_files.push_back(export_path);
            spdlog::info("Saved {} file: {}", ActivityExporter::getExtension(format), export_path.string());

            current_step++;
        }

        if (progress)
        {
            progress(1 + export_formats.size(), 1 + export_formats.size(), "Complete");
        }

        return saved_files;
    }

    std::vector<ActivityManager::ActivitySummary> ActivityManager::listWatchFiles()
    {
        std::vector<ActivitySummary> summaries;
        auto activities = watch_->activities().list();

        for (size_t i = 0; i < activities.size(); ++i)
        {
            const auto &activity = activities[i];
            ActivitySummary summary;
            summary.index = static_cast<uint16_t>(i);
            summary.type = std::string(tomtom::services::activity::toString(activity.type));
            summary.start_time = activity.start_time;
            summary.duration_seconds = activity.duration_seconds;
            summary.distance_meters = activity.distance_meters;
            summary.calories = activity.calories;
            summaries.push_back(summary);
        }

        return summaries;
    }

    std::vector<std::filesystem::path> ActivityManager::listLocalFiles(const std::filesystem::path &dir)
    {
        std::vector<std::filesystem::path> files;

        if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir))
        {
            return files;
        }

        for (const auto &entry : std::filesystem::directory_iterator(dir))
        {
            if (entry.is_regular_file())
            {
                auto ext = entry.path().extension().string();
                if (ext == ".ttbin" || ext == ".gpx" || ext == ".tcx" || ext == ".csv")
                {
                    files.push_back(entry.path());
                }
            }
        }

        return files;
    }

    size_t ActivityManager::clearWatchHistory()
    {
        spdlog::info("Clearing all activities from watch");
        return watch_->activities().removeAll();
    }

    std::string ActivityManager::generateFilename(
        const tomtom::services::activity::models::Activity &activity,
        const std::string &extension) const
    {
        // Format: YYYYMMDD_HHMMSS_Type.ext
        std::tm *tm = std::localtime(&activity.start_time);
        std::ostringstream filename;

        filename << std::put_time(tm, "%Y%m%d_%H%M%S")
                 << "_" << tomtom::services::activity::toString(activity.type)
                 << extension;

        return filename.str();
    }

} // namespace tomtom::sdk
