#pragma once

#include "tomtom/protocol/definition/packet_header.hpp"
#include "tomtom/protocol/definition/packet_types.hpp"
#include "transaction_exceptions.hpp"

namespace tomtom::protocol::runtime
{
    class PacketValidator
    {
    public:
        /**
         * @brief Validates the common header fields of a received packet.
         *
         * @param header The received header.
         * @throws MalformedPacketError if the header is invalid (e.g. wrong direction).
         */
        static void validateHeader(const definition::PacketHeader &header)
        {
            if (header.direction != definition::PacketDirection::RX)
            {
                throw MalformedPacketError("Invalid packet direction (expected RX)");
            }

            // Note: We cannot easily validate 'Length' here without knowing the
            // total bytes read, but the PacketHandler logic usually ensures
            // read correctness based on this field.
        }

        /**
         * @brief Verifies that a received packet matches the expected type.
         *
         * @tparam ExpectedPacket The full Packet<T> type being expected.
         * @param header The received header.
         * @throws UnexpectedPacketError if types do not match.
         */
        template <typename ExpectedPacket>
        static void expectType(const definition::PacketHeader &header)
        {
            using Traits = definition::PayloadTraits<typename ExpectedPacket::PayloadType>;

            // Note: Packet<T> doesn't expose PayloadType directly in your definition,
            // so we infer it from the packet instance or use the Packet's payload member type.
            // A cleaner way is accessing the type from the header directly against Traits.

            constexpr definition::MessageType expected = Traits::type;

            if (header.type != expected)
            {
                // Allow UNKNOWN/Generic matches if necessary, but strictly:
                throw UnexpectedPacketError(expected, header.type);
            }
        }
    };
}