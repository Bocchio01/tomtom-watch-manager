
#include "tomtom/services/preferences/preferences_service.hpp"
#include "tomtom/services/files/files.hpp"
#include <spdlog/spdlog.h>

namespace tomtom::services::preferences
{

    PreferencesService::PreferencesService(std::shared_ptr<services::files::FileService> file_service)
        : file_service_(std::move(file_service)) {}

    models::Preferences PreferencesService::get()
    {
        spdlog::debug("Reading preferences file 0x{:08X}", files::PREFERENCES.value);

        auto data = file_service_->readFile(files::PREFERENCES);
        return parser_.parse(data);
    }

    void PreferencesService::set(const models::Preferences &prefs)
    {
        spdlog::debug("Writing preferences file 0x{:08X}", files::PREFERENCES.value);

        auto data = serializer_.serialize(prefs);
        file_service_->writeFile(files::PREFERENCES, data);
    }

    bool PreferencesService::exists()
    {
        return file_service_->fileExists(files::PREFERENCES);
    }

}
