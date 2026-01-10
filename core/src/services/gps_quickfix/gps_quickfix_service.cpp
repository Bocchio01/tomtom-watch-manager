
#include "tomtom/services/gps_quickfix/gps_quickfix_service.hpp"
#include "tomtom/services/file_ids.hpp"
#include <spdlog/spdlog.h>
#include <stdexcept>

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

    void GpsQuickFixService::updateEphemeris(
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

} // namespace tomtom::services::gps_quickfix
