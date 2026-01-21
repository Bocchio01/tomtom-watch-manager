#include <tomtom/cli/cli_helpers.hpp>
#include <tomtom/core/transport/connection_factory.hpp>
#include <CLI/CLI.hpp>
#include <iostream>

using namespace tomtom;
using namespace tomtom::cli;

namespace commands
{
    void setupDeviceCommands(CLI::App &app, CliContext &ctx)
    {
        auto *device = app.add_subcommand("device", "Device management");

        // Sync Time
        device->add_subcommand("sync-time", "Synchronize watch time with PC")->callback([&ctx]()
                                                                                        {
            auto& mgr = ctx.getManager();
            ProgressIndicator progress("Synchronizing time");
            mgr.device()->syncTime();
            progress.complete(); });

        // Get Stats
        device->add_subcommand("info", "Show device information")->callback([&ctx]()
                                                                            {
            auto& mgr = ctx.getManager();
            auto stats = mgr.device()->getStats();

            std::cout << "Device Info:\n";
            std::cout << "  Product: " << Color::bold(stats.product_name) << "\n";
            std::cout << "  Serial:  " << stats.serial_number << "\n";
            std::cout << "  Battery: " << stats.battery_level << "%\n"; });

        // List Devices (doesn't require manager)
        device->add_subcommand("list", "List detected devices")->callback([]()
                                                                          {
            auto devices = core::transport::DeviceConnectionFactory::enumerate();
            if (devices.empty()) {
                std::cout << "No devices found.\n";
                return;
            }
            std::cout << "Found " << devices.size() << " device(s):\n";
            for (const auto& dev : devices) {
                std::cout << "  " << Color::bold(dev.product_name) << "\n";
                std::cout << "    Serial: " << dev.serial_number << "\n";
                std::cout << "    VID:PID: " << std::hex << static_cast<int>(dev.vendor_id)
                          << ":" << static_cast<int>(dev.product_id) << std::dec << "\n";
            } });
    }
}
