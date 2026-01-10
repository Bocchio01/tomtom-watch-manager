// ============================================================================
// activity.hpp - Activity domain public API
// ============================================================================
#pragma once

// Main includes for activity module
#include "activity_types.hpp"
#include "activity_models.hpp"
#include "activity_parser.hpp"
#include "activity_service.hpp"

// Record types
#include "records/activity_record.hpp"
#include "records/gps_record.hpp"
#include "records/heart_rate_record.hpp"
#include "records/summary_record.hpp"
#include "records/status_record.hpp"
#include "records/lap_record.hpp"
#include "records/altitude_record.hpp"
#include "records/cycling_cadence_record.hpp"
#include "records/race_result_record.hpp"
#include "records/swimming_record.hpp"
#include "records/treadmill_record.hpp"
#include "records/gym_record.hpp"
#include "records/fitness_point_record.hpp"
#include "records/pool_size_record.hpp"
#include "records/wheel_size_record.hpp"
#include "records/goal_progress_record.hpp"
#include "records/training_setup_record.hpp"
#include "records/interval_setup_record.hpp"
#include "records/interval_start_record.hpp"
#include "records/interval_finish_record.hpp"
#include "records/race_setup_record.hpp"
#include "records/heart_rate_recovery_record.hpp"
#include "records/indoor_cycling_record.hpp"

/**
 * @brief Activity domain namespace
 *
 * Contains all functionality related to workout activities:
 * - Activity types and enumerations
 * - Activity data models and records
 * - Binary file parsing (.ttbin format)
 * - High-level service for activity management
 * - Export functionality (GPX, TCX, CSV)
 *
 * Usage example:
 * @code
 * using namespace tomtom::services::activity;
 *
 * ActivityService service(file_service);
 * auto activities = service.list();
 *
 * for (const auto& info : activities) {
 *     std::cout << toString(info.type) << ": "
 *               << info.getDistanceKm() << " km\n";
 * }
 *
 * auto activity = service.get(0);
 * std::string gpx = service.exportToGPX(activity);
 * @endcode
 */
namespace tomtom::services::activity
{

    // Re-export commonly used types for convenience
    using Activity = models::Activity;
    using ActivityInfo = models::ActivityInfo;

    // Record types
    using records::ActivityRecord;
    using records::GPSRecord;
    using records::HeartRateRecord;
    using records::LapRecord;
    using records::StatusRecord;
    using records::SummaryRecord;

} // namespace tomtom::services::activity
