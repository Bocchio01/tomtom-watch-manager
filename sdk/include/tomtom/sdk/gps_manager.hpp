#pragma once

#include <tomtom/watch.hpp>
#include <functional>
#include <memory>
#include <string>

namespace tomtom::sdk
{

    /**
     * @brief GPS ephemeris (QuickFix) update manager
     *
     * Handles downloading and updating GPS QuickFix data (ephemeris) to improve
     * GPS acquisition time. Downloads from TomTom servers and writes to watch.
     */
    class GpsManager
    {
    public:
        /**
         * @brief Progress callback type
         * @param downloaded Bytes downloaded so far
         * @param total Total file size (0 if unknown)
         */
        using ProgressCallback = std::function<void(size_t downloaded, size_t total)>;

        /**
         * @brief Default TomTom ephemeris URL
         */
        static constexpr const char *DEFAULT_URL = "https://gpsquickfix.services.tomtom.com/fitness/sifgps.f2p{DAYS}enc.ee";

        /**
         * @brief Construct GPS manager
         * @param watch Connected watch instance
         */
        explicit GpsManager(std::shared_ptr<tomtom::Watch> watch);

        // ====================================================================
        // Update Operations
        // ====================================================================

        /**
         * @brief Update GPS QuickFix from URL
         * @param url Ephemeris URL (use {DAYS} placeholder)
         * @param days Number of days (3 or 7)
         * @param progress Optional progress callback
         *
         * Downloads ephemeris data from TomTom servers and writes to watch.
         */
        void updateQuickFix(
            const std::string &url = DEFAULT_URL,
            int days = 7,
            ProgressCallback progress = nullptr);

        /**
         * @brief Check if ephemeris data is expired
         * @return true if GPS data needs update (stub for now)
         *
         * TODO: Implement by reading ephemeris timestamp from watch
         */
        bool isEphemerisExpired();

    private:
        std::shared_ptr<tomtom::Watch> watch_;
    };

} // namespace tomtom::sdk
