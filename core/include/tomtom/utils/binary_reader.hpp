#pragma once

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <cstring>

namespace tomtom::utils
{

    /**
     * @brief Helper class for reading binary data with automatic endianness handling
     *
     * TomTom watches use little-endian format for all binary data.
     */
    class BinaryReader
    {
    public:
        /**
         * @brief Construct a binary reader from data vector
         * @param data The binary data to read
         */
        explicit BinaryReader(const std::vector<uint8_t> &data)
            : data_(data), position_(0) {}

        /**
         * @brief Construct a binary reader from data pointer and size
         * @param data Pointer to binary data
         * @param size Size of data in bytes
         */
        BinaryReader(const uint8_t *data, size_t size)
            : data_(data, data + size), position_(0) {}

        // ====================================================================
        // Position Management
        // ====================================================================

        /**
         * @brief Get current read position
         */
        size_t position() const { return position_; }

        /**
         * @brief Set read position
         * @throws std::out_of_range if position is beyond data size
         */
        void seek(size_t pos)
        {
            if (pos > data_.size())
            {
                throw std::out_of_range("Seek position beyond data size");
            }
            position_ = pos;
        }

        /**
         * @brief Skip bytes forward
         * @param count Number of bytes to skip
         */
        void skip(size_t count)
        {
            seek(position_ + count);
        }

        /**
         * @brief Get number of bytes remaining
         */
        size_t remaining() const
        {
            return data_.size() - position_;
        }

        /**
         * @brief Check if we've reached end of data
         */
        bool eof() const
        {
            return position_ >= data_.size();
        }

        /**
         * @brief Get total size of data
         */
        size_t size() const
        {
            return data_.size();
        }

        // ====================================================================
        // Reading Primitives (Little-Endian)
        // ====================================================================

        /**
         * @brief Read a single byte
         */
        uint8_t readU8()
        {
            checkAvailable(1);
            return data_[position_++];
        }

        /**
         * @brief Read a signed byte
         */
        int8_t readI8()
        {
            return static_cast<int8_t>(readU8());
        }

        /**
         * @brief Read an unsigned 16-bit integer (little-endian)
         */
        uint16_t readU16()
        {
            checkAvailable(2);
            uint16_t value = data_[position_] | (data_[position_ + 1] << 8);
            position_ += 2;
            return value;
        }

        /**
         * @brief Read a signed 16-bit integer (little-endian)
         */
        int16_t readI16()
        {
            return static_cast<int16_t>(readU16());
        }

        /**
         * @brief Read an unsigned 32-bit integer (little-endian)
         */
        uint32_t readU32()
        {
            checkAvailable(4);
            uint32_t value = data_[position_] |
                             (data_[position_ + 1] << 8) |
                             (data_[position_ + 2] << 16) |
                             (data_[position_ + 3] << 24);
            position_ += 4;
            return value;
        }

        /**
         * @brief Read a signed 32-bit integer (little-endian)
         */
        int32_t readI32()
        {
            return static_cast<int32_t>(readU32());
        }

        /**
         * @brief Read an unsigned 64-bit integer (little-endian)
         */
        uint64_t readU64()
        {
            checkAvailable(8);
            uint64_t value = static_cast<uint64_t>(data_[position_]) |
                             (static_cast<uint64_t>(data_[position_ + 1]) << 8) |
                             (static_cast<uint64_t>(data_[position_ + 2]) << 16) |
                             (static_cast<uint64_t>(data_[position_ + 3]) << 24) |
                             (static_cast<uint64_t>(data_[position_ + 4]) << 32) |
                             (static_cast<uint64_t>(data_[position_ + 5]) << 40) |
                             (static_cast<uint64_t>(data_[position_ + 6]) << 48) |
                             (static_cast<uint64_t>(data_[position_ + 7]) << 56);
            position_ += 8;
            return value;
        }

        /**
         * @brief Read a signed 64-bit integer (little-endian)
         */
        int64_t readI64()
        {
            return static_cast<int64_t>(readU64());
        }

        /**
         * @brief Read a 32-bit float (little-endian)
         */
        float readFloat()
        {
            uint32_t bits = readU32();
            float value;
            std::memcpy(&value, &bits, sizeof(float));
            return value;
        }

        /**
         * @brief Read a 64-bit double (little-endian)
         */
        double readDouble()
        {
            uint64_t bits = readU64();
            double value;
            std::memcpy(&value, &bits, sizeof(double));
            return value;
        }

        // ====================================================================
        // Reading Arrays and Buffers
        // ====================================================================

        /**
         * @brief Read raw bytes into a buffer
         * @param buffer Destination buffer
         * @param count Number of bytes to read
         */
        void readBytes(uint8_t *buffer, size_t count)
        {
            checkAvailable(count);
            std::memcpy(buffer, data_.data() + position_, count);
            position_ += count;
        }

        /**
         * @brief Read raw bytes into a vector
         * @param count Number of bytes to read
         * @return Vector containing the bytes
         */
        std::vector<uint8_t> readBytes(size_t count)
        {
            checkAvailable(count);
            std::vector<uint8_t> result(data_.begin() + position_,
                                        data_.begin() + position_ + count);
            position_ += count;
            return result;
        }

        /**
         * @brief Read null-terminated string
         * @param maxLength Maximum length to read (0 = unlimited)
         * @return The string (without null terminator)
         */
        std::string readCString(size_t maxLength = 0)
        {
            std::string result;
            size_t count = 0;

            while (!eof() && (maxLength == 0 || count < maxLength))
            {
                uint8_t ch = readU8();
                if (ch == 0)
                    break;
                result += static_cast<char>(ch);
                count++;
            }

            return result;
        }

        /**
         * @brief Read fixed-length string (may contain nulls)
         * @param length Number of characters to read
         * @return The string
         */
        std::string readString(size_t length)
        {
            checkAvailable(length);
            std::string result(reinterpret_cast<const char *>(data_.data() + position_), length);
            position_ += length;
            return result;
        }

        /**
         * @brief Peek at next byte without advancing position
         */
        uint8_t peek() const
        {
            if (position_ >= data_.size())
            {
                throw std::out_of_range("Peek beyond end of data");
            }
            return data_[position_];
        }

        /**
         * @brief Peek at byte at specific offset from current position
         */
        uint8_t peekAt(size_t offset) const
        {
            if (position_ + offset >= data_.size())
            {
                throw std::out_of_range("Peek beyond end of data");
            }
            return data_[position_ + offset];
        }

        /**
         * @brief Get pointer to current position in data
         */
        const uint8_t *currentPtr() const
        {
            return data_.data() + position_;
        }

    private:
        std::vector<uint8_t> data_;
        size_t position_;

        void checkAvailable(size_t count) const
        {
            if (position_ + count > data_.size())
            {
                throw std::out_of_range("Not enough data available to read");
            }
        }
    };

}
