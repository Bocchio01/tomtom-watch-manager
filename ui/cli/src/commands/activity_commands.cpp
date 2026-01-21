#include <tomtom/cli/cli_helpers.hpp>
#include <tomtom/sdk/models/activity_model.hpp>
#include <tomtom/sdk/converters/gpx_converter.hpp>
#include <CLI/CLI.hpp>
#include <iostream>

using namespace tomtom;
using namespace tomtom::cli;

namespace commands
{
    void setupActivityCommands(CLI::App &app, CliContext &ctx)
    {
        auto *act = app.add_subcommand("activity", "Activity management");

        // LIST
        act->add_subcommand("list", "List files on watch")->callback([&ctx]()
                                                                     {
                                                                         auto &mgr = ctx.getManager();
                                                                         auto files = mgr.activity()->listActivities();

                                                                         std::cout << "Found " << files.size() << " activity files on watch.\n"; });

        // SYNC / DOWNLOAD
        auto *get = act->add_subcommand("sync", "Download and process activities");
        get->callback([&ctx]()
                      {
            auto& mgr = ctx.getManager();

            auto files = mgr.activity()->listActivities();
            if (files.empty()) {
                std::cout << "No new activities found.\n";
                return;
            }

            std::cout << "Processing " << files.size() << " activities...\n";

            int processed = 0;

            for (const auto& fileId : files) {
                try {
                    // Download activity (service handles parsing and raw storage automatically)
                    std::cout << "  Downloading 0x" << std::hex << fileId.value << std::dec << "... " << std::flush;

                    auto activity = mgr.activity()->downloadActivity(fileId,
                        [](size_t, size_t, const std::string&) {
                            // Progress callback if needed
                        });

                    if (!activity) {
                        std::cout << Color::red("Failed") << "\n";
                        continue;
                    }

                    std::string dateStr = Format::formatDateTime(activity->start_time);

                    // Export to configured formats using converters
                    // For now, we'll export to GPX as an example
                    try {
                        sdk::converters::GpxConverter gpxConverter;
                        auto path = mgr.activity()->exportAndSaveActivity(*activity, gpxConverter);
                        std::cout << Color::green("Saved") << " -> " << path.filename().string() << "\n";
                        processed++;
                    } catch (const std::exception& e) {
                        std::cout << Color::yellow("Exported but save failed: ") << e.what() << "\n";
                    }

                } catch (const std::exception& e) {
                    std::cout << Color::red("Failed: ") << e.what() << "\n";
                }
            }

            std::cout << "\nSync complete. Processed " << processed << " activities.\n"; });
    }
}
