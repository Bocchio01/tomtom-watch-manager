#include "tomtom/sdk/gps_manager.hpp"
#include "tomtom/sdk/networking/http_client.hpp"
#include <spdlog/spdlog.h>
#include <sstream>
#include <stdexcept>

namespace tomtom::sdk
{

    GpsManager::GpsManager(std::shared_ptr<tomtom::Watch> watch)
        : watch_(std::move(watch))
    {
        if (!watch_)
        {
            throw std::invalid_argument("Watch cannot be null");
        }
    }

    void GpsManager::updateQuickFix(
        const std::string &url,
        int days,
        ProgressCallback progress)
    {
        // Validate days parameter
        if (days != 3 && days != 7)
        {
            throw std::invalid_argument("Days must be 3 or 7");
        }

        spdlog::info("Updating GPS QuickFix from URL ({} days)", days);

        // Build URL by replacing {DAYS} placeholder
        std::string final_url = url;
        size_t pos = final_url.find("{DAYS}");
        if (pos != std::string::npos)
        {
            final_url.replace(pos, 6, std::to_string(days));
        }

        spdlog::debug("Ephemeris URL: {}", final_url);

        // Download ephemeris data from TomTom servers
        spdlog::info("Downloading GPS QuickFix data...");
        auto result = networking::downloadFile(final_url, progress);

        if (!result.success())
        {
            throw std::runtime_error("Failed to download ephemeris: " + result.error_message);
        }

        if (result.data.empty())
        {
            throw std::runtime_error("Downloaded ephemeris data is empty");
        }

        spdlog::info("Downloaded {} bytes of ephemeris data", result.data.size());

        // Write ephemeris data to watch
        watch_->gpsQuickFix().updateEphemeris(result.data);

        spdlog::info("GPS QuickFix update completed successfully");
    }

    bool GpsManager::isEphemerisExpired()
    {
        // TODO: Implement by reading ephemeris timestamp from watch
        // For now, return false (not implemented)
        spdlog::warn("isEphemerisExpired() not yet implemented");
        return false;
    }

} // namespace tomtom::sdk
