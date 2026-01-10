#pragma once

#include "tomtom/services/file_service.hpp"
#include "tomtom/services/watch_control_service.hpp"

#include <memory>
#include <vector>

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
            std::shared_ptr<services::FileService> file_service,
            std::shared_ptr<services::WatchControlService> control_service);

        // ====================================================================
        // Update Operations
        // ====================================================================

        /**
         * @brief Update GPS QuickFix data from raw ephemeris data
         * @param data Ephemeris data bytes
         * @param reset_gps Whether to reset GPS processor after update
         * @throws std::runtime_error if update fails
         */
        void updateEphemeris(
            const std::vector<uint8_t> &data,
            bool reset_gps = true);

    private:
        std::shared_ptr<services::FileService> file_service_;
        std::shared_ptr<services::WatchControlService> control_service_;
    };

} // namespace tomtom::services::gps_quickfix
