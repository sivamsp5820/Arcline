#pragma once

#include "domain/Preferences.h"
#include "domain/RecentProjects.h"
#include <string>

namespace sonora::infrastructure {

class PreferencesRepository {
public:
    PreferencesRepository();
    explicit PreferencesRepository(std::string configDirectory);

    domain::Preferences loadPreferences();
    bool savePreferences(const domain::Preferences& prefs);

    domain::RecentProjectsModel loadRecentProjects();
    bool saveRecentProjects(const domain::RecentProjectsModel& recent);

    const std::string& getConfigDirectory() const noexcept { return configDir_; }

private:
    std::string configDir_;
    std::string prefsFilePath_;
    std::string recentsFilePath_;
};

} // namespace sonora::infrastructure
