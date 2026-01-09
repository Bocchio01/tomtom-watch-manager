// ============================================================================
// preferences_service.cpp - High-level preferences operations implementation
// ============================================================================

#include "tomtom/files/preferences/preferences_service.hpp"
#include "tomtom/files/file_ids.hpp"
#include <chrono>
#include <spdlog/spdlog.h>

namespace tomtom::files::preferences
{

    PreferencesService::PreferencesService(std::shared_ptr<protocol::services::FileService> file_service)
        : file_service_(std::move(file_service)) {}

    models::Preferences PreferencesService::get()
    {
        spdlog::debug("Reading preferences file 0x{:08X}", PREFERENCES.value);

        auto data = file_service_->readFile(PREFERENCES);
        return parser_.parse(data);
    }

    void PreferencesService::set(const models::Preferences &prefs)
    {
        spdlog::debug("Writing preferences file 0x{:08X}", PREFERENCES.value);

        auto data = serializer_.serialize(prefs);
        file_service_->writeFile(PREFERENCES, data);
    }

    void PreferencesService::setWatchName(const std::string &name)
    {
        auto prefs = get();
        prefs.watch_name = name;
        set(prefs);

        spdlog::info("Updated watch name to '{}'", name);
    }

    std::string PreferencesService::getWatchName()
    {
        return get().watch_name;
    }

    void PreferencesService::setAuthentication(
        const std::string &token,
        const std::string &secret,
        const std::string &user_id)
    {
        auto prefs = get();
        prefs.auth_token = token;
        prefs.token_secret = secret;

        if (!user_id.empty())
        {
            prefs.user_id = user_id;
        }

        set(prefs);

        spdlog::info("Updated authentication credentials");
    }

    void PreferencesService::clearAuthentication()
    {
        auto prefs = get();
        prefs.clearAuthentication();
        set(prefs);

        spdlog::info("Cleared authentication credentials");
    }

    bool PreferencesService::isAuthenticated()
    {
        try
        {
            return get().hasAuthentication();
        }
        catch (...)
        {
            return false;
        }
    }

    bool PreferencesService::exists()
    {
        try
        {
            auto size = file_service_->getFileSize(PREFERENCES);
            return size > 0;
        }
        catch (...)
        {
            return false;
        }
    }

    void PreferencesService::createDefault(const std::string &watch_name)
    {
        models::Preferences prefs;
        prefs.version = "1.0";
        prefs.watch_name = watch_name;
        prefs.modified = std::chrono::system_clock::now();

        set(prefs);

        spdlog::info("Created default preferences with name '{}'", watch_name);
    }

} // namespace tomtom::files::preferences
