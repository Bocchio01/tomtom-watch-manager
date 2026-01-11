#pragma once

#include <memory>

#include "tomtom/services/files/file_service.hpp"

#include "preferences_model.hpp"
#include "preferences_parser.hpp"
#include "preferences_serializer.hpp"

namespace tomtom::services::preferences
{

    /**
     * @brief High-level service for preferences management
     *
     * Provides low-level methods for reading and writing watch preferences.
     */
    class PreferencesService
    {
    public:
        /**
         * @brief Construct preferences service
         * @param file_service Low-level file service for watch communication
         */
        explicit PreferencesService(std::shared_ptr<services::files::FileService> file_service);

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

        /**
         * @brief Check if preferences file exists
         * @return true if file exists
         */
        bool exists();

    private:
        std::shared_ptr<services::files::FileService> file_service_;
        PreferencesParser parser_;
        PreferencesSerializer serializer_;
    };

}
