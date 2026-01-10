// ============================================================================
// gps_quickfix_service.cpp - GPS QuickFix update service implementation
// ============================================================================

#include "tomtom/services/gps_quickfix/gps_quickfix_service.hpp"
#include "tomtom/services/file_ids.hpp"
#include "tomtom/utils/http_utils.hpp"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <algorithm>

namespace tomtom::services::gps_quickfix
{

    GpsQuickFixService::GpsQuickFixService(
        std::shared_ptr<services::FileService> file_service,
        std::shared_ptr<services::WatchControlService> control_service)
        : file_service_(std::move(file_service)),
          control_service_(std::move(control_service))
    {
        if (!file_service_)
        {
            throw std::invalid_argument("FileService cannot be null");
        }
        if (!control_service_)
        {
            throw std::invalid_argument("WatchControlService cannot be null");
        }
    }

    void GpsQuickFixService::update(
        const std::string &url,
        int days,
        bool reset_gps,
        ProgressCallback progress)
    {
        if (url.empty())
        {
            throw std::invalid_argument("URL cannot be empty");
        }

        if (days != 3 && days != 7)
        {
            throw std::invalid_argument("Days must be either 3 or 7");
        }

        spdlog::info("Updating GPS QuickFix data ({} days)", days);

        // Build URL with days parameter
        std::string final_url = buildUrl(url, days);
        spdlog::debug("Ephemeris URL: {}", final_url);

        // Download the file
        spdlog::info("Downloading GPS QuickFix data...");
        auto data = downloadFile(final_url, progress);

        if (data.empty())
        {
            throw std::runtime_error("Downloaded data is empty");
        }

        spdlog::info("Downloaded {} bytes", data.size());

        // Upload to watch
        updateFromData(data, reset_gps);
    }

    void GpsQuickFixService::updateFromData(
        const std::vector<uint8_t> &data,
        bool reset_gps)
    {
        if (data.empty())
        {
            throw std::invalid_argument("Data cannot be empty");
        }

        spdlog::info("Writing GPS QuickFix data to watch ({} bytes)...", data.size());

        try
        {
            // Write the data to the GPS QuickFix file
            file_service_->writeFile(GPS_QUICKFIX, data);
            spdlog::info("GPS QuickFix data successfully written to watch");

            // Reset GPS processor if requested
            if (reset_gps)
            {
                spdlog::info("Resetting GPS processor...");
                std::string reset_msg = control_service_->resetGpsProcessor();
                spdlog::debug("GPS reset response: {}", reset_msg);
            }

            spdlog::info("GPS QuickFix update completed successfully");
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to update GPS QuickFix data: {}", e.what());
            throw std::runtime_error(std::string("GPS QuickFix update failed: ") + e.what());
        }
    }

    std::string GpsQuickFixService::buildUrl(const std::string &url_template, int days)
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

    bool GpsQuickFixService::validateUrl(const std::string &url)
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

        // Check if URL contains {DAYS} placeholder
        // if (url.find("{DAYS}") == std::string::npos)
        // {
        //     spdlog::warn("URL does not contain {{DAYS}} placeholder, will use as-is");
        // }

        return true;
    }

    std::vector<uint8_t> GpsQuickFixService::downloadFile(const std::string &url, ProgressCallback progress)
    {
        if (!validateUrl(url))
        {
            throw std::invalid_argument("Invalid URL: " + url);
        }

        auto result = utils::downloadFile(url, progress);

        if (!result.success())
        {
            throw std::runtime_error("Failed to download file: " + result.error_message);
        }

        return result.data;
    }

} // namespace tomtom::services::gps_quickfix
