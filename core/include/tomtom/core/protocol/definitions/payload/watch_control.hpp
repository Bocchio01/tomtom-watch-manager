#pragma once

#include "../packet_types.hpp"
#include "../payload_traits.hpp"

#pragma pack(push, 1)

namespace tomtom::core::protocol::definitions
{

    // ============================================================================
    // Format Watch
    // ============================================================================

    /**
     * @brief Request to format watch storage
     *
     * @warning This operation erases all user data on the watch!
     */
    struct FormatWatchTxPayload
    {
        // Empty payload
    };

    template <>
    struct PayloadTraits<FormatWatchTxPayload>
    {
        static constexpr MessageType type = MessageType::FORMAT_WATCH;
        static constexpr PacketDirection direction = PacketDirection::TX;
    };

    /**
     * @brief Format operation response
     */
    struct FormatWatchRxPayload
    {
        uint32_t reserved_1;
        uint32_t reserved_2;
        uint32_t reserved_3;
        uint32_t reserved_4;
        ProtocolError error;
    };

    template <>
    struct PayloadTraits<FormatWatchRxPayload>
    {
        static constexpr MessageType type = MessageType::FORMAT_WATCH;
        static constexpr PacketDirection direction = PacketDirection::RX;
    };

    // ============================================================================
    // Reset Device
    // ============================================================================

    /**
     * @brief Request device reset/reboot
     *
     * @details Causes the watch to restart. No response is typically received.
     */
    struct ResetDeviceTxPayload
    {
        // Empty payload
    };

    template <>
    struct PayloadTraits<ResetDeviceTxPayload>
    {
        static constexpr MessageType type = MessageType::RESET_DEVICE;
        static constexpr PacketDirection direction = PacketDirection::TX;
    };

    // ============================================================================
    // Reset GPS Processor
    // ============================================================================

    /**
     * @brief Request GPS processor reset
     *
     * @details Resets the GPS subsystem without rebooting the entire watch
     */
    struct ResetGpsTxPayload
    {
        // Empty payload
    };

    template <>
    struct PayloadTraits<ResetGpsTxPayload>
    {
        static constexpr MessageType type = MessageType::RESET_GPS_PROCESSOR;
        static constexpr PacketDirection direction = PacketDirection::TX;
    };

    /**
     * @brief GPS reset response
     *
     * @details Variable-length message follows (typically reboot confirmation)
     */
    struct ResetGpsRxPayload
    {
        // uint8_t reboot_message[];
    };

    template <>
    struct PayloadTraits<ResetGpsRxPayload>
    {
        static constexpr MessageType type = MessageType::RESET_GPS_PROCESSOR;
        static constexpr PacketDirection direction = PacketDirection::RX;
    };

}

#pragma pack(pop)
