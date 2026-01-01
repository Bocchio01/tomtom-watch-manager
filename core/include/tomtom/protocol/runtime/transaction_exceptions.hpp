#pragma once

#include <stdexcept>
#include <string>

#include "tomtom/protocol/definition/packet_types.hpp"

namespace tomtom::protocol::runtime
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
        UnexpectedPacketError(definition::MessageType expected, definition::MessageType actual)
            : CodecError("Unexpected packet type received"), expected_(expected), actual_(actual)
        {
        }

        definition::MessageType expected() const { return expected_; }
        definition::MessageType actual() const { return actual_; }

    private:
        definition::MessageType expected_;
        definition::MessageType actual_;
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