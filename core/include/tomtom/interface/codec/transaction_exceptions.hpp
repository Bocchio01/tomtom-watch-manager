#pragma once

#include <stdexcept>
#include <string>

#include "tomtom/interface/protocol/packet_types.hpp"

namespace tomtom::interface::codec
{
    /**
     * @brief Base class for all codec/transaction related errors
     */
    class CodecError : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    /**
     * @brief Thrown when a device operation times out
     */
    class TimeoutError : public CodecError
    {
    public:
        explicit TimeoutError(const std::string &msg) : CodecError(msg) {}
    };

    /**
     * @brief Thrown when received data violates protocol rules
     * (e.g., wrong direction, invalid length, checksum mismatch)
     */
    class MalformedPacketError : public CodecError
    {
    public:
        explicit MalformedPacketError(const std::string &msg) : CodecError(msg) {}
    };

    /**
     * @brief Thrown when the received packet type does not match the expected type
     */
    class UnexpectedPacketError : public CodecError
    {
    public:
        UnexpectedPacketError(protocol::MessageType expected, protocol::MessageType actual)
            : CodecError("Unexpected packet type received"), expected_(expected), actual_(actual)
        {
        }

        protocol::MessageType expected() const { return expected_; }
        protocol::MessageType actual() const { return actual_; }

    private:
        protocol::MessageType expected_;
        protocol::MessageType actual_;
    };

    /**
     * @brief Thrown when the underlying connection fails (IO error)
     */
    class ConnectionError : public CodecError
    {
    public:
        explicit ConnectionError(const std::string &msg) : CodecError(msg) {}
    };
}