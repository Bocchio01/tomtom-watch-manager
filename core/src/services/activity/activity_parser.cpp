
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

        utils::BinaryReader reader(data);
        models::Activity activity;

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
                        auto *summary = static_cast<records::SummaryRecord *>(record.get());
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

    void ActivityParser::parseHeader(models::Activity &activity, utils::BinaryReader &reader)
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

    std::unique_ptr<records::ActivityRecord> ActivityParser::parseRecord(std::map<RecordTag, uint16_t> &record_lengths, utils::BinaryReader &reader)
    {
        if (reader.eof())
        {
            return nullptr;
        }

        // Read record tag
        RecordTag tag = static_cast<RecordTag>(reader.readU8());
        const uint16_t record_length = record_lengths.count(tag) ? record_lengths[tag] : 0;
        const uint8_t *dataPtr = reader.currentPtr();

        // Parse based on tag
        switch (tag)
        {
        case RecordTag::GPS:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::GPSRecordData)));
            reader.skip(sizeof(records::GPSRecordData));
            return records::GPSRecord::fromBinary(dataPtr);

        case RecordTag::HeartRate:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::HeartRateRecordData)));
            reader.skip(sizeof(records::HeartRateRecordData));
            return records::HeartRateRecord::fromBinary(dataPtr);

        case RecordTag::Summary:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::SummaryRecordData)));
            reader.skip(sizeof(records::SummaryRecordData));
            return records::SummaryRecord::fromBinary(dataPtr);

        case RecordTag::Status:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::StatusRecordData)));
            reader.skip(sizeof(records::StatusRecordData));
            return records::StatusRecord::fromBinary(dataPtr);

        case RecordTag::Lap:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::LapRecordData)));
            reader.skip(sizeof(records::LapRecordData));
            return records::LapRecord::fromBinary(dataPtr);

        case RecordTag::AltitudeUpdate:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::AltitudeRecordData)));
            reader.skip(sizeof(records::AltitudeRecordData));
            return records::AltitudeRecord::fromBinary(dataPtr);

        case RecordTag::CyclingCadence:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::CyclingCadenceRecordData)));
            reader.skip(sizeof(records::CyclingCadenceRecordData));
            return records::CyclingCadenceRecord::fromBinary(dataPtr);

        case RecordTag::RaceResult:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::RaceResultRecordData)));
            reader.skip(sizeof(records::RaceResultRecordData));
            return records::RaceResultRecord::fromBinary(dataPtr);

        case RecordTag::Swim:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::SwimmingRecordData)));
            reader.skip(sizeof(records::SwimmingRecordData));
            return records::SwimmingRecord::fromBinary(dataPtr);

        case RecordTag::Treadmill:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::TreadmillRecordData)));
            reader.skip(sizeof(records::TreadmillRecordData));
            return records::TreadmillRecord::fromBinary(dataPtr);

        case RecordTag::Gym:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::GymRecordData)));
            reader.skip(sizeof(records::GymRecordData));
            return records::GymRecord::fromBinary(dataPtr);

        case RecordTag::FitnessPoint:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::FitnessPointRecordData)));
            reader.skip(sizeof(records::FitnessPointRecordData));
            return records::FitnessPointRecord::fromBinary(dataPtr);

        case RecordTag::PoolSize:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::PoolSizeRecordData)));
            reader.skip(sizeof(records::PoolSizeRecordData));
            return records::PoolSizeRecord::fromBinary(dataPtr);

        case RecordTag::WheelSize:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::WheelSizeRecordData)));
            reader.skip(sizeof(records::WheelSizeRecordData));
            return records::WheelSizeRecord::fromBinary(dataPtr);

        case RecordTag::GoalProgress:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::GoalProgressRecordData)));
            reader.skip(sizeof(records::GoalProgressRecordData));
            return records::GoalProgressRecord::fromBinary(dataPtr);

        case RecordTag::TrainingSetup:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::TrainingSetupRecordData)));
            reader.skip(sizeof(records::TrainingSetupRecordData));
            return records::TrainingSetupRecord::fromBinary(dataPtr);

        case RecordTag::IntervalSetup:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::IntervalSetupRecordData)));
            reader.skip(sizeof(records::IntervalSetupRecordData));
            return records::IntervalSetupRecord::fromBinary(dataPtr);

        case RecordTag::IntervalStart:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::IntervalStartRecordData)));
            reader.skip(sizeof(records::IntervalStartRecordData));
            return records::IntervalStartRecord::fromBinary(dataPtr);

        case RecordTag::IntervalFinish:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::IntervalFinishRecordData)));
            reader.skip(sizeof(records::IntervalFinishRecordData));
            return records::IntervalFinishRecord::fromBinary(dataPtr);

        case RecordTag::RaceSetup:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::RaceSetupRecordData)));
            reader.skip(sizeof(records::RaceSetupRecordData));
            return records::RaceSetupRecord::fromBinary(dataPtr);

        case RecordTag::HeartRateRecovery:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::HeartRateRecoveryRecordData)));
            reader.skip(sizeof(records::HeartRateRecoveryRecordData));
            return records::HeartRateRecoveryRecord::fromBinary(dataPtr);

        case RecordTag::IndoorCycling:
            assert(record_length == (sizeof(uint8_t) + sizeof(records::IndoorCyclingRecordData)));
            reader.skip(sizeof(records::IndoorCyclingRecordData));
            return records::IndoorCyclingRecord::fromBinary(dataPtr);

        default:
            // Unknown record type - log and skip
            spdlog::debug("Unknown record tag: 0x{:02X} at position {}",
                          static_cast<uint8_t>(tag), reader.position() - 1);

            // Look up expected length from header info
            auto it = record_lengths.find(tag);
            if (it != record_lengths.end())
            {
                uint16_t length = it->second;
                if (length == 0xFFFF)
                {
                    length = reader.readU16();
                    reader.skip(length); // In this case length includes only the remaining data
                }
                else
                {
                    reader.skip(length - 1); // -1 for already read tag byte
                }
            }
            else
            {
                throw ActivityParseError("Unknown record tag with no length info: 0x" +
                                         std::to_string(static_cast<uint8_t>(tag)));
            }

            return nullptr;
        }
    }

} // namespace tomtom::services::activity
