#pragma once

#include <memory>
#include <vector>

#include "tomtom/services/files/files.hpp"
#include "tomtom/services/watch/watch_service.hpp"
#include "tomtom/services/preferences/preferences.hpp"

namespace tomtom::services::gps_quickfix
{

    /**
     * @brief Service for updating GPS QuickFix (ephemeris) data
     *
     * Handles downloading and uploading GPS ephemeris data to improve
     * GPS fix acquisition time. The data is downloaded from TomTom's
     * ephemeris service and written to file ID 0x00010100.
     */
    class GpsQuickFixService
    {
    public:
        /**
         * @brief Construct GPS QuickFix service
         * @param file_service Low-level file service for watch communication
         * @param control_service Watch control service for GPS reset
         */
        explicit GpsQuickFixService(
            std::shared_ptr<services::files::FileService> file_service,
            std::shared_ptr<services::watch::WatchService> watch_service,
            std::shared_ptr<services::preferences::PreferencesService> preferences_service);

        /**
         * @brief Update GPS QuickFix data from raw ephemeris data
         * @param data Ephemeris data bytes
         * @throws std::runtime_error if update fails
         */
        void updateEphemeris(const std::vector<uint8_t> &data);

    private:
        std::shared_ptr<services::files::FileService> file_service_;
        std::shared_ptr<services::watch::WatchService> watch_service_;
        std::shared_ptr<services::preferences::PreferencesService> preferences_service_;
    };

}
