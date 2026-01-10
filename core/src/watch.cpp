#include <spdlog/spdlog.h>

#include "tomtom/defines.hpp"
#include "tomtom/watch.hpp"
#include "tomtom/protocol/definition/protocol.hpp"

namespace tomtom
{
    Watch::Watch(std::shared_ptr<transport::DeviceConnection> conn) : connection(std::move(conn))
    {
        if (!connection)
        {
            throw std::invalid_argument("Connection cannot be null");
        }

        if (!connection->open())
        {
            throw std::runtime_error("Failed to open connection to the watch");
        }

        packet_handler_ = std::make_shared<protocol::runtime::PacketHandler>(connection);
        file_service_ = std::make_shared<services::FileService>(packet_handler_);
        info_service_ = std::make_unique<services::WatchInfoService>(packet_handler_);
        control_service_ = std::make_shared<services::WatchControlService>(packet_handler_);

        // Initialize domain-specific services
        activity_service_ = std::make_unique<services::activity::ActivityService>(file_service_);
        preferences_service_ = std::make_unique<services::preferences::PreferencesService>(file_service_);
        // tracking_service_ = std::make_unique<services::tracking::TrackingService>(file_service_);  // TODO: Not yet implemented
        // route_service_ = std::make_unique<services::routes::RouteService>(file_service_);          // TODO: Not yet implemented
        // manifest_service_ = std::make_unique<services::manifest::ManifestService>(*file_service_); // TODO: Not yet implemented
        gps_quickfix_service_ = std::make_unique<services::gps_quickfix::GpsQuickFixService>(file_service_, control_service_);

        spdlog::info("Connected to watch: {} (Product ID: 0x{:04X}, Serial: {})",
                     getProductName(), getProductId(), getSerialNumber());
    }

    Watch::~Watch()
    {
        if (connection && connection->isOpen())
        {
            connection->close();
        }
    }

    Watch::Watch(Watch &&other) noexcept : connection(std::move(other.connection))
    {
    }

    Watch &Watch::operator=(Watch &&other) noexcept
    {
        if (this != &other)
        {
            connection = std::move(other.connection);
        }
        return *this;
    }

}