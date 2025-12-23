#pragma once

#include <string>
#include <vector>
#include <memory>
#include "tomtom/manager.hpp"

namespace tomtom::cli
{

    class CliApp
    {
    public:
        CliApp();
        ~CliApp();

        int run(int argc, char *argv[]);

    private:
        void printUsage();
        void printVersion();
        int handleCommand(const std::vector<std::string> &args);

        std::unique_ptr<Manager> manager_;
    };

} // namespace tomtom::cli