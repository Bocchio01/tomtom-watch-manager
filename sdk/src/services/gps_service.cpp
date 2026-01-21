#include <spdlog/spdlog.h>
#include <chrono>

#include "tomtom/sdk/services/gps_service.hpp"
#include "tomtom/sdk/utils/http_client.hpp"
#include "tomtom/sdk/utils/files_identifier.hpp"
#include "tomtom/sdk/utils/string_utils.hpp"

namespace tomtom::sdk::services
{
    GpsService::GpsService(std::shared_ptr<core::Watch> watch,
                           std::shared_ptr<PreferencesService> prefsService)
        : watch_(std::move(watch)), prefsService_(std::move(prefsService))
    {
    }

    bool GpsService::isUpdateNeeded() const
    {
        try
        {
            auto prefs = prefsService_->get();
            auto now = std::chrono::system_clock::now();
            auto diff = now - prefs.ephemeris_modified;

            // If data is older than 3 days (72 hours), update is needed
            bool needed = diff > std::chrono::hours(72);

            if (needed)
            {
                spdlog::info("GPS QuickFix update needed (data is {} hours old)",
                             std::chrono::duration_cast<std::chrono::hours>(diff).count());
            }
            else
            {
                spdlog::debug("GPS QuickFix is up to date");
            }

            return needed;
        }
        catch (const std::exception &e)
        {
            // If we can't read dates, assume update is needed
            spdlog::warn("Unable to check QuickFix status: {}", e.what());
            return true;
        }
    }

    void GpsService::updateQuickFix(ProgressCallback progress)
    {
        spdlog::info("Starting GPS QuickFix update");

        if (progress)
            progress(0.1f, "Reading preferences...");

        // 1. Get URL from preferences
        auto prefs = prefsService_->get();
        if (prefs.ephemeris_url.empty())
        {
            // Fallback URL if preference is missing
            prefs.ephemeris_url = "http://home.tomtom.com/download/ephemeris.7days.bin";
            spdlog::warn("Ephemeris URL missing, using default: {}", prefs.ephemeris_url);
        }

        // 2. Expand URL (request 3 or 7 days)
        std::string url = expandUrl(prefs.ephemeris_url, 7);
        spdlog::debug("Downloading QuickFix from: {}", url);

        if (progress)
            progress(0.2f, "Downloading satellite data...");

        // 3. Download
        auto result = utils::downloadFile(url, [&progress](size_t downloaded, size_t total)
                                          {
            if (progress && total > 0) {
                // Scale download progress from 0.2 to 0.5
                float downloadProgress = 0.2f + (0.3f * static_cast<float>(downloaded) / total);
                progress(downloadProgress, "Downloading satellite data...");
            } });

        if (!result.success())
        {
            std::string errorMsg = "Failed to download QuickFix data: " + result.error_message;
            spdlog::error("{}", errorMsg);
            throw std::runtime_error(errorMsg);
        }

        spdlog::info("Downloaded QuickFix data: {} bytes", result.data.size());

        if (progress)
            progress(0.5f, "Uploading to watch...");

        // 4. Write to Watch (File ID 0x00010100)
        try
        {
            watch_->files().writeFile(utils::GPS_QUICKFIX.value, result.data);
            spdlog::info("Successfully uploaded QuickFix data to watch");
        }
        catch (const std::exception &e)
        {
            std::string errorMsg = "Failed to upload QuickFix data to watch: " + std::string(e.what());
            spdlog::error("{}", errorMsg);
            throw std::runtime_error(errorMsg);
        }

        // 5. Update Timestamp in Preferences
        if (progress)
            progress(0.9f, "Updating timestamps...");

        prefs.ephemeris_modified = std::chrono::system_clock::now();
        prefsService_->set(prefs);

        spdlog::info("GPS QuickFix update completed successfully");

        if (progress)
            progress(1.0f, "Done");
    }

    std::string GpsService::expandUrl(std::string url, int days) const
    {
        // TomTom URLs often look like: .../ephemeris.{DAYS}days.bin
        return utils::replaceAll(url, "{DAYS}", std::to_string(days));
    }
}
