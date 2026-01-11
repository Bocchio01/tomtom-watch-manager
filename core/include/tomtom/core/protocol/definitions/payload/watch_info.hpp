#pragma once

#include "../packet_types.hpp"
#include "../payload_traits.hpp"

#pragma pack(push, 1)

namespace tomtom::core::protocol::definitions
{

    // ============================================================================
    // Watch Time
    // ============================================================================

    /**
     * @brief Request current watch time
     */
    struct GetWatchTimeTxPayload
    {
        // Empty payload
    };

    template <>
    struct PayloadTraits<GetWatchTimeTxPayload>
    {
        static constexpr MessageType type = MessageType::GET_WATCH_TIME;
        static constexpr PacketDirection direction = PacketDirection::TX;
    };

    /**
     * @brief Watch time response
     *
     * @details Time is typically Unix timestamp (seconds since epoch)
     */
    struct GetWatchTimeRxPayload
    {
        uint32_t time;
        uint32_t reserved_1;
        uint32_t reserved_2;
        uint32_t reserved_3;
        uint32_t reserved_4;
    };

    template <>
    struct PayloadTraits<GetWatchTimeRxPayload>
    {
        static constexpr MessageType type = MessageType::GET_WATCH_TIME;
        static constexpr PacketDirection direction = PacketDirection::RX;
    };

    // ============================================================================
    // Firmware Version
    // ============================================================================

    /**
     * @brief Request firmware version string
     */
    struct GetFirmwareVersionTxPayload
    {
        // Empty payload
    };

    template <>
    struct PayloadTraits<GetFirmwareVersionTxPayload>
    {
        static constexpr MessageType type = MessageType::GET_FIRMWARE_VERSION;
        static constexpr PacketDirection direction = PacketDirection::TX;
    };

    /**
     * @brief Firmware version response
     *
     * @details Variable-length string follows this structure
     */
    struct GetFirmwareVersionRxPayload
    {
        // uint8_t version_string[];
    };

    template <>
    struct PayloadTraits<GetFirmwareVersionRxPayload>
    {
        static constexpr MessageType type = MessageType::GET_FIRMWARE_VERSION;
        static constexpr PacketDirection direction = PacketDirection::RX;
    };

    // ============================================================================
    // BLE Version
    // ============================================================================

    /**
     * @brief Request BLE firmware version
     */
    struct GetBleVersionTxPayload
    {
        // Empty payload
    };

    template <>
    struct PayloadTraits<GetBleVersionTxPayload>
    {
        static constexpr MessageType type = MessageType::GET_BLE_VERSION;
        static constexpr PacketDirection direction = PacketDirection::TX;
    };

    /**
     * @brief BLE version response
     *
     * @details Variable-length string follows this structure
     */
    struct GetBleVersionRxPayload
    {
        // uint8_t ble_version_string[];
    };

    template <>
    struct PayloadTraits<GetBleVersionRxPayload>
    {
        static constexpr MessageType type = MessageType::GET_BLE_VERSION;
        static constexpr PacketDirection direction = PacketDirection::RX;
    };

    // ============================================================================
    // Product ID
    // ============================================================================

    /**
     * @brief Request product identifier
     */
    struct GetProductIdTxPayload
    {
        // Empty payload
    };

    template <>
    struct PayloadTraits<GetProductIdTxPayload>
    {
        static constexpr MessageType type = MessageType::GET_PRODUCT_ID;
        static constexpr PacketDirection direction = PacketDirection::TX;
    };

    /**
     * @brief Product ID response
     */
    struct GetProductIdRxPayload
    {
        uint32_t product_id;
    };

    template <>
    struct PayloadTraits<GetProductIdRxPayload>
    {
        static constexpr MessageType type = MessageType::GET_PRODUCT_ID;
        static constexpr PacketDirection direction = PacketDirection::RX;
    };

}

#pragma pack(pop)
