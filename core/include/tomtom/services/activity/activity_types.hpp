#pragma once

#include <cstdint>
#include <string_view>

namespace tomtom::services::activity
{

    /**
     * @brief Activity type enumeration
     *
     * Defines the type of workout/activity performed.
     * Values from TomTom Watch Interface specification.
     */
    enum class ActivityType : uint8_t
    {
        Running = 0,
        Cycling = 1,
        Swimming = 2,
        Stopwatch = 6, // Doesn't actually log any data
        Treadmill = 7,
        Freestyle = 8,
        Gym = 9,
        Hiking = 10,
        IndoorCycling = 11, // Previously 0x0B
        TrailRunning = 14,
        Skiing = 15,
        Snowboarding = 16,
        Unknown = 0xFF
    };

    /**
     * @brief Record tag enumeration
     *
     * Identifies the type of record in the activity file.
     */
    enum class RecordTag : uint8_t
    {
        FileHeader = 0x20,
        Status = 0x21,
        GPS = 0x22,
        ExtendedGPS = 0x23,
        HeartRate = 0x25,
        Summary = 0x27,
        PoolSize = 0x2A,
        WheelSize = 0x2B,
        TrainingSetup = 0x2D,
        Lap = 0x2F,
        TimeToSatelliteFix = 0x30,
        CyclingCadence = 0x31,
        Treadmill = 0x32,
        Swim = 0x34,
        GoalProgress = 0x35,
        GenericRecord37 = 0x37,
        IntervalSetup = 0x39,
        IntervalStart = 0x3A,
        IntervalFinish = 0x3B,
        RaceSetup = 0x3C,
        RaceResult = 0x3D,
        AltitudeUpdate = 0x3E,
        HeartRateRecovery = 0x3F,
        IndoorCycling = 0x40,
        Gym = 0x41,
        Movement = 0x42,
        RouteDescription = 0x43,
        Elevation = 0x47,
        Battery = 0x49,
        FitnessPoint = 0x4A,
        Workout4B = 0x4B,
        Workout4C = 0x4C,
        UnknownTag = 0xFF
    };

    /**
     * @brief Activity status enumeration
     *
     * Indicates the current state of the activity.
     */
    enum class ActivityStatus : uint8_t
    {
        Ready = 0,  // Ready to start
        Active = 1, // Activity in progress
        Paused = 2, // Activity paused
        Stopped = 3 // Activity stopped/finished
    };

    /**
     * @brief Swimming stroke type
     */
    enum class SwimStroke : uint8_t
    {
        Freestyle = 0,
        Backstroke = 1,
        Breaststroke = 2,
        Butterfly = 3,
        Drill = 4,
        Mixed = 5
    };

    /**
     * @brief Interval training type
     */
    enum class IntervalType : uint8_t
    {
        Warmup = 1,
        Work = 2,
        Rest = 3,
        Cooldown = 4,
        Finished = 5
    };

    /**
     * @brief Training setup type
     */
    enum class TrainingType : uint8_t
    {
        GoalDistance = 0,
        GoalTime = 1,
        GoalCalories = 2,
        ZonesPace = 3,
        ZonesHeart = 4,
        ZonesCadence = 5,
        Race = 6,
        LapsTime = 7,
        LapsDistance = 8,
        LapsManual = 9,
        StrokeRate = 10,
        ZonesSpeed = 11,
        Intervals = 12
    };

    /**
     * @brief Interval duration type
     */
    enum class IntervalDurationType : uint8_t
    {
        Distance = 0,
        Time = 1
    };

    /**
     * @brief Heart rate recovery status
     */
    enum class RecoveryStatus : uint32_t
    {
        Poor = 1,
        Decent = 2,
        Good = 3,
        Excellent = 4
    };

    // ========================================================================
    // Conversion Functions
    // ========================================================================

    /**
     * @brief Convert ActivityType to string
     */
    constexpr std::string_view toString(ActivityType type)
    {
        switch (type)
        {
        case ActivityType::Running:
            return "Running";
        case ActivityType::Cycling:
            return "Cycling";
        case ActivityType::Swimming:
            return "Swimming";
        case ActivityType::Stopwatch:
            return "Stopwatch";
        case ActivityType::Treadmill:
            return "Treadmill";
        case ActivityType::Freestyle:
            return "Freestyle";
        case ActivityType::Gym:
            return "Gym";
        case ActivityType::Hiking:
            return "Hiking";
        case ActivityType::IndoorCycling:
            return "Indoor Cycling";
        case ActivityType::TrailRunning:
            return "Trail Running";
        case ActivityType::Skiing:
            return "Skiing";
        case ActivityType::Snowboarding:
            return "Snowboarding";
        default:
            return "Unknown";
        }
    }

    /**
     * @brief Convert RecordTag to string
     */
    constexpr std::string_view toString(RecordTag tag)
    {
        switch (tag)
        {
        case RecordTag::FileHeader:
            return "FileHeader";
        case RecordTag::Status:
            return "Status";
        case RecordTag::GPS:
            return "GPS";
        case RecordTag::HeartRate:
            return "HeartRate";
        case RecordTag::Summary:
            return "Summary";
        case RecordTag::PoolSize:
            return "PoolSize";
        case RecordTag::WheelSize:
            return "WheelSize";
        case RecordTag::TrainingSetup:
            return "TrainingSetup";
        case RecordTag::Lap:
            return "Lap";
        case RecordTag::CyclingCadence:
            return "CyclingCadence";
        case RecordTag::Treadmill:
            return "Treadmill";
        case RecordTag::Swim:
            return "Swim";
        case RecordTag::GoalProgress:
            return "GoalProgress";
        case RecordTag::IntervalSetup:
            return "IntervalSetup";
        case RecordTag::IntervalStart:
            return "IntervalStart";
        case RecordTag::IntervalFinish:
            return "IntervalFinish";
        case RecordTag::RaceSetup:
            return "RaceSetup";
        case RecordTag::RaceResult:
            return "RaceResult";
        case RecordTag::AltitudeUpdate:
            return "AltitudeUpdate";
        case RecordTag::HeartRateRecovery:
            return "HeartRateRecovery";
        case RecordTag::IndoorCycling:
            return "IndoorCycling";
        case RecordTag::Gym:
            return "Gym";
        case RecordTag::FitnessPoint:
            return "FitnessPoint";
        case RecordTag::Workout4B:
            return "Workout4B";
        default:
            return "Unknown";
        }
    }

    /**
     * @brief Convert ActivityStatus to string
     */
    constexpr std::string_view toString(ActivityStatus status)
    {
        switch (status)
        {
        case ActivityStatus::Ready:
            return "Ready";
        case ActivityStatus::Active:
            return "Active";
        case ActivityStatus::Paused:
            return "Paused";
        case ActivityStatus::Stopped:
            return "Stopped";
        default:
            return "Unknown";
        }
    }

    /**
     * @brief Convert SwimStroke to string
     */
    constexpr std::string_view toString(SwimStroke stroke)
    {
        switch (stroke)
        {
        case SwimStroke::Freestyle:
            return "Freestyle";
        case SwimStroke::Backstroke:
            return "Backstroke";
        case SwimStroke::Breaststroke:
            return "Breaststroke";
        case SwimStroke::Butterfly:
            return "Butterfly";
        case SwimStroke::Drill:
            return "Drill";
        case SwimStroke::Mixed:
            return "Mixed";
        default:
            return "Unknown";
        }
    }

    /**
     * @brief Convert IntervalType to string
     */
    constexpr std::string_view toString(IntervalType type)
    {
        switch (type)
        {
        case IntervalType::Warmup:
            return "Warmup";
        case IntervalType::Work:
            return "Work";
        case IntervalType::Rest:
            return "Rest";
        case IntervalType::Cooldown:
            return "Cooldown";
        case IntervalType::Finished:
            return "Finished";
        default:
            return "Unknown";
        }
    }

}
