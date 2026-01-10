// ============================================================================
// preferences.hpp - Preferences domain public API
// ============================================================================
#pragma once

#include "preferences_model.hpp"
#include "preferences_parser.hpp"
#include "preferences_serializer.hpp"
#include "preferences_service.hpp"

/**
 * @brief Preferences domain namespace
 *
 * Contains all functionality related to watch preferences and settings:
 * - Preferences data model
 * - XML parsing and serialization
 * - High-level service for preferences management
 *
 * Usage example:
 * @code
 * using namespace tomtom::services::preferences;
 *
 * PreferencesService service(file_service);
 *
 * // Get current preferences
 * auto prefs = service.get();
 * std::cout << "Watch name: " << prefs.watch_name << "\n";
 *
 * // Update watch name
 * service.setWatchName("My TomTom");
 *
 * // Set authentication
 * service.setAuthentication("token", "secret");
 * @endcode
 */
namespace tomtom::services::preferences
{

    // Re-export commonly used types for convenience
    using Preferences = models::Preferences;

} // namespace tomtom::services::preferences
