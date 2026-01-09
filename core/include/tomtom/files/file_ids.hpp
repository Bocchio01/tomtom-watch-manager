// ============================================================================
// file_ids.hpp - Central definition of all file IDs
// ============================================================================
#pragma once

#include "tomtom/protocol/definition/payload/common.hpp"

namespace tomtom::files
{

    using FileId = protocol::definition::FileId;

    // ========================================================================
    // System Files
    // ========================================================================
    constexpr FileId PREFERENCES = FileId(0x00F20000);
    constexpr FileId SYSTEM_FIRMWARE = FileId(0x000000F0);
    constexpr FileId BLE_FIRMWARE = FileId(0x00000012);
    constexpr FileId GPS_QUICKFIX = FileId(0x00010100);
    constexpr FileId GPS_FIRMWARE = FileId(0x00010200);
    constexpr FileId SYSTEM_LOG = FileId(0x00013100);

    // ========================================================================
    // File ID Ranges
    // ========================================================================
    namespace ranges
    {
        constexpr uint32_t ACTIVITY_BASE = 0x00910000;
        constexpr uint32_t ACTIVITY_SUMMARY_BASE = 0x00830000;
        constexpr uint32_t MANIFEST_BASE = 0x00850000;
        constexpr uint32_t TRACKING_UPLOAD_BASE = 0x00B10000;
        constexpr uint32_t TRACKING_TEMP = 0x00B20000;
        constexpr uint32_t TRACKING_WEEKLY_BASE = 0x00B30000;
        constexpr uint32_t ROUTE_BASE = 0x00B80000;
        constexpr uint32_t WORKOUT_BASE = 0x00BE0000;
        constexpr uint32_t RACE_BASE = 0x00710000;
    }

    // ========================================================================
    // File ID Construction Helpers
    // ========================================================================

    /**
     * @brief Construct activity file ID from index
     * @param index Activity index (0-65535)
     * @return File ID for activity
     */
    inline constexpr FileId activityFile(uint16_t index)
    {
        return FileId(ranges::ACTIVITY_BASE | index);
    }

    /**
     * @brief Construct activity summary file ID from index
     * @param index Activity index (0-65535)
     * @return File ID for activity summary
     */
    inline constexpr FileId activitySummaryFile(uint16_t index)
    {
        return FileId(ranges::ACTIVITY_SUMMARY_BASE | index);
    }

    /**
     * @brief Construct manifest file ID from index
     * @param index Manifest index (0-255)
     * @return File ID for manifest
     */
    inline constexpr FileId manifestFile(uint8_t index)
    {
        return FileId(ranges::MANIFEST_BASE | index);
    }

    /**
     * @brief Construct route file ID from index
     * @param index Route index (0-255)
     * @return File ID for route
     */
    inline constexpr FileId routeFile(uint8_t index)
    {
        return FileId(ranges::ROUTE_BASE | index);
    }

    /**
     * @brief Construct workout file ID from index
     * @param index Workout index (0-255)
     * @return File ID for workout
     */
    inline constexpr FileId workoutFile(uint8_t index)
    {
        return FileId(ranges::WORKOUT_BASE | index);
    }

    /**
     * @brief Construct tracking upload file ID from index
     * @param index Tracking index (0-65535)
     * @return File ID for tracking upload
     */
    inline constexpr FileId trackingUploadFile(uint16_t index)
    {
        return FileId(ranges::TRACKING_UPLOAD_BASE | index);
    }

    /**
     * @brief Construct weekly tracking file ID from index
     * @param index Week index (0-255)
     * @return File ID for weekly tracking
     */
    inline constexpr FileId trackingWeeklyFile(uint8_t index)
    {
        return FileId(ranges::TRACKING_WEEKLY_BASE | index);
    }

    /**
     * @brief Construct race file ID from index
     * @param index Race index (0-65535)
     * @return File ID for race
     */
    inline constexpr FileId raceFile(uint16_t index)
    {
        return FileId(ranges::RACE_BASE | index);
    }

    // ========================================================================
    // File ID Extraction Helpers
    // ========================================================================

    /**
     * @brief Extract activity index from file ID
     * @param id File ID
     * @return Activity index (0-65535)
     */
    inline constexpr uint16_t getActivityIndex(FileId id)
    {
        return static_cast<uint16_t>(id.value & 0xFFFF);
    }

    /**
     * @brief Extract manifest index from file ID
     * @param id File ID
     * @return Manifest index (0-255)
     */
    inline constexpr uint8_t getManifestIndex(FileId id)
    {
        return static_cast<uint8_t>(id.value & 0xFF);
    }

    /**
     * @brief Extract route index from file ID
     * @param id File ID
     * @return Route index (0-255)
     */
    inline constexpr uint8_t getRouteIndex(FileId id)
    {
        return static_cast<uint8_t>(id.value & 0xFF);
    }

    /**
     * @brief Extract workout index from file ID
     * @param id File ID
     * @return Workout index (0-255)
     */
    inline constexpr uint8_t getWorkoutIndex(FileId id)
    {
        return static_cast<uint8_t>(id.value & 0xFF);
    }

    /**
     * @brief Extract tracking index from file ID
     * @param id File ID
     * @return Tracking index (0-65535)
     */
    inline constexpr uint16_t getTrackingIndex(FileId id)
    {
        return static_cast<uint16_t>(id.value & 0xFFFF);
    }

    // ========================================================================
    // File ID Type Detection
    // ========================================================================

    /**
     * @brief Check if file ID is an activity file
     */
    inline constexpr bool isActivityFile(FileId id)
    {
        return (id.value & 0xFFFF0000) == ranges::ACTIVITY_BASE;
    }

    /**
     * @brief Check if file ID is an activity summary file
     */
    inline constexpr bool isActivitySummaryFile(FileId id)
    {
        return (id.value & 0xFFFF0000) == ranges::ACTIVITY_SUMMARY_BASE;
    }

    /**
     * @brief Check if file ID is a manifest file
     */
    inline constexpr bool isManifestFile(FileId id)
    {
        return (id.value & 0xFFFFFF00) == ranges::MANIFEST_BASE;
    }

    /**
     * @brief Check if file ID is a route file
     */
    inline constexpr bool isRouteFile(FileId id)
    {
        return (id.value & 0xFFFFFF00) == ranges::ROUTE_BASE;
    }

    /**
     * @brief Check if file ID is a workout file
     */
    inline constexpr bool isWorkoutFile(FileId id)
    {
        return (id.value & 0xFFFFFF00) == ranges::WORKOUT_BASE;
    }

    /**
     * @brief Check if file ID is a tracking file
     */
    inline constexpr bool isTrackingFile(FileId id)
    {
        return (id.value & 0xFFFF0000) == ranges::TRACKING_UPLOAD_BASE ||
               (id.value & 0xFFFFFF00) == ranges::TRACKING_WEEKLY_BASE ||
               id.value == ranges::TRACKING_TEMP;
    }

} // namespace tomtom::files
