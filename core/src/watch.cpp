#include "tomtom/core/watch.hpp"

namespace tomtom::core
{
    Watch::Watch(std::shared_ptr<transport::DeviceConnection> connection)
        : connection_(std::move(connection)),
          packet_handler_(std::make_shared<protocol::runtime::PacketHandler>(connection_)),
          kernel_operations_(std::make_unique<commands::KernelOperations>(packet_handler_)),
          file_operations_(std::make_unique<commands::FileOperations>(packet_handler_))
    {
        if (!connection_ || !connection_->isOpen())
        {
            throw std::invalid_argument("DeviceConnection must be valid and open");
        }
    }

    Watch::~Watch()
    {
    }

    Watch::Watch(Watch &&other) noexcept : connection_(std::move(other.connection_))
    {
    }

    Watch &Watch::operator=(Watch &&other) noexcept
    {
        if (this != &other)
        {
            connection_ = std::move(other.connection_);
        }
        return *this;
    }

}