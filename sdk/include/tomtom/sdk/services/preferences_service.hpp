#pragma once

#include <memory>

#include <tomtom/core/watch.hpp>

#include "tomtom/sdk/models/preferences_model.hpp"
#include "tomtom/sdk/parsers/preferences_parser.hpp"
#include "tomtom/sdk/serializers/preferences_serializer.hpp"

namespace tomtom::sdk::services
{
    /**
     * @brief High-level service for preferences management
     *
     * Provides methods for reading and writing watch preferences.
     */
    class PreferencesService
    {
    public:
        /**
         * @brief Construct preferences service
         * @param watch Watch instance for communication
         */
        explicit PreferencesService(std::shared_ptr<core::Watch> watch);

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
        std::shared_ptr<core::Watch> watch_;
        parsers::PreferencesParser parser_;
        serializers::PreferencesSerializer serializer_;
    };
}
