#include <CLI/CLI.hpp>
#include <iostream>
#include <filesystem>

#include <tomtom/cli/cli_helpers.hpp>
#include <tomtom/sdk/manager.hpp>

using namespace tomtom;
using namespace tomtom::cli;

// Forward declarations
namespace commands
{
    void setupDeviceCommands(CLI::App &app, CliContext &ctx);
    void setupActivityCommands(CLI::App &app, CliContext &ctx);
    void setupGpsCommands(CLI::App &app, CliContext &ctx);
    void setupSyncCommands(CLI::App &app, CliContext &ctx);
    void setupUtilsCommands(CLI::App &app, CliContext &ctx);
}

int main(int argc, char **argv)
{
    CLI::App app{"TomTom Watch Manager"};
    app.set_version_flag("--version,-v", "1.1.0");
    app.require_subcommand(1);

    CliContext ctx;

    // Global options
    app.add_option("--config", ctx.config_path, "Override config file path");
    app.add_option("--store", ctx.activity_store_override, "Override storage root");
    app.add_option("--device,-d", ctx.device_filter, "Select device by serial/name");
    app.add_flag("--debug", ctx.debug_mode, "Enable debug output");
    app.add_flag("--packets", ctx.packet_logging, "Enable packet logging");
    app.add_flag("--unsafe", ctx.unsafe_mode, "Enable unsafe operations");
    app.add_flag("--yes,-y", ctx.auto_yes, "Auto-confirm prompts");

    commands::setupDeviceCommands(app, ctx);
    commands::setupActivityCommands(app, ctx);
    commands::setupGpsCommands(app, ctx);
    commands::setupSyncCommands(app, ctx);
    commands::setupUtilsCommands(app, ctx);

    try
    {
        CLI11_PARSE(app, argc, argv);
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << Color::red("Error: ") << e.what() << "\n";
        return 1;
    }
}
