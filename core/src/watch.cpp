#include <spdlog/spdlog.h>

#include "tomtom/defines.hpp"
#include "tomtom/watch.hpp"
#include "tomtom/protocol/definition/protocol.hpp"

namespace tomtom
{
    Watch::Watch(std::shared_ptr<transport::DeviceConnection> connection)
        : connection_(std::move(connection))
    {
        if (!connection_)
        {
            throw std::invalid_argument("Connection cannot be null");
        }

        if (!connection_->open())
        {
            throw std::runtime_error("Failed to open connection to the watch");
        }

        packet_handler_ = std::make_shared<protocol::runtime::PacketHandler>(connection_);
        file_service_ = std::make_shared<services::files::FileService>(packet_handler_);
        watch_service_ = std::make_shared<services::watch::WatchService>(packet_handler_);

        activity_service_ = std::make_shared<services::activity::ActivityService>(file_service_);
        preferences_service_ = std::make_shared<services::preferences::PreferencesService>(file_service_);
        gps_quickfix_service_ = std::make_shared<services::gps_quickfix::GpsQuickFixService>(file_service_, watch_service_, preferences_service_);

        spdlog::info("Connected to watch: {} (Product ID: 0x{:04X}, Serial: {})",
                     getProductName(), getProductId(), getSerialNumber());
    }

    Watch::~Watch()
    {
        if (connection_ && connection_->isOpen())
        {
            connection_->close();
        }
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