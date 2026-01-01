#include <iostream>
#include <spdlog/spdlog.h>

#include "tomtom/manager.hpp"
#include "tomtom/watch.hpp"
#include "../test_utils.hpp"

using namespace tomtom;

// Test counter
int tests_passed = 0;
int tests_failed = 0;

// =============================================================================
// Manager Tests (Require Hardware)
// =============================================================================

TEST(test_get_time)
{
    Manager manager;
    auto watch = manager.connectToWatch();

    watch->printTime();
}

// =============================================================================
// Main Test Runner
// =============================================================================

int main()
{
    spdlog::set_level(spdlog::level::info);
    spdlog::info("=================================================");
    spdlog::info("Running Manager Integration Tests");
    spdlog::info("Note: These tests require a connected watch");
    spdlog::info("=================================================");

    // Run tests
    RUN_TEST(test_get_time);

    // Print results
    spdlog::info("=================================================");
    spdlog::info("Test Results:");
    spdlog::info("  Passed: {}", tests_passed);
    spdlog::info("  Failed: {}", tests_failed);
    spdlog::info("=================================================");

    if (tests_failed > 0)
    {
        spdlog::error("Some tests failed!");
        return 1;
    }

    spdlog::info("All tests passed!");
    return 0;
}