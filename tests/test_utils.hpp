#pragma once

#include <spdlog/spdlog.h>

// Test counter (must be defined in each test file)
extern int tests_passed;
extern int tests_failed;

// Test function definition macro
#define TEST(name) \
    void name();   \
    void name()

// Basic assertion macros
#define ASSERT_TRUE(condition, message)                          \
    do                                                           \
    {                                                            \
        if (!(condition))                                        \
        {                                                        \
            spdlog::error("FAILED: {} - {}", __func__, message); \
            tests_failed++;                                      \
            return;                                              \
        }                                                        \
    } while (0)

#define ASSERT_EQ(expected, actual, message)                     \
    do                                                           \
    {                                                            \
        if ((expected) != (actual))                              \
        {                                                        \
            spdlog::error("FAILED: {} - {}", __func__, message); \
            tests_failed++;                                      \
            return;                                              \
        }                                                        \
    } while (0)

#define ASSERT_NOT_NULL(ptr, message)                            \
    do                                                           \
    {                                                            \
        if (!(ptr))                                              \
        {                                                        \
            spdlog::error("FAILED: {} - {}", __func__, message); \
            tests_failed++;                                      \
            return;                                              \
        }                                                        \
    } while (0)

#define ASSERT_GE(actual, expected, message)                     \
    do                                                           \
    {                                                            \
        if ((actual) < (expected))                               \
        {                                                        \
            spdlog::error("FAILED: {} - {}", __func__, message); \
            tests_failed++;                                      \
            return;                                              \
        }                                                        \
    } while (0)

// Test runner macro
#define RUN_TEST(test_func)                           \
    do                                                \
    {                                                 \
        spdlog::info("Running test: {}", #test_func); \
        test_func();                                  \
        tests_passed++;                               \
    } while (0)
