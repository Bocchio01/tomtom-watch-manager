#include <iostream>
#include "cli/cli_app.hpp"

int main(int argc, char *argv[])
{
    try
    {
        tomtom::cli::CliApp app;
        return app.run(argc, argv);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}