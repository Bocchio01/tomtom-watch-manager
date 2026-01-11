#pragma once

#include <map>
#include <cstdint>
#include <vector>
#include <memory>
#include <stdexcept>

#include "tomtom/utils/binary_reader.hpp"

#include "activity_models.hpp"

namespace tomtom::services::activity
{

    /**
     * @brief Exception thrown when activity file parsing fails
     */
    class ActivityParseError : public std::runtime_error
    {
    public:
        explicit ActivityParseError(const std::string &message)
            : std::runtime_error("Activity parse error: " + message) {}
    };

    /**
     * @brief Parser for TomTom .ttbin activity files
     *
     * Parses binary activity files (0x0091nnnn) into structured data.
     * See PDF specification pages 9-30 for format details.
     */
    class ActivityParser
    {
    public:
        /**
         * @brief Parse a complete activity file
         * @param data Raw binary data from activity file
         * @return Parsed activity structure
         * @throws ActivityParseError if file is malformed
         */
        models::Activity parse(const std::vector<uint8_t> &data);

    private:
        // Parse individual sections
        void parseHeader(models::Activity &activity, utils::BinaryReader &reader);
        std::unique_ptr<records::ActivityRecord> parseRecord(std::map<RecordTag, uint16_t> &record_lengths, utils::BinaryReader &reader);

        // Validation
        void validateHeader(const models::Activity &activity);
    };

}
