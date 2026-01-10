// ============================================================================
// activity_parser.cpp - TomTom .ttbin activity file parser implementation
// ============================================================================

#include "tomtom/services/activity/activity_parser.hpp"
#include <spdlog/spdlog.h>

namespace tomtom::services::activity
{

    models::Activity ActivityParser::parse(const std::vector<uint8_t> &data)
    {
        if (data.empty())
        {
            throw ActivityParseError("Empty activity file");
        }

        common::BinaryReader reader(data);
        models::Activity activity;

        // Parse header
        parseHeader(activity, reader);
        validateHeader(activity);

        // Parse all records
        while (!reader.eof())
        {
            try
            {
                auto record = parseRecord(reader);
                if (record)
                {
                    // Extract summary information from Summary record
                    if (record->tag == RecordTag::Summary)
                    {
                        auto *summary = static_cast<records::SummaryRecord *>(record.get());
                        activity.type = static_cast<ActivityType>(summary->activity_type);
                        activity.duration_seconds = summary->duration_seconds;
                        activity.distance_meters = summary->distance_meters;
                        activity.calories = summary->calories;

                        if (summary->heart_rate_avg > 0)
                        {
                            activity.heart_rate_avg = summary->heart_rate_avg;
                            activity.heart_rate_max = summary->heart_rate_max;
                            activity.heart_rate_min = summary->heart_rate_min;
                        }

                        if (summary->ascent > 0 || summary->descent > 0)
                        {
                            activity.ascent = summary->ascent;
                            activity.descent = summary->descent;
                        }
                    }

                    activity.records.push_back(std::move(record));
                }
            }
            catch (const std::exception &e)
            {
                spdlog::warn("Failed to parse record at position {}: {}",
                             reader.position(), e.what());
                break;
            }
        }

        spdlog::info("Parsed activity with {} records", activity.records.size());
        return activity;
    }

    void ActivityParser::parseHeader(models::Activity &activity, common::BinaryReader &reader)
    {
        if (reader.remaining() < 24)
        {
            throw ActivityParseError("File too small for header");
        }

        // File format version (2 bytes)
        activity.format_version = reader.readU16();

        // Firmware version (6 bytes)
        activity.firmware_version[0] = reader.readU8();
        activity.firmware_version[1] = reader.readU8();
        activity.firmware_version[2] = reader.readU8();
        activity.firmware_version[3] = reader.readU8();
        activity.firmware_version[4] = reader.readU8();
        activity.firmware_version[5] = reader.readU8();

        // Product ID (2 bytes)
        activity.product_id = reader.readU16();

        // Start time (4 bytes, Unix timestamp)
        activity.start_time = static_cast<std::time_t>(reader.readU32());

        // Software version (16 bytes) - skip
        reader.skip(16);

        // GPS firmware version (80 bytes) - skip
        reader.skip(80);

        // Watch time (4 bytes, Unix timestamp)
        activity.watch_time = static_cast<std::time_t>(reader.readU32());

        // Local time offset (4 bytes, seconds from UTC)
        activity.local_time_offset = reader.readU32();

        // Reserved (1 byte) - skip
        reader.skip(1);

        // Length records (1 byte) - skip
        uint8_t length_count = reader.readU8();

        // reader.skip(length_count * sizeof(uint16_t) + length_count * sizeof(uint8_t));
        for (uint8_t i = 0; i < length_count; ++i)
        {
            spdlog::debug("Skipping length record {}: tag=0x{:02X}, length={}",
                          i, reader.readU8(), reader.readU16());
        }

        // Initialize summary fields (will be filled from Summary record)
        activity.type = ActivityType::Unknown;
        activity.duration_seconds = 0;
        activity.distance_meters = 0.0f;
        activity.calories = 0;
    }

    void ActivityParser::validateHeader(const models::Activity &activity)
    {
        if (activity.format_version == 0 || activity.format_version > 10)
        {
            spdlog::warn("Unusual format version: {}", activity.format_version);
        }

        if (activity.product_id == 0)
        {
            spdlog::warn("Product ID is zero");
        }

        // Check if timestamps are reasonable (after 2010, before 2100)
        constexpr std::time_t min_time = 1262304000; // 2010-01-01
        constexpr std::time_t max_time = 4102444800; // 2100-01-01

        if (activity.start_time < min_time || activity.start_time > max_time)
        {
            spdlog::warn("Start time looks invalid: {}", activity.start_time);
        }
    }

    std::unique_ptr<records::ActivityRecord> ActivityParser::parseRecord(common::BinaryReader &reader)
    {
        if (reader.eof())
        {
            return nullptr;
        }

        // Read record tag
        RecordTag tag = static_cast<RecordTag>(reader.readU8());
        const uint8_t *dataPtr = reader.currentPtr();

        // Parse based on tag
        switch (tag)
        {
        case RecordTag::GPS:
            reader.skip(sizeof(records::GPSRecordData));
            return records::GPSRecord::fromBinary(dataPtr);

        case RecordTag::HeartRate:
            reader.skip(sizeof(records::HeartRateRecordData));
            return records::HeartRateRecord::fromBinary(dataPtr);

        case RecordTag::Summary:
            reader.skip(sizeof(records::SummaryRecordData));
            return records::SummaryRecord::fromBinary(dataPtr);

        case RecordTag::Status:
            reader.skip(sizeof(records::StatusRecordData));
            return records::StatusRecord::fromBinary(dataPtr);

        case RecordTag::Lap:
            reader.skip(sizeof(records::LapRecordData));
            return records::LapRecord::fromBinary(dataPtr);

        case RecordTag::AltitudeUpdate:
            reader.skip(sizeof(records::AltitudeRecordData));
            return records::AltitudeRecord::fromBinary(dataPtr);

        case RecordTag::CyclingCadence:
            reader.skip(sizeof(records::CyclingCadenceRecordData));
            return records::CyclingCadenceRecord::fromBinary(dataPtr);

        case RecordTag::RaceResult:
            reader.skip(sizeof(records::RaceResultRecordData));
            return records::RaceResultRecord::fromBinary(dataPtr);

        case RecordTag::Swim:
            reader.skip(sizeof(records::SwimmingRecordData));
            return records::SwimmingRecord::fromBinary(dataPtr);

        case RecordTag::Treadmill:
            reader.skip(sizeof(records::TreadmillRecordData));
            return records::TreadmillRecord::fromBinary(dataPtr);

        case RecordTag::Gym:
            reader.skip(sizeof(records::GymRecordData));
            return records::GymRecord::fromBinary(dataPtr);

        case RecordTag::FitnessPoint:
            reader.skip(sizeof(records::FitnessPointRecordData));
            return records::FitnessPointRecord::fromBinary(dataPtr);

        case RecordTag::PoolSize:
            reader.skip(sizeof(records::PoolSizeRecordData));
            return records::PoolSizeRecord::fromBinary(dataPtr);

        case RecordTag::WheelSize:
            reader.skip(sizeof(records::WheelSizeRecordData));
            return records::WheelSizeRecord::fromBinary(dataPtr);

        case RecordTag::GoalProgress:
            reader.skip(sizeof(records::GoalProgressRecordData));
            return records::GoalProgressRecord::fromBinary(dataPtr);

        case RecordTag::TrainingSetup:
            reader.skip(sizeof(records::TrainingSetupRecordData));
            return records::TrainingSetupRecord::fromBinary(dataPtr);

        case RecordTag::IntervalSetup:
            reader.skip(sizeof(records::IntervalSetupRecordData));
            return records::IntervalSetupRecord::fromBinary(dataPtr);

        case RecordTag::IntervalStart:
            reader.skip(sizeof(records::IntervalStartRecordData));
            return records::IntervalStartRecord::fromBinary(dataPtr);

        case RecordTag::IntervalFinish:
            reader.skip(sizeof(records::IntervalFinishRecordData));
            return records::IntervalFinishRecord::fromBinary(dataPtr);

        case RecordTag::RaceSetup:
            reader.skip(sizeof(records::RaceSetupRecordData));
            return records::RaceSetupRecord::fromBinary(dataPtr);

        case RecordTag::HeartRateRecovery:
            reader.skip(sizeof(records::HeartRateRecoveryRecordData));
            return records::HeartRateRecoveryRecord::fromBinary(dataPtr);

        case RecordTag::IndoorCycling:
            reader.skip(sizeof(records::IndoorCyclingRecordData));
            return records::IndoorCyclingRecord::fromBinary(dataPtr);

        default:
            // Unknown record type - log and skip
            spdlog::debug("Unknown record tag: 0x{:02X} at position {}",
                          static_cast<uint8_t>(tag), reader.position() - 1);
            return nullptr;
        }
    }

} // namespace tomtom::services::activity
