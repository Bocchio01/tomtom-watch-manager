#include <spdlog/spdlog.h>

#include "tomtom/sdk/parsers/activity_parser.hpp"
#include "tomtom/sdk/models/activity_records.hpp"

using namespace tomtom::sdk::models;

namespace tomtom::sdk::parsers
{
    // Factory map for creating records from binary data
    static const std::unordered_map<RecordTag, std::function<std::unique_ptr<ActivityRecord>(const uint8_t *, const uint16_t)>> recordFactories = {
        {RecordTag::GPS, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<GPSRecord, GPSRecordData>(data, length); }},
        {RecordTag::HeartRate, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<HeartRateRecord, HeartRateRecordData>(data, length); }},
        {RecordTag::Summary, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<SummaryRecord, SummaryRecordData>(data, length); }},
        {RecordTag::Status, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<StatusRecord, StatusRecordData>(data, length); }},
        {RecordTag::Lap, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<LapRecord, LapRecordData>(data, length); }},
        {RecordTag::AltitudeUpdate, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<AltitudeRecord, AltitudeRecordData>(data, length); }},
        {RecordTag::CyclingCadence, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<CyclingCadenceRecord, CyclingCadenceRecordData>(data, length); }},
        {RecordTag::RaceResult, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<RaceResultRecord, RaceResultRecordData>(data, length); }},
        {RecordTag::Swim, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<SwimmingRecord, SwimmingRecordData>(data, length); }},
        {RecordTag::Treadmill, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<TreadmillRecord, TreadmillRecordData>(data, length); }},
        {RecordTag::Gym, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<GymRecord, GymRecordData>(data, length); }},
        {RecordTag::FitnessPoint, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<FitnessPointRecord, FitnessPointRecordData>(data, length); }},
        {RecordTag::PoolSize, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<PoolSizeRecord, PoolSizeRecordData>(data, length); }},
        {RecordTag::WheelSize, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<WheelSizeRecord, WheelSizeRecordData>(data, length); }},
        {RecordTag::GoalProgress, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<GoalProgressRecord, GoalProgressRecordData>(data, length); }},
        {RecordTag::TrainingSetup, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<TrainingSetupRecord, TrainingSetupRecordData>(data, length); }},
        {RecordTag::IntervalSetup, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<IntervalSetupRecord, IntervalSetupRecordData>(data, length); }},
        {RecordTag::IntervalStart, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<IntervalStartRecord, IntervalStartRecordData>(data, length); }},
        {RecordTag::IntervalFinish, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<IntervalFinishRecord, IntervalFinishRecordData>(data, length); }},
        {RecordTag::RaceSetup, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<RaceSetupRecord, RaceSetupRecordData>(data, length); }},
        {RecordTag::HeartRateRecovery, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<HeartRateRecoveryRecord, HeartRateRecoveryRecordData>(data, length); }},
        {RecordTag::IndoorCycling, [](const uint8_t *data, const uint16_t length)
         { return ActivityRecord::makeRecordFromBinary<IndoorCyclingRecord, IndoorCyclingRecordData>(data, length); }}};

    Activity ActivityParser::parse(const std::vector<uint8_t> &data)
    {
        if (data.empty())
        {
            throw ActivityParseError("Empty activity file");
        }

        utils::BinaryReader reader(data);
        Activity activity;

        // Parse header
        parseHeader(activity, reader);
        validateHeader(activity);

        // Parse all records
        while (!reader.eof())
        {
            try
            {
                auto record = parseRecord(activity.record_lengths, reader);
                if (record)
                {
                    // Extract summary information from Summary record
                    if (record->tag == RecordTag::Summary)
                    {
                        auto *summary = static_cast<SummaryRecord *>(record.get());
                        activity.type = static_cast<ActivityType>(summary->activity_type);
                        activity.duration_seconds = summary->duration_seconds;
                        activity.distance_meters = summary->distance_meters;
                        activity.calories = summary->calories;
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

    void ActivityParser::parseHeader(Activity &activity, utils::BinaryReader &reader)
    {
        if (reader.remaining() < 24)
        {
            throw ActivityParseError("File too small for header");
        }

        if (static_cast<RecordTag>(reader.readU8()) != RecordTag::FileHeader)
        {
            throw ActivityParseError("Missing file header tag");
        }

        // File format version (2 bytes)
        activity.format_version = reader.readU16();

        // Firmware version (3 or 6 bytes)
        if (activity.format_version < 10)
        {
            activity.firmware_version[0] = reader.readU8();
            activity.firmware_version[1] = reader.readU8();
            activity.firmware_version[2] = reader.readU8();
            activity.firmware_version[3] = 0;
            activity.firmware_version[4] = 0;
            activity.firmware_version[5] = 0;
        }
        else if (activity.format_version == 10)
        {
            activity.firmware_version[0] = reader.readU8();
            activity.firmware_version[1] = reader.readU8();
            activity.firmware_version[2] = reader.readU8();
            activity.firmware_version[3] = reader.readU8();
            activity.firmware_version[4] = reader.readU8();
            activity.firmware_version[5] = reader.readU8();
        }
        else
        {
            throw ActivityParseError("Unsupported format version: " +
                                     std::to_string(activity.format_version));
        }

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

        // Length records (1 byte)
        uint8_t length_count = reader.readU8();

        // Read length records
        for (uint8_t i = 0; i < length_count; ++i)
        {
            uint8_t tag = reader.readU8();
            uint16_t length = reader.readU16();
            activity.record_lengths[static_cast<RecordTag>(tag)] = length;
        }

        // Initialize summary fields (will be filled from Summary record)
        activity.type = ActivityType::Unknown;
        activity.duration_seconds = 0;
        activity.distance_meters = 0.0f;
        activity.calories = 0;
    }

    void ActivityParser::validateHeader(const Activity &activity)
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

    std::unique_ptr<ActivityRecord> ActivityParser::parseRecord(std::map<RecordTag, uint16_t> &record_lengths, utils::BinaryReader &reader)
    {

        if (reader.eof())
        {
            return nullptr;
        }

        // Read record metadata
        RecordTag tag = static_cast<RecordTag>(reader.readU8());
        const uint16_t record_length = record_lengths.count(tag) ? record_lengths[tag] : 0;
        const uint8_t *dataPtr = reader.currentPtr();

        // Look up the record factory
        auto it = recordFactories.find(tag);
        if (it != recordFactories.end())
        {
            const auto &factory = it->second;

            // Create the record using the factory
            auto record = factory(dataPtr, record_length);
            reader.skip(record_length - 1); // -1 for already read tag byte

            // Handle special case for GPS invalid record
            if (tag == RecordTag::GPS)
            {
                auto *gpsRecord = static_cast<GPSRecord *>(record.get());
                if (gpsRecord->timestamp == 0xFFFFFFFF)
                {
                    // Invalid GPS record (no fix)
                    return nullptr;
                }
            }

            return record;
        }
        else
        {
            // Look up expected length from header info
            if (record_length > 0)
            {
                if (record_length == 0xFFFF)
                {
                    uint16_t length = reader.readU16();
                    reader.skip(length); // In this case length includes only the remaining data
                }
                else
                {
                    reader.skip(record_length - 1); // -1 for already read tag byte
                }
            }
            else if (record_length == 0)
            {
                // Skip to the end of the file
                reader.seek(reader.size());
            }
            else
            {
                throw ActivityParseError("Unknown record tag with no length info: 0x" +
                                         std::to_string(static_cast<uint8_t>(tag)));
            }

            return nullptr;
        }
    }

}
