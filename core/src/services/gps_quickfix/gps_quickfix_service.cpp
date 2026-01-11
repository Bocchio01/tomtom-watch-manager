#include <stdexcept>
#include <spdlog/spdlog.h>

#include "tomtom/services/files/files.hpp"
#include "tomtom/services/watch/watch_service.hpp"
#include "tomtom/services/preferences/preferences.hpp"
#include "tomtom/services/gps_quickfix/gps_quickfix_service.hpp"

namespace tomtom::services::gps_quickfix
{

    GpsQuickFixService::GpsQuickFixService(
        std::shared_ptr<services::files::FileService> file_service,
        std::shared_ptr<services::watch::WatchService> watch_service,
        std::shared_ptr<services::preferences::PreferencesService> preferences_service)
        : file_service_(std::move(file_service)),
          watch_service_(std::move(watch_service)),
          preferences_service_(std::move(preferences_service))
    {
        if (!file_service_)
        {
            throw std::invalid_argument("FileService cannot be null");
        }
        if (!watch_service_)
        {
            throw std::invalid_argument("WatchService cannot be null");
        }
        if (!preferences_service_)
        {
            throw std::invalid_argument("PreferencesService cannot be null");
        }
    }

    void GpsQuickFixService::updateEphemeris(const std::vector<uint8_t> &data)
    {
        if (data.empty())
        {
            throw std::invalid_argument("Data cannot be empty");
        }

        spdlog::info("Writing GPS QuickFix data to watch ({} bytes)...", data.size());

        try
        {
            // Write the data to the GPS QuickFix file
            // file_service_->writeFile(files::GPS_QUICKFIX, data);

            // Reset GPS processor
            std::string reset_msg = watch_service_->resetGpsProcessor();
            spdlog::debug("GPS reset response: {}", reset_msg);

            // Update ephemeris modified timestamp in XML file
            preferences_service_->set(
                [prefs = preferences_service_->get()]() mutable
                {
                    prefs.ephemeris_modified = std::chrono::system_clock::from_time_t(std::time(nullptr));
                    return prefs;
                }());
        }
        catch (const std::exception &e)
        {
            throw std::runtime_error(std::string("GPS QuickFix update failed: ") + e.what());
        }
    }

}
