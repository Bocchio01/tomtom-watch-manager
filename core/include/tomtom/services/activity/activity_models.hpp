// ============================================================================
// activity_models.hpp - Activity data structures
// ============================================================================
#pragma once

#include "activity_types.hpp"
#include "records/gps_record.hpp"
#include "records/status_record.hpp"
#include "records/treadmill_record.hpp"
#include "records/swimming_record.hpp"
#include "records/gym_record.hpp"
#include "records/fitness_point_record.hpp"
#include "records/lap_record.hpp"
#include "records/heart_rate_record.hpp"
#include "records/race_setup_record.hpp"
#include "records/race_result_record.hpp"
#include "records/training_setup_record.hpp"
#include "records/goal_progress_record.hpp"
#include "records/interval_setup_record.hpp"
#include "records/interval_start_record.hpp"
#include "records/interval_finish_record.hpp"
#include "records/heart_rate_recovery_record.hpp"
#include "records/altitude_record.hpp"
#include "records/pool_size_record.hpp"
#include "records/wheel_size_record.hpp"
#include "records/cycling_cadence_record.hpp"
#include "records/indoor_cycling_record.hpp"

#include "records/activity_record.hpp"
#include "records/summary_record.hpp"
#include "../file_ids.hpp"

#include <vector>
#include <memory>
#include <ctime>
#include <optional>
#include <cstdio>
#include <string>

namespace tomtom::services::activity::models
{

    /**
     * @brief Complete activity/workout session data
     *
     * Represents a parsed TomTom .ttbin activity file with all
     * header information, summary, and detailed records.
     */
    struct Activity
    {
        // ====================================================================
        // Header Information
        // ====================================================================
        uint8_t format_version;      // File format version
        uint8_t firmware_version[6]; // Firmware version [major, minor, patch]
        uint16_t product_id;         // Watch product ID
        std::time_t start_time;      // Activity start time (UTC)
        std::time_t watch_time;      // Watch's current time when saved
        int32_t local_time_offset;   // Offset from UTC in seconds

        // ====================================================================
        // Summary Information (from Summary record if present)
        // ====================================================================
        ActivityType type;
        uint32_t duration_seconds;
        float distance_meters;
        uint16_t calories;

        std::optional<uint16_t> heart_rate_avg;
        std::optional<uint16_t> heart_rate_max;
        std::optional<uint16_t> heart_rate_min;
        std::optional<uint16_t> ascent;
        std::optional<uint16_t> descent;

        // ====================================================================
        // All Records from Activity File
        // ====================================================================
        std::vector<std::unique_ptr<records::ActivityRecord>> records;

        // ====================================================================
        // Convenience Methods
        // ====================================================================

        /**
         * @brief Get all GPS records
         */
        std::vector<records::GPSRecord *> getGPSRecords() const
        {
            std::vector<records::GPSRecord *> result;
            for (const auto &record : records)
            {
                if (record->tag == RecordTag::GPS)
                {
                    result.push_back(static_cast<records::GPSRecord *>(record.get()));
                }
            }
            return result;
        }

        /**
         * @brief Get all heart rate records
         */
        std::vector<records::HeartRateRecord *> getHeartRateRecords() const
        {
            std::vector<records::HeartRateRecord *> result;
            for (const auto &record : records)
            {
                if (record->tag == RecordTag::HeartRate)
                {
                    result.push_back(static_cast<records::HeartRateRecord *>(record.get()));
                }
            }
            return result;
        }

        /**
         * @brief Get summary record if present
         */
        records::SummaryRecord *getSummaryRecord() const
        {
            for (const auto &record : records)
            {
                if (record->tag == RecordTag::Summary)
                {
                    return static_cast<records::SummaryRecord *>(record.get());
                }
            }
            return nullptr;
        }

        /**
         * @brief Get all lap records
         */
        std::vector<records::LapRecord *> getLaps() const
        {
            std::vector<records::LapRecord *> result;
            for (const auto &record : records)
            {
                if (record->tag == RecordTag::Lap)
                {
                    result.push_back(static_cast<records::LapRecord *>(record.get()));
                }
            }
            return result;
        }

        /**
         * @brief Check if activity has GPS data
         */
        bool hasGPS() const
        {
            return !getGPSRecords().empty();
        }

        /**
         * @brief Check if activity has heart rate data
         */
        bool hasHeartRate() const
        {
            return !getHeartRateRecords().empty();
        }

        /**
         * @brief Get total number of records
         */
        size_t recordCount() const
        {
            return records.size();
        }

        /**
         * @brief Get distance in kilometers
         */
        double getDistanceKm() const
        {
            return distance_meters / 1000.0;
        }

        /**
         * @brief Get distance in miles
         */
        double getDistanceMiles() const
        {
            return distance_meters / 1609.344;
        }

        /**
         * @brief Get duration as hours:minutes:seconds string
         */
        std::string getDurationString() const
        {
            uint32_t hours = duration_seconds / 3600;
            uint32_t minutes = (duration_seconds % 3600) / 60;
            uint32_t seconds = duration_seconds % 60;

            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%02u:%02u:%02u", hours, minutes, seconds);
            return buffer;
        }
    };

    /**
     * @brief Lightweight activity metadata for listing
     *
     * Used when displaying a list of activities without loading
     * the full activity data.
     */
    struct ActivityInfo
    {
        uint16_t index;            // Activity index (from file ID)
        FileId file_id;            // Full file ID
        ActivityType type;         // Activity type
        std::time_t start_time;    // Start time (UTC)
        uint32_t duration_seconds; // Duration in seconds
        float distance_meters;     // Distance in meters
        uint32_t file_size;        // File size in bytes

        /**
         * @brief Get distance in kilometers
         */
        double getDistanceKm() const
        {
            return distance_meters / 1000.0;
        }

        /**
         * @brief Get distance in miles
         */
        double getDistanceMiles() const
        {
            return distance_meters / 1609.344;
        }

        /**
         * @brief Get duration as hours:minutes:seconds string
         */
        std::string getDurationString() const
        {
            uint32_t hours = duration_seconds / 3600;
            uint32_t minutes = (duration_seconds % 3600) / 60;
            uint32_t seconds = duration_seconds % 60;

            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%02u:%02u:%02u", hours, minutes, seconds);
            return buffer;
        }
    };

} // namespace tomtom::services::activity::models
