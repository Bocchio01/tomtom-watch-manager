#pragma once

#include <cstdint>
#include <type_traits>

namespace tomtom::core::utils
{

    /**
     * @brief Compile-time endianness detection and conversion utilities (C++17)
     *
     * This file provides utilities for detecting system endianness and converting
     * between little-endian and big-endian byte orders.
     */

    // ====================================================================
    // Endianness Detection (C++17 compatible)
    // ====================================================================

    namespace detail
    {
        constexpr uint32_t endian_test = 0x01020304;
        constexpr uint8_t endian_byte = (const uint8_t &)endian_test;
    }

    /**
     * @brief Detect system endianness at compile time
     */
    constexpr bool isLittleEndian()
    {
        return detail::endian_byte == 0x04;
    }

    constexpr bool isBigEndian()
    {
        return detail::endian_byte == 0x01;
    }

    // ====================================================================
    // Byte Swapping Functions
    // ====================================================================

    /**
     * @brief Swap bytes in a 16-bit value
     */
    constexpr uint16_t byteSwap16(uint16_t value)
    {
        return (value >> 8) | (value << 8);
    }

    /**
     * @brief Swap bytes in a 32-bit value
     */
    constexpr uint32_t byteSwap32(uint32_t value)
    {
        return ((value >> 24) & 0x000000FF) |
               ((value >> 8) & 0x0000FF00) |
               ((value << 8) & 0x00FF0000) |
               ((value << 24) & 0xFF000000);
    }

    /**
     * @brief Swap bytes in a 64-bit value
     */
    constexpr uint64_t byteSwap64(uint64_t value)
    {
        return ((value >> 56) & 0x00000000000000FFULL) |
               ((value >> 40) & 0x000000000000FF00ULL) |
               ((value >> 24) & 0x0000000000FF0000ULL) |
               ((value >> 8) & 0x00000000FF000000ULL) |
               ((value << 8) & 0x000000FF00000000ULL) |
               ((value << 24) & 0x0000FF0000000000ULL) |
               ((value << 40) & 0x00FF000000000000ULL) |
               ((value << 56) & 0xFF00000000000000ULL);
    }

    // ====================================================================
    // Little-Endian Conversions (for TomTom watch data)
    // ====================================================================

    /**
     * @brief Convert from little-endian to host byte order
     */
    template <typename T>
    constexpr T littleEndianToHost(T value)
    {
        if (sizeof(T) == 1)
        {
            return value;
        }
        else if (isLittleEndian())
        {
            return value; // Already in correct order
        }
        else // Big endian system
        {
            if (sizeof(T) == 2)
                return static_cast<T>(byteSwap16(static_cast<uint16_t>(value)));
            else if (sizeof(T) == 4)
                return static_cast<T>(byteSwap32(static_cast<uint32_t>(value)));
            else if (sizeof(T) == 8)
                return static_cast<T>(byteSwap64(static_cast<uint64_t>(value)));
        }
        return value;
    }

    /**
     * @brief Convert from host byte order to little-endian
     */
    template <typename T>
    constexpr T hostToLittleEndian(T value)
    {
        // Conversion is symmetric
        return littleEndianToHost(value);
    }

    // ====================================================================
    // Big-Endian Conversions
    // ====================================================================

    /**
     * @brief Convert from big-endian to host byte order
     */
    template <typename T>
    constexpr T bigEndianToHost(T value)
    {
        if (sizeof(T) == 1)
        {
            return value;
        }
        else if (isBigEndian())
        {
            return value; // Already in correct order
        }
        else // Little endian system
        {
            if (sizeof(T) == 2)
                return static_cast<T>(byteSwap16(static_cast<uint16_t>(value)));
            else if (sizeof(T) == 4)
                return static_cast<T>(byteSwap32(static_cast<uint32_t>(value)));
            else if (sizeof(T) == 8)
                return static_cast<T>(byteSwap64(static_cast<uint64_t>(value)));
        }
        return value;
    }

    /**
     * @brief Convert from host byte order to big-endian
     */
    template <typename T>
    constexpr T hostToBigEndian(T value)
    {
        // Conversion is symmetric
        return bigEndianToHost(value);
    }

}