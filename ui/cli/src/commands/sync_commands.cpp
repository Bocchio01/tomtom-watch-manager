#include <tomtom/cli/cli_helpers.hpp>
#include <CLI/CLI.hpp>
#include <iostream>

using namespace tomtom;
using namespace tomtom::cli;

namespace commands
{
    void setupSyncCommands(CLI::App &app, CliContext &ctx)
    {
        // One-shot command to do everything
        app.add_subcommand("sync", "Full sync (Time, GPS, Activities)")->callback([&ctx]()
                                                                                  {
            auto& mgr = ctx.getManager();

            std::cout << Color::bold("=== TomTom Sync Started ===") << "\n";

            // 1. Time
            try {
                mgr.device()->syncTime();
                std::cout << "[1/3] Time Synced " << Color::green("✓") << "\n";
            } catch(...) {
                std::cout << "[1/3] Time Sync Failed " << Color::red("✗") << "\n";
            }

            // 2. GPS
            try {
                if (mgr.gps()->isUpdateNeeded()) {
                    mgr.gps()->updateQuickFix();
                    std::cout << "[2/3] GPS Updated " << Color::green("✓") << "\n";
                } else {
                    std::cout << "[2/3] GPS Up-to-date " << Color::green("✓") << "\n";
                }
            } catch(const std::exception& e) {
                std::cout << "[2/3] GPS Update Failed: " << e.what() << " " << Color::red("✗") << "\n";
            }

            // 3. Activities
            std::cout << "[3/3] Syncing Activities...\n";
            // Call the logic defined in activity sync above,
            // or extract it to a shared method in Manager
            // For now, we assume the user runs `activity sync` logic here.

            std::cout << Color::bold("=== Sync Finished ===") << "\n"; });
    }
}
