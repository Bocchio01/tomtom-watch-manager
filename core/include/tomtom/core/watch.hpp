#pragma once

#include "tomtom/core/transport/connection.hpp"
#include "tomtom/core/protocol/definitions/definitions.hpp"
#include "tomtom/core/protocol/runtime/packet_handler.hpp"
#include "tomtom/core/commands/kernel_operations.hpp"
#include "tomtom/core/commands/file_operations.hpp"

namespace tomtom::core
{
    /**
     * @brief Represents a TomTom watch device and provides access to its data.
     */
    class Watch
    {
    public:
        /**
         * @brief Constructs a new Watch instance.
         * @param connection Unique pointer to an established DeviceConnection.
         */
        explicit Watch(std::shared_ptr<transport::DeviceConnection> connection);

        /**
         * @brief Closes the connection to the watch and releases resources.
         */
        ~Watch();

        // Non-copyable and movable
        Watch(const Watch &) = delete;
        Watch &operator=(const Watch &) = delete;
        Watch(Watch &&) noexcept;
        Watch &operator=(Watch &&) noexcept;

        /**
         * @brief Gets the underlying DeviceConnection.
         * @return Shared pointer to the DeviceConnection.
         */
        std::shared_ptr<transport::DeviceConnection> connection() const { return connection_; }

        /**
         * @brief Gets the PacketHandler for protocol operations.
         * @return Shared pointer to the PacketHandler.
         */
        std::shared_ptr<protocol::runtime::PacketHandler> packets() const { return packet_handler_; }

        /**
         * @brief Gets the KernelOperations command interface.
         * @return Shared pointer to the KernelOperations.
         */
        commands::KernelOperations kernel() const { return *kernel_operations_; }

        /**
         * @brief Gets the FileOperations command interface.
         * @return Shared pointer to the FileOperations.
         */
        commands::FileOperations files() const { return *file_operations_; }

    private:
        std::shared_ptr<transport::DeviceConnection> connection_;
        std::shared_ptr<protocol::runtime::PacketHandler> packet_handler_;
        std::unique_ptr<commands::KernelOperations> kernel_operations_;
        std::unique_ptr<commands::FileOperations> file_operations_;
    };

}