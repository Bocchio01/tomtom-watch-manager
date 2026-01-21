#include <spdlog/spdlog.h>
#include <algorithm>

#include <tomtom/core/watch.hpp>

#include "tomtom/sdk/utils/files_identifier.hpp"
#include "tomtom/sdk/services/preferences_service.hpp"

using namespace tomtom::sdk::utils;
using namespace tomtom::sdk::models;

namespace tomtom::sdk::services
{

    PreferencesService::PreferencesService(std::shared_ptr<core::Watch> watch)
        : watch_(std::move(watch)) {}

    Preferences PreferencesService::get()
    {
        spdlog::debug("Reading preferences file 0x{:08X}", PREFERENCES.value);

        auto data = watch_->files().readFile(PREFERENCES.value);
        return parser_.parse(data);
    }

    void PreferencesService::set(const Preferences &prefs)
    {
        spdlog::debug("Writing preferences file 0x{:08X}", PREFERENCES.value);

        auto data = serializer_.serialize(prefs);
        watch_->files().writeFile(PREFERENCES.value, data);
    }

    bool PreferencesService::exists()
    {
        auto files = watch_->files().listFiles();
        return std::find_if(files.begin(), files.end(),
                            [](const auto &entry)
                            { return entry.id == PREFERENCES.value; }) != files.end();
    }

}
