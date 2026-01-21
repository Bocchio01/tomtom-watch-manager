#include <tomtom/cli/cli_helpers.hpp>
#include <CLI/CLI.hpp>
#include <iostream>

using namespace tomtom;
using namespace tomtom::cli;

namespace commands
{
    void setupGpsCommands(CLI::App &app, CliContext &ctx)
    {
        auto *gps = app.add_subcommand("gps", "GPS QuickFix updates");

        auto *update = gps->add_subcommand("update", "Update QuickFix data");
        update->callback([&ctx]()
                         {
            auto& mgr = ctx.getManager();

            // Check if update is needed
            if (!mgr.gps()->isUpdateNeeded() && !ctx.unsafe_mode) { // Use unsafe to force
                std::cout << "GPS data is up to date (use --unsafe to force).\n";
                return;
            }

            ProgressIndicator progress("Updating QuickFix");
            mgr.gps()->updateQuickFix([&progress](float p, const std::string& msg) {
                progress.update(static_cast<size_t>(p * 100), 100, msg);
            });
            progress.complete("GPS Updated"); });
    }
}
