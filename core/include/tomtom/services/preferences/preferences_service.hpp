#pragma once

#include "preferences_model.hpp"
#include "preferences_parser.hpp"
#include "preferences_serializer.hpp"
#include "tomtom/services/file_service.hpp"

#include <memory>

namespace tomtom::services::preferences
{

    /**
     * @brief High-level service for preferences management
     *
     * Provides convenient methods for reading and writing watch preferences.
     */
    class PreferencesService
    {
    public:
        /**
         * @brief Construct preferences service
         * @param file_service Low-level file service for watch communication
         */
        explicit PreferencesService(
            std::shared_ptr<services::FileService> file_service);

        // ====================================================================
        // Read/Write Operations
        // ====================================================================

        /**
         * @brief Read current preferences from watch
         * @return Preferences structure
         * @throws std::runtime_error if preferences cannot be read
         */
        models::Preferences get();

        /**
         * @brief Write preferences to watch
         * @param prefs Preferences to write
         * @throws std::runtime_error if preferences cannot be written
         */
        void set(const models::Preferences &prefs);

        // ====================================================================
        // Convenience Methods
        // ====================================================================

        /**
         * @brief Update just the watch name
         * @param name New watch name
         */
        void setWatchName(const std::string &name);

        /**
         * @brief Get the watch name
         * @return Current watch name
         */
        std::string getWatchName();

        /**
         * @brief Set authentication credentials
         * @param token Authentication token
         * @param secret Token secret
         * @param user_id Optional user ID
         */
        void setAuthentication(
            const std::string &token,
            const std::string &secret,
            const std::string &user_id = "");

        /**
         * @brief Clear authentication credentials
         */
        void clearAuthentication();

        /**
         * @brief Check if watch is authenticated
         * @return true if auth credentials are present
         */
        bool isAuthenticated();

        // ====================================================================
        // File Operations
        // ====================================================================

        /**
         * @brief Check if preferences file exists
         * @return true if file exists
         */
        bool exists();

        /**
         * @brief Create default preferences file
         * @param watch_name Initial watch name
         */
        void createDefault(const std::string &watch_name = "TomTom Watch");

    private:
        std::shared_ptr<services::FileService> file_service_;
        PreferencesParser parser_;
        PreferencesSerializer serializer_;
    };

} // namespace tomtom::services::preferences
