#include <tomtom/cli/cli_helpers.hpp>
#include <tomtom/core/transport/connection_factory.hpp>
#include <CLI/CLI.hpp>
#include <iostream>

using namespace tomtom;
using namespace tomtom::cli;

namespace commands
{
    void setupUtilsCommands(CLI::App &app, CliContext &ctx)
    {
        auto *utils = app.add_subcommand("utils", "Utilities");

        utils->add_subcommand("list-devices", "List connected USB devices")->callback([]()
                                                                                      {
            // This does NOT use the Manager, to allow debugging connection issues
            auto devices = tomtom::core::transport::DeviceConnectionFactory::enumerate();
            if (devices.empty()) {
                std::cout << "No devices found.\n";
                return;
            }
            for (const auto& d : devices) {
                std::cout << d.product_name << " (" << d.serial_number << ")\n";
            } });

        utils->add_subcommand("config-path", "Show path to config file")->callback([&ctx]()
                                                                                   {
            auto& mgr = ctx.getManager();
            std::cout << mgr.store().getConfigPath().string() << "\n"; });
    }
}
