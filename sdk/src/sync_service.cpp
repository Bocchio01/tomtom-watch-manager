#include "tomtom/sdk/sync_service.hpp"
#include <spdlog/spdlog.h>

namespace tomtom::sdk
{

    SyncService::SyncService(
        std::shared_ptr<tomtom::Watch> watch,
        const std::filesystem::path &activity_destination,
        const std::vector<ActivityExporter::Format> &export_formats)
        : watch_(std::move(watch)),
          activity_destination_(activity_destination),
          export_formats_(export_formats)
    {
        if (!watch_)
        {
            throw std::invalid_argument("Watch cannot be null");
        }

        // Initialize managers
        settings_ = std::make_unique<SettingsManager>(watch_);
        gps_ = std::make_unique<GpsManager>(watch_);
        activity_ = std::make_unique<ActivityManager>(watch_);
    }

    void SyncService::runAutoSync(Config config, ProgressCallback progress)
    {
        spdlog::info("Starting auto-sync sequence");

        int total_stages = 0;
        int current_stage = 0;

        // Count enabled stages
        if (config.sync_time)
            total_stages++;
        if (config.update_gps)
            total_stages++;
        if (config.download_activities)
            total_stages++;

        if (total_stages == 0)
        {
            spdlog::warn("No sync operations enabled");
            return;
        }

        try
        {
            // Stage 1: Time Sync
            if (config.sync_time)
            {
                current_stage++;
                if (progress)
                {
                    progress("Time Sync", 0, "Synchronizing watch time...");
                }

                spdlog::info("Stage {}/{}: Synchronizing time", current_stage, total_stages);
                settings_->syncTime();

                if (progress)
                {
                    progress("Time Sync", 100, "Time synchronized");
                }
            }

            // Stage 2: GPS QuickFix Update
            if (config.update_gps)
            {
                current_stage++;
                if (progress)
                {
                    progress("GPS Update", 0, "Updating GPS QuickFix data...");
                }

                spdlog::info("Stage {}/{}: Updating GPS QuickFix", current_stage, total_stages);

                // Create progress wrapper to convert GPS progress to stage progress
                auto gps_progress = [&progress](size_t downloaded, size_t total)
                {
                    if (progress && total > 0)
                    {
                        int percent = static_cast<int>((downloaded * 100) / total);
                        progress("GPS Update", percent, "Downloading ephemeris data...");
                    }
                };

                gps_->updateQuickFix(GpsManager::DEFAULT_URL, 7, gps_progress);

                if (progress)
                {
                    progress("GPS Update", 100, "GPS data updated");
                }
            }

            // Stage 3: Download Activities
            if (config.download_activities)
            {
                current_stage++;
                if (progress)
                {
                    progress("Activities", 0, "Downloading activities...");
                }

                spdlog::info("Stage {}/{}: Downloading activities", current_stage, total_stages);

                // Create progress wrapper to convert activity progress to stage progress
                auto activity_progress = [&progress](size_t current, size_t total, const std::string &message)
                {
                    if (progress && total > 0)
                    {
                        int percent = static_cast<int>((current * 100) / total);
                        progress("Activities", percent, message);
                    }
                };

                size_t downloaded = activity_->downloadNewActivities(
                    activity_destination_,
                    export_formats_,
                    activity_progress);

                if (progress)
                {
                    progress("Activities", 100, "Downloaded " + std::to_string(downloaded) + " activities");
                }
            }

            spdlog::info("Auto-sync completed successfully");

            if (progress)
            {
                progress("Complete", 100, "All operations completed");
            }
        }
        catch (const std::exception &e)
        {
            spdlog::error("Auto-sync failed: {}", e.what());
            if (progress)
            {
                progress("Error", 0, std::string("Failed: ") + e.what());
            }
            throw;
        }
    }

    void SyncService::setActivityDestination(const std::filesystem::path &destination)
    {
        activity_destination_ = destination;
        spdlog::debug("Activity destination set to: {}", destination.string());
    }

    void SyncService::setExportFormats(const std::vector<ActivityExporter::Format> &formats)
    {
        export_formats_ = formats;
        spdlog::debug("Export formats updated ({} formats)", formats.size());
    }

} // namespace tomtom::sdk
