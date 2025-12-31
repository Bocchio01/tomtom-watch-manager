#include <iostream>
#include <algorithm>
#include "cli/cli_app.hpp"
#include "cli/commands.hpp"

namespace tomtom::cli
{

    CliApp::CliApp()
        : manager_(std::make_unique<Manager>())
    {
    }

    CliApp::~CliApp() = default;

    void CliApp::printUsage()
    {
        std::cout << "TomTom Watch Manager CLI\n\n"
                  << "Usage: tomtom <command> [options]\n\n"
                  << "Commands:\n"
                  << "  list              List connected TomTom devices\n"
                  << "  info              Show device information\n"
                  << "  files             List all files on the watch\n"
                  << "  download <id> <path>  Download file from watch\n"
                  << "  upload <path> <id>    Upload file to watch\n"
                  << "  delete <id>           Delete file from watch\n"
                  << "  sync              Sync watch with cloud\n"
                  << "  firmware <path>   Update watch firmware\n"
                  << "  time              Get current watch time\n"
                  << "  version           Show version information\n"
                  << "  help              Show this help message\n"
                  << std::endl;
    }

    void CliApp::printVersion()
    {
        std::cout << "TomTom Watch Manager CLI v1.0.0" << std::endl;
    }

    int CliApp::run(int argc, char *argv[])
    {
        std::vector<std::string> args;
        for (int i = 1; i < argc; ++i)
        {
            args.emplace_back(argv[i]);
        }

        if (args.empty())
        {
            printUsage();
            return 0;
        }

        return handleCommand(args);
    }

    int CliApp::handleCommand(const std::vector<std::string> &args)
    {
        const std::string &command = args[0];
        Commands commands(*manager_);

        if (command == "help" || command == "-h" || command == "--help")
        {
            printUsage();
            return 0;
        }

        if (command == "version" || command == "-v" || command == "--version")
        {
            printVersion();
            return 0;
        }

        if (command == "list")
        {
            return commands.listDevices();
        }

        if (command == "info")
        {
            return commands.getDeviceInfo();
        }

        if (command == "files")
        {
            return commands.listFiles();
        }

        if (command == "download")
        {
            if (args.size() < 3)
            {
                std::cerr << "Error: download requires <file_id> and <output_path>" << std::endl;
                return 1;
            }
            return commands.downloadFile(args[1], args[2]);
        }

        if (command == "upload")
        {
            if (args.size() < 3)
            {
                std::cerr << "Error: upload requires <input_path> and <file_id>" << std::endl;
                return 1;
            }
            return commands.uploadFile(args[1], args[2]);
        }

        if (command == "delete")
        {
            if (args.size() < 2)
            {
                std::cerr << "Error: delete requires <file_id>" << std::endl;
                return 1;
            }
            return commands.deleteFile(args[1]);
        }

        if (command == "sync")
        {
            return commands.syncWatch();
        }

        if (command == "firmware")
        {
            if (args.size() < 2)
            {
                std::cerr << "Error: firmware requires <firmware_path>" << std::endl;
                return 1;
            }
            return commands.updateFirmware(args[1]);
        }

        if (command == "time")
        {
            return commands.getCurrentTime();
        }

        std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
        printUsage();
        return 1;
    }

}
::cli