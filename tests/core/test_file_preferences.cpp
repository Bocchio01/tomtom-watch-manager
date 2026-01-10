#include "tomtom/services/preferences/preferences.hpp"
#include "tomtom/services/file_service.hpp"
#include "tomtom/protocol/runtime/packet_handler.hpp"
#include "tomtom/transport/connection.hpp"
#include "tomtom/services/file_ids.hpp"
#include "tomtom/manager.hpp"
#include "../test_utils.hpp"

#include <ctime>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

using namespace tomtom;
using namespace tomtom::services::preferences;
using namespace tomtom::services::preferences::models;
using namespace tomtom::services;
using namespace tomtom::protocol::definition;
using namespace tomtom::protocol::runtime;

// Test counters
int tests_passed = 0;
int tests_failed = 0;

// Global watch pointer for hardware tests
std::shared_ptr<Watch> g_watch = nullptr;

// ============================================================================
// Test XML Data
// ============================================================================

const std::string SAMPLE_PREFERENCES_XML = R"(<?xml version="1.0" encoding="UTF-8"?>
<preferences>
  <version>1.0</version>
  <modified>1234567890</modified>
  <name>My TomTom Watch</name>
  <ephemeris_url>https://config.tomtom.com</ephemeris_url>
  <auth_token>sample_auth_token_12345</auth_token>
  <token_secret>sample_secret_67890</token_secret>
  <user_id>user123</user_id>
  <ephemeris_modified>1234567800</ephemeris_modified>
  <language>en_US</language>
  <units>metric</units>
</preferences>)";

const std::string MINIMAL_PREFERENCES_XML = R"(<?xml version="1.0" encoding="UTF-8"?>
<preferences>
  <version>1.0</version>
  <modified>1234567890</modified>
  <name>TomTom</name>
</preferences>)";

// ============================================================================
// Helper Functions
// ============================================================================

std::vector<uint8_t> toBytes(const std::string &str)
{
    return std::vector<uint8_t>(str.begin(), str.end());
}

std::string fromBytes(const std::vector<uint8_t> &data)
{
    return std::string(data.begin(), data.end());
}

bool stringContains(const std::string &haystack, const std::string &needle)
{
    return haystack.find(needle) != std::string::npos;
}

// ============================================================================
// PreferencesModel Tests
// ============================================================================

TEST(test_has_authentication_returns_true_when_set)
{
    Preferences prefs;
    prefs.auth_token = "token123";
    prefs.token_secret = "secret456";

    ASSERT_TRUE(prefs.hasAuthentication(), "Should have authentication when both token and secret are set");
}

TEST(test_has_authentication_returns_false_when_missing_token)
{
    Preferences prefs;
    prefs.token_secret = "secret456";

    ASSERT_TRUE(!prefs.hasAuthentication(), "Should not have authentication when token is missing");
}

TEST(test_has_authentication_returns_false_when_missing_secret)
{
    Preferences prefs;
    prefs.auth_token = "token123";

    ASSERT_TRUE(!prefs.hasAuthentication(), "Should not have authentication when secret is missing");
}

TEST(test_has_authentication_returns_false_when_empty)
{
    Preferences prefs;
    prefs.auth_token = "";
    prefs.token_secret = "secret456";

    ASSERT_TRUE(!prefs.hasAuthentication(), "Should not have authentication when token is empty");

    prefs.auth_token = "token123";
    prefs.token_secret = "";

    ASSERT_TRUE(!prefs.hasAuthentication(), "Should not have authentication when secret is empty");
}

TEST(test_has_custom_name_returns_true_when_set)
{
    Preferences prefs;
    prefs.watch_name = "My Custom Watch";

    ASSERT_TRUE(prefs.hasCustomName(), "Should have custom name when name is set");
}

TEST(test_has_custom_name_returns_false_when_empty)
{
    Preferences prefs;
    prefs.watch_name = "";

    ASSERT_TRUE(!prefs.hasCustomName(), "Should not have custom name when name is empty");
}

TEST(test_clear_authentication_removes_credentials)
{
    Preferences prefs;
    prefs.auth_token = "token123";
    prefs.token_secret = "secret456";
    prefs.user_id = "user789";

    prefs.clearAuthentication();

    ASSERT_TRUE(!prefs.auth_token.has_value(), "Auth token should be cleared");
    ASSERT_TRUE(!prefs.token_secret.has_value(), "Token secret should be cleared");
    ASSERT_TRUE(!prefs.user_id.has_value(), "User ID should be cleared");
    ASSERT_TRUE(!prefs.hasAuthentication(), "Should not have authentication after clearing");
}

// ============================================================================
// PreferencesParser Tests
// ============================================================================

TEST(test_parse_valid_xml_succeeds)
{
    PreferencesParser parser;
    auto prefs = parser.parseString(SAMPLE_PREFERENCES_XML);

    ASSERT_EQ("1.0", prefs.version, "Version should be 1.0");

    // Check timestamp (1234567890 seconds since epoch)
    auto expected_time = std::chrono::system_clock::time_point{std::chrono::seconds{1234567890}};
    ASSERT_EQ(expected_time, prefs.modified, "Modified timestamp should match");

    ASSERT_EQ("My TomTom Watch", prefs.watch_name, "Watch name should match");
    ASSERT_EQ("https://config.tomtom.com", prefs.ephemeris_url, "Config URL should match");

    ASSERT_TRUE(prefs.auth_token.has_value(), "Auth token should be present");
    ASSERT_EQ("sample_auth_token_12345", *prefs.auth_token, "Auth token should match");

    ASSERT_TRUE(prefs.token_secret.has_value(), "Token secret should be present");
    ASSERT_EQ("sample_secret_67890", *prefs.token_secret, "Token secret should match");

    ASSERT_TRUE(prefs.user_id.has_value(), "User ID should be present");
    ASSERT_EQ("user123", *prefs.user_id, "User ID should match");

    auto expected_ephemeris = std::chrono::system_clock::time_point{std::chrono::seconds{1234567800}};
    ASSERT_EQ(expected_ephemeris, prefs.ephemeris_modified, "Ephemeris modified should match");

    ASSERT_TRUE(prefs.language.has_value(), "Language should be present");
    ASSERT_EQ("en_US", *prefs.language, "Language should match");

    ASSERT_TRUE(prefs.units.has_value(), "Units should be present");
    ASSERT_EQ("metric", *prefs.units, "Units should match");
}

TEST(test_parse_minimal_xml_succeeds)
{
    PreferencesParser parser;
    auto prefs = parser.parseString(MINIMAL_PREFERENCES_XML);

    ASSERT_EQ("1.0", prefs.version, "Version should be 1.0");
    ASSERT_EQ("TomTom", prefs.watch_name, "Watch name should match");
    ASSERT_TRUE(!prefs.auth_token.has_value(), "Auth token should not be present");
    ASSERT_TRUE(!prefs.token_secret.has_value(), "Token secret should not be present");
    ASSERT_TRUE(!prefs.language.has_value(), "Language should not be present");
}

TEST(test_parse_from_byte_vector)
{
    PreferencesParser parser;
    std::vector<uint8_t> xml_data = toBytes(SAMPLE_PREFERENCES_XML);

    auto prefs = parser.parse(xml_data);

    ASSERT_EQ("My TomTom Watch", prefs.watch_name, "Watch name should match");
}

TEST(test_parse_empty_xml_returns_empty_fields)
{
    PreferencesParser parser;
    const std::string empty_xml = "<?xml version=\"1.0\"?><preferences></preferences>";

    auto prefs = parser.parseString(empty_xml);

    ASSERT_TRUE(prefs.watch_name.empty(), "Watch name should be empty");
    ASSERT_TRUE(prefs.version.empty(), "Version should be empty");
}

// ============================================================================
// PreferencesSerializer Tests
// ============================================================================

TEST(test_serialize_basic_preferences)
{
    PreferencesSerializer serializer;
    Preferences prefs;
    prefs.version = "1.0";
    prefs.modified = std::chrono::system_clock::time_point{std::chrono::seconds{1234567890}};
    prefs.watch_name = "Test Watch";
    prefs.ephemeris_url = "https://test.com";

    auto xml = serializer.serializeToString(prefs);

    ASSERT_TRUE(stringContains(xml, "<?xml"), "Should contain XML declaration");
    ASSERT_TRUE(stringContains(xml, "<preferences>"), "Should contain preferences tag");
    ASSERT_TRUE(stringContains(xml, "<version>1.0</version>"), "Should contain version");
    ASSERT_TRUE(stringContains(xml, "<name>Test Watch</name>"), "Should contain watch name");
    ASSERT_TRUE(stringContains(xml, "<ephemeris_url>https://test.com</ephemeris_url>"), "Should contain config URL");
}

TEST(test_serialize_with_authentication)
{
    PreferencesSerializer serializer;
    Preferences prefs;
    prefs.version = "1.0";
    prefs.modified = std::chrono::system_clock::time_point{std::chrono::seconds{1234567890}};
    prefs.watch_name = "Test";
    prefs.auth_token = "token123";
    prefs.token_secret = "secret456";
    prefs.user_id = "user789";

    auto xml = serializer.serializeToString(prefs);

    ASSERT_TRUE(stringContains(xml, "<auth_token>token123</auth_token>"), "Should contain auth token");
    ASSERT_TRUE(stringContains(xml, "<token_secret>secret456</token_secret>"), "Should contain token secret");
    ASSERT_TRUE(stringContains(xml, "<user_id>user789</user_id>"), "Should contain user ID");
}

TEST(test_serialize_with_display_settings)
{
    PreferencesSerializer serializer;
    Preferences prefs;
    prefs.version = "1.0";
    prefs.modified = std::chrono::system_clock::time_point{std::chrono::seconds{1234567890}};
    prefs.watch_name = "Test";
    prefs.language = "de_DE";
    prefs.units = "imperial";

    auto xml = serializer.serializeToString(prefs);

    ASSERT_TRUE(stringContains(xml, "<language>de_DE</language>"), "Should contain language");
    ASSERT_TRUE(stringContains(xml, "<units>imperial</units>"), "Should contain units");
}

TEST(test_serialize_to_byte_vector)
{
    PreferencesSerializer serializer;
    Preferences prefs;
    prefs.version = "1.0";
    prefs.modified = std::chrono::system_clock::time_point{std::chrono::seconds{1234567890}};
    prefs.watch_name = "Test";

    auto data = serializer.serialize(prefs);

    ASSERT_TRUE(!data.empty(), "Data should not be empty");

    std::string xml_str = fromBytes(data);
    ASSERT_TRUE(stringContains(xml_str, "<preferences>"), "Should contain preferences tag");
}

TEST(test_round_trip_preserves_data)
{
    PreferencesParser parser;
    PreferencesSerializer serializer;

    Preferences original;
    original.version = "1.0";
    original.modified = std::chrono::system_clock::time_point{std::chrono::seconds{1234567890}};
    original.watch_name = "Round Trip Test";
    original.ephemeris_url = "https://test.com";
    original.auth_token = "token123";
    original.token_secret = "secret456";
    original.language = "en_US";
    original.units = "metric";
    original.ephemeris_modified = std::chrono::system_clock::time_point{std::chrono::seconds{4103184686}};

    auto xml = serializer.serializeToString(original);
    auto parsed = parser.parseString(xml);

    ASSERT_EQ(original.version, parsed.version, "Version should match");
    ASSERT_EQ(original.watch_name, parsed.watch_name, "Watch name should match");
    ASSERT_EQ(original.ephemeris_url, parsed.ephemeris_url, "Config URL should match");
    ASSERT_EQ(*original.auth_token, *parsed.auth_token, "Auth token should match");
    ASSERT_EQ(*original.token_secret, *parsed.token_secret, "Token secret should match");
    ASSERT_EQ(*original.language, *parsed.language, "Language should match");
    ASSERT_EQ(*original.units, *parsed.units, "Units should match");
    ASSERT_EQ(original.ephemeris_modified, parsed.ephemeris_modified, "Ephemeris modified should match");
    ASSERT_EQ(original.modified, parsed.modified, "Modified timestamp should match");
}

// ============================================================================
// PreferencesService Tests (Hardware Required)
// ============================================================================
// These tests run only if a watch is connected

TEST(test_hardware_service_get_reads_preferences)
{
    ASSERT_NOT_NULL(g_watch, "Watch not connected - test skipped");

    auto prefs = g_watch->preferences().get();

    // Just verify we can read without crashing
    ASSERT_TRUE(!prefs.watch_name.empty(), "Watch name should not be empty");
    spdlog::info("Read preferences - Watch name: {}", prefs.watch_name);
}

TEST(test_hardware_service_get_watch_name)
{
    ASSERT_NOT_NULL(g_watch, "Watch not connected - test skipped");

    auto name = g_watch->preferences().getWatchName();

    ASSERT_TRUE(!name.empty(), "Watch name should not be empty");
    spdlog::info("Watch name: {}", name);
}

TEST(test_hardware_service_is_authenticated)
{
    ASSERT_NOT_NULL(g_watch, "Watch not connected - test skipped");

    bool authenticated = g_watch->preferences().isAuthenticated();

    spdlog::info("Watch authentication status: {}", authenticated ? "Yes" : "No");
    // No assertion - just checking it doesn't crash
}

TEST(test_hardware_service_preferences_roundtrip)
{
    ASSERT_NOT_NULL(g_watch, "Watch not connected - test skipped");

    // Read current preferences
    auto original = g_watch->preferences().get();
    auto original_name = original.watch_name;

    // Modify and write
    std::string test_name = "Test_" + std::to_string(std::time(nullptr));
    original.watch_name = test_name;
    g_watch->preferences().set(original);

    // Read back and verify
    auto modified = g_watch->preferences().get();
    ASSERT_EQ(test_name, modified.watch_name, "Watch name should be updated");

    // Restore original
    original.watch_name = original_name;
    g_watch->preferences().set(original);

    // Verify restoration
    auto restored = g_watch->preferences().get();
    ASSERT_EQ(original_name, restored.watch_name, "Watch name should be restored");

    spdlog::info("Roundtrip test successful - name restored to: {}", restored.watch_name);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(test_complete_workflow)
{
    PreferencesParser parser;
    PreferencesSerializer serializer;

    // Create preferences
    Preferences original;
    original.version = "1.0";
    original.modified = std::chrono::system_clock::now();
    original.watch_name = "Integration Test Watch";
    original.ephemeris_url = "https://test.com";
    original.auth_token = "test_token";
    original.token_secret = "test_secret";
    original.user_id = "test_user";
    original.language = "en_US";
    original.units = "metric";
    original.ephemeris_modified = std::chrono::system_clock::time_point{std::chrono::seconds{1234567890}};

    // Serialize
    auto xml = serializer.serializeToString(original);
    ASSERT_TRUE(!xml.empty(), "XML should not be empty");

    // Parse back
    auto parsed = parser.parseString(xml);

    // Verify round-trip
    ASSERT_EQ(original.watch_name, parsed.watch_name, "Watch name should match");
    ASSERT_EQ(original.ephemeris_url, parsed.ephemeris_url, "Config URL should match");
    ASSERT_EQ(*original.auth_token, *parsed.auth_token, "Auth token should match");
    ASSERT_EQ(*original.token_secret, *parsed.token_secret, "Token secret should match");
    ASSERT_EQ(*original.user_id, *parsed.user_id, "User ID should match");
    ASSERT_EQ(*original.language, *parsed.language, "Language should match");
    ASSERT_EQ(*original.units, *parsed.units, "Units should match");
    ASSERT_EQ(original.ephemeris_modified, parsed.ephemeris_modified, "Ephemeris modified should match");
}

// ============================================================================
// Main
// ============================================================================

int main(int /* argc */, char ** /* argv */)
{
    spdlog::set_level(spdlog::level::info);
    spdlog::info("Starting preferences tests...");

    // Try to detect and connect to watch
    bool hardware_available = false;
    try
    {
        Manager manager;
        auto watches = manager.detectWatches();

        if (!watches.empty())
        {
            spdlog::info("Detected {} watch(es) - will run hardware tests", watches.size());
            g_watch = manager.connectToWatch();

            if (g_watch)
            {
                spdlog::info("Connected to: {} - {}",
                             g_watch->getManufacturer(),
                             g_watch->getProductName());
                hardware_available = true;
            }
        }
        else
        {
            spdlog::info("No watches detected - hardware tests will be skipped");
        }
    }
    catch (const std::exception &e)
    {
        spdlog::warn("Could not connect to watch: {} - hardware tests will be skipped", e.what());
    }

    spdlog::info("==================================================");

    // Model tests
    RUN_TEST(test_has_authentication_returns_true_when_set);
    RUN_TEST(test_has_authentication_returns_false_when_missing_token);
    RUN_TEST(test_has_authentication_returns_false_when_missing_secret);
    RUN_TEST(test_has_authentication_returns_false_when_empty);
    RUN_TEST(test_has_custom_name_returns_true_when_set);
    RUN_TEST(test_has_custom_name_returns_false_when_empty);
    RUN_TEST(test_clear_authentication_removes_credentials);

    // Parser tests
    RUN_TEST(test_parse_valid_xml_succeeds);
    RUN_TEST(test_parse_minimal_xml_succeeds);
    RUN_TEST(test_parse_from_byte_vector);
    RUN_TEST(test_parse_empty_xml_returns_empty_fields);

    // Serializer tests
    RUN_TEST(test_serialize_basic_preferences);
    RUN_TEST(test_serialize_with_authentication);
    RUN_TEST(test_serialize_with_display_settings);
    RUN_TEST(test_serialize_to_byte_vector);
    RUN_TEST(test_round_trip_preserves_data);

    // Hardware-dependent service tests
    if (hardware_available)
    {
        spdlog::info("==================================================");
        spdlog::info("Running hardware-dependent tests...");
        RUN_TEST(test_hardware_service_get_reads_preferences);
        RUN_TEST(test_hardware_service_get_watch_name);
        RUN_TEST(test_hardware_service_is_authenticated);
        RUN_TEST(test_hardware_service_preferences_roundtrip);
    }
    else
    {
        spdlog::info("==================================================");
        spdlog::info("Skipping hardware-dependent tests (no watch connected)");
    }

    // Integration tests
    spdlog::info("==================================================");
    RUN_TEST(test_complete_workflow);

    // Summary
    spdlog::info("==================================================");
    spdlog::info("Test Results:");
    spdlog::info("  Passed: {}", tests_passed);
    spdlog::info("  Failed: {}", tests_failed);
    if (hardware_available)
    {
        spdlog::info("  Hardware tests: ENABLED");
    }
    else
    {
        spdlog::info("  Hardware tests: SKIPPED (no watch connected)");
    }
    spdlog::info("==================================================");

    return tests_failed > 0 ? 1 : 0;
}