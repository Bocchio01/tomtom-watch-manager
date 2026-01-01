#pragma once

/**
 * @file protocol.hpp
 * @brief Main include file for TomTom Watch Protocol
 *
 * @details
 * This header provides the complete protocol protocol for communicating
 * with TomTom watches. It includes:
 *
 * - Core types and enumerations
 * - Packet structures with automatic header management
 * - All payload definitions organized by functionality
 * - Type-safe packet aliases for TX/RX operations
 *
 * Usage:
 * @code
 * #include "protocol.hpp"
 *
 * using namespace tomtom::interface::protocol;
 *
 * // Create a packet to read a file
 * OpenFileReadTx packet;
 * packet.payload.file_id = FileId(0x12345678);
 * packet.header.counter = 42;
 *
 * // Send packet...
 * @endcode
 */

// Core components
#include "packet_types.hpp"
#include "packet_header.hpp"
#include "payload_traits.hpp"
#include "packet.hpp"

// Payload definitions
#include "payload/common.hpp"
#include "payload/file_operations.hpp"
#include "payload/file_transfer.hpp"
#include "payload/file_discovery.hpp"
#include "payload/watch_info.hpp"
#include "payload/watch_control.hpp"

namespace tomtom::interface::protocol
{
    // ============================================================================
    // File Operation Packets (TX)
    // ============================================================================

    using OpenFileReadTx = Packet<OpenFileReadTxPayload>;
    using OpenFileWriteTx = Packet<OpenFileWriteTxPayload>;
    using CloseFileTx = Packet<CloseFileTxPayload>;
    using DeleteFileTx = Packet<DeleteFileTxPayload>;
    using GetFileSizeTx = Packet<GetFileSizeTxPayload>;

    // ============================================================================
    // File Transfer Packets (TX)
    // ============================================================================

    using ReadFileDataTx = Packet<ReadFileDataTxPayload>;
    using WriteFileDataTx = Packet<WriteFileDataTxPayload>;

    // ============================================================================
    // File Discovery Packets (TX)
    // ============================================================================

    using FindFirstFileTx = Packet<FindFirstFileTxPayload>;
    using FindNextFileTx = Packet<FindNextFileTxPayload>;

    // ============================================================================
    // Watch Information Packets (TX)
    // ============================================================================

    using GetWatchTimeTx = Packet<GetWatchTimeTxPayload>;
    using GetFirmwareVersionTx = Packet<GetFirmwareVersionTxPayload>;
    using GetBleVersionTx = Packet<GetBleVersionTxPayload>;
    using GetProductIdTx = Packet<GetProductIdTxPayload>;

    // ============================================================================
    // Watch Control Packets (TX)
    // ============================================================================

    using FormatWatchTx = Packet<FormatWatchTxPayload>;
    using ResetDeviceTx = Packet<ResetDeviceTxPayload>;
    using ResetGpsTx = Packet<ResetGpsTxPayload>;

    // ============================================================================
    // File Operation Packets (RX)
    // ============================================================================

    using OpenFileReadRx = Packet<OpenFileReadRxPayload>;
    using OpenFileWriteRx = Packet<OpenFileWriteRxPayload>;
    using CloseFileRx = Packet<CloseFileRxPayload>;
    using DeleteFileRx = Packet<DeleteFileRxPayload>;
    using GetFileSizeRx = Packet<GetFileSizeRxPayload>;

    // ============================================================================
    // File Transfer Packets (RX)
    // ============================================================================

    using ReadFileDataRx = Packet<ReadFileDataRxPayload>;
    using WriteFileDataRx = Packet<WriteFileDataRxPayload>;

    // ============================================================================
    // File Discovery Packets (RX)
    // ============================================================================

    using FindFirstFileRx = Packet<FindFirstFileRxPayload>;
    using FindNextFileRx = Packet<FindNextFileRxPayload>;

    // ============================================================================
    // Watch Information Packets (RX)
    // ============================================================================

    using GetWatchTimeRx = Packet<GetWatchTimeRxPayload>;
    using GetFirmwareVersionRx = Packet<GetFirmwareVersionRxPayload>;
    using GetBleVersionRx = Packet<GetBleVersionRxPayload>;
    using GetProductIdRx = Packet<GetProductIdRxPayload>;

    // ============================================================================
    // Watch Control Packets (RX)
    // ============================================================================

    using FormatWatchRx = Packet<FormatWatchRxPayload>;
    using ResetGpsRx = Packet<ResetGpsRxPayload>;

}

/**
 * @namespace tomtom::interface::protocol
 * @brief TomTom watch communication protocol
 *
 * This namespace contains all types and structures for implementing
 * the TomTom watch binary protocol over USB/BLE connections.
 */
