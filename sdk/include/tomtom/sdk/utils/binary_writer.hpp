#pragma once

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <string>

namespace tomtom::sdk::utils
{

    /**
     * @brief Helper class for writing binary data with automatic endianness handling
     *
     * TomTom watches use little-endian format for all binary data.
     * This writer ensures data is written in the correct byte order.
     */
    class BinaryWriter
    {
    public:
        /**
         * @brief Construct a binary writer with initial capacity
         * @param initialCapacity Initial buffer capacity (default: 1024 bytes)
         */
        explicit BinaryWriter(size_t initialCapacity = 1024)
        {
            data_.reserve(initialCapacity);
        }

        // ====================================================================
        // Position Management
        // ====================================================================

        /**
         * @brief Get current write position
         */
        size_t position() const { return data_.size(); }

        /**
         * @brief Get total size of written data
         */
        size_t size() const { return data_.size(); }

        /**
         * @brief Check if buffer is empty
         */
        bool empty() const { return data_.empty(); }

        /**
         * @brief Clear all written data
         */
        void clear()
        {
            data_.clear();
        }

        /**
         * @brief Reserve capacity in the buffer
         */
        void reserve(size_t capacity)
        {
            data_.reserve(capacity);
        }

        /**
         * @brief Get the underlying data buffer
         */
        const std::vector<uint8_t> &data() const { return data_; }

        /**
         * @brief Get pointer to the data
         */
        const uint8_t *dataPtr() const { return data_.data(); }

        // ====================================================================
        // Writing Primitives (Little-Endian)
        // ====================================================================

        /**
         * @brief Write a single byte
         */
        void writeU8(uint8_t value)
        {
            data_.push_back(value);
        }

        /**
         * @brief Write a signed byte
         */
        void writeI8(int8_t value)
        {
            writeU8(static_cast<uint8_t>(value));
        }

        /**
         * @brief Write an unsigned 16-bit integer (little-endian)
         */
        void writeU16(uint16_t value)
        {
            data_.push_back(static_cast<uint8_t>(value & 0xFF));
            data_.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        }

        /**
         * @brief Write a signed 16-bit integer (little-endian)
         */
        void writeI16(int16_t value)
        {
            writeU16(static_cast<uint16_t>(value));
        }

        /**
         * @brief Write an unsigned 32-bit integer (little-endian)
         */
        void writeU32(uint32_t value)
        {
            data_.push_back(static_cast<uint8_t>(value & 0xFF));
            data_.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
            data_.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
            data_.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
        }

        /**
         * @brief Write a signed 32-bit integer (little-endian)
         */
        void writeI32(int32_t value)
        {
            writeU32(static_cast<uint32_t>(value));
        }

        /**
         * @brief Write an unsigned 64-bit integer (little-endian)
         */
        void writeU64(uint64_t value)
        {
            data_.push_back(static_cast<uint8_t>(value & 0xFF));
            data_.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
            data_.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
            data_.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
            data_.push_back(static_cast<uint8_t>((value >> 32) & 0xFF));
            data_.push_back(static_cast<uint8_t>((value >> 40) & 0xFF));
            data_.push_back(static_cast<uint8_t>((value >> 48) & 0xFF));
            data_.push_back(static_cast<uint8_t>((value >> 56) & 0xFF));
        }

        /**
         * @brief Write a signed 64-bit integer (little-endian)
         */
        void writeI64(int64_t value)
        {
            writeU64(static_cast<uint64_t>(value));
        }

        /**
         * @brief Write a 32-bit float (little-endian)
         */
        void writeFloat(float value)
        {
            uint32_t bits;
            std::memcpy(&bits, &value, sizeof(float));
            writeU32(bits);
        }

        /**
         * @brief Write a 64-bit double (little-endian)
         */
        void writeDouble(double value)
        {
            uint64_t bits;
            std::memcpy(&bits, &value, sizeof(double));
            writeU64(bits);
        }

        // ====================================================================
        // Writing Arrays and Buffers
        // ====================================================================

        /**
         * @brief Write raw bytes from a buffer
         * @param buffer Source buffer
         * @param count Number of bytes to write
         */
        void writeBytes(const uint8_t *buffer, size_t count)
        {
            data_.insert(data_.end(), buffer, buffer + count);
        }

        /**
         * @brief Write raw bytes from a vector
         * @param bytes Vector containing the bytes
         */
        void writeBytes(const std::vector<uint8_t> &bytes)
        {
            data_.insert(data_.end(), bytes.begin(), bytes.end());
        }

        /**
         * @brief Write null-terminated string
         * @param str The string to write (null terminator will be added)
         */
        void writeCString(const std::string &str)
        {
            data_.insert(data_.end(), str.begin(), str.end());
            data_.push_back(0); // Null terminator
        }

        /**
         * @brief Write fixed-length string (without null terminator)
         * @param str The string to write
         * @param length Fixed length (will pad with zeros or truncate)
         */
        void writeString(const std::string &str, size_t length)
        {
            size_t writeLen = std::min(str.size(), length);
            data_.insert(data_.end(), str.begin(), str.begin() + writeLen);

            // Pad with zeros if needed
            if (writeLen < length)
            {
                data_.insert(data_.end(), length - writeLen, 0);
            }
        }

        /**
         * @brief Write string without length or terminator
         * @param str The string to write
         */
        void writeString(const std::string &str)
        {
            data_.insert(data_.end(), str.begin(), str.end());
        }

        // ====================================================================
        // Padding and Alignment
        // ====================================================================

        /**
         * @brief Write padding bytes (zeros)
         * @param count Number of padding bytes
         */
        void writePadding(size_t count)
        {
            data_.insert(data_.end(), count, 0);
        }

        /**
         * @brief Align to boundary by adding padding if needed
         * @param alignment Alignment boundary (must be power of 2)
         */
        void alignTo(size_t alignment)
        {
            size_t remainder = data_.size() % alignment;
            if (remainder != 0)
            {
                writePadding(alignment - remainder);
            }
        }

        // ====================================================================
        // Buffer Operations
        // ====================================================================

        /**
         * @brief Write data to another writer
         * @param other The writer to append to
         */
        void writeTo(BinaryWriter &other) const
        {
            other.writeBytes(data_);
        }

        /**
         * @brief Get a copy of the data
         */
        std::vector<uint8_t> toVector() const
        {
            return data_;
        }

        /**
         * @brief Move the data out of the writer
         */
        std::vector<uint8_t> moveData()
        {
            return std::move(data_);
        }

    private:
        std::vector<uint8_t> data_;
    };

}