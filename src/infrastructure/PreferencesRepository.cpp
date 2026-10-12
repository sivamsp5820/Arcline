#include "infrastructure/PreferencesRepository.h"
#include "infrastructure/FileSystem.h"
#include <juce_core/juce_core.h>

namespace sonora::infrastructure {

PreferencesRepository::PreferencesRepository()
    : PreferencesRepository(FileSystem::getAppDataDirectory()) {
}

PreferencesRepository::PreferencesRepository(std::string configDirectory)
    : configDir_(std::move(configDirectory)) {
    FileSystem::createDirectories(configDir_);
    prefsFilePath_ = FileSystem::combinePath(configDir_, "preferences.json");
    recentsFilePath_ = FileSystem::combinePath(configDir_, "recent_projects.json");
}

domain::Preferences PreferencesRepository::loadPreferences() {
    domain::Preferences prefs;
    if (!FileSystem::fileExists(prefsFilePath_)) {
        return prefs;
    }

    bool success = false;
    std::string content = FileSystem::readTextFile(prefsFilePath_, success);
    if (!success || content.empty()) {
        return prefs;
    }

    auto parsed = juce::JSON::parse(juce::String(content));
    if (!parsed.isObject()) {
        return prefs;
    }

    auto* obj = parsed.getDynamicObject();
    if (!obj) return prefs;

    // Check schema version and format
    int schemaVer = 1;
    if (obj->hasProperty("schemaVersion")) {
        schemaVer = static_cast<int>(obj->getProperty("schemaVersion"));
    }
    prefs.setSchemaVersion(schemaVer);

    if (obj->hasProperty("themeMode")) {
        prefs.setThemeMode(obj->getProperty("themeMode").toString().toStdString());
    }
    if (obj->hasProperty("defaultProjectsDirectory")) {
        prefs.setDefaultProjectsDirectory(obj->getProperty("defaultProjectsDirectory").toString().toStdString());
    }
    if (obj->hasProperty("confirmOnExit")) {
        prefs.setConfirmOnExit(static_cast<bool>(obj->getProperty("confirmOnExit")));
    }
    if (obj->hasProperty("maxRecentProjects")) {
        prefs.setMaxRecentProjects(static_cast<int>(obj->getProperty("maxRecentProjects")));
    }
    if (obj->hasProperty("uiScaleFactor")) {
        prefs.setUiScaleFactor(static_cast<double>(obj->getProperty("uiScaleFactor")));
    }
    if (obj->hasProperty("autoSaveEnabled")) {
        prefs.setAutoSaveEnabled(static_cast<bool>(obj->getProperty("autoSaveEnabled")));
    }
    if (obj->hasProperty("autoSaveIntervalMinutes")) {
        prefs.setAutoSaveIntervalMinutes(static_cast<int>(obj->getProperty("autoSaveIntervalMinutes")));
    }

    // Configurable panel layout settings
    if (obj->hasProperty("inspectorWidth")) {
        prefs.setInspectorWidth(static_cast<int>(obj->getProperty("inspectorWidth")));
    }
    if (obj->hasProperty("libraryWidth")) {
        prefs.setLibraryWidth(static_cast<int>(obj->getProperty("libraryWidth")));
    }
    if (obj->hasProperty("lowerEditorHeight")) {
        prefs.setLowerEditorHeight(static_cast<int>(obj->getProperty("lowerEditorHeight")));
    }
    if (obj->hasProperty("inspectorVisible")) {
        prefs.setInspectorVisible(static_cast<bool>(obj->getProperty("inspectorVisible")));
    }
    if (obj->hasProperty("libraryVisible")) {
        prefs.setLibraryVisible(static_cast<bool>(obj->getProperty("libraryVisible")));
    }
    if (obj->hasProperty("lowerEditorTab")) {
        prefs.setLowerEditorTab(obj->getProperty("lowerEditorTab").toString().toStdString());
    }

    return prefs;
}

bool PreferencesRepository::savePreferences(const domain::Preferences& prefs) {
    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("configFormat", juce::String(domain::Preferences::kConfigFormatId));
    obj->setProperty("schemaVersion", prefs.getSchemaVersion());
    obj->setProperty("themeMode", juce::String(prefs.getThemeMode()));
    obj->setProperty("defaultProjectsDirectory", juce::String(prefs.getDefaultProjectsDirectory()));
    obj->setProperty("confirmOnExit", prefs.getConfirmOnExit());
    obj->setProperty("maxRecentProjects", prefs.getMaxRecentProjects());
    obj->setProperty("uiScaleFactor", prefs.getUiScaleFactor());
    obj->setProperty("autoSaveEnabled", prefs.getAutoSaveEnabled());
    obj->setProperty("autoSaveIntervalMinutes", prefs.getAutoSaveIntervalMinutes());

    // Configurable panel layout settings
    obj->setProperty("inspectorWidth", prefs.getInspectorWidth());
    obj->setProperty("libraryWidth", prefs.getLibraryWidth());
    obj->setProperty("lowerEditorHeight", prefs.getLowerEditorHeight());
    obj->setProperty("inspectorVisible", prefs.isInspectorVisible());
    obj->setProperty("libraryVisible", prefs.isLibraryVisible());
    obj->setProperty("lowerEditorTab", juce::String(prefs.getLowerEditorTab()));

    std::string json = juce::JSON::toString(juce::var(obj.get()), false).toStdString();
    return FileSystem::writeTextAtomic(prefsFilePath_, json);
}

domain::RecentProjectsModel PreferencesRepository::loadRecentProjects() {
    domain::RecentProjectsModel model;
    if (!FileSystem::fileExists(recentsFilePath_)) {
        return model;
    }

    bool success = false;
    std::string content = FileSystem::readTextFile(recentsFilePath_, success);
    if (!success || content.empty()) {
        return model;
    }

    auto parsed = juce::JSON::parse(juce::String(content));
    if (!parsed.isArray()) {
        return model;
    }

    auto* arr = parsed.getArray();
    if (!arr) return model;

    std::vector<domain::RecentProjectEntry> list;
    for (const auto& item : *arr) {
        if (!item.isObject()) continue;
        auto* obj = item.getDynamicObject();
        if (!obj) continue;

        domain::RecentProjectEntry entry;
        entry.name = obj->getProperty("name").toString().toStdString();
        entry.filePath = obj->getProperty("filePath").toString().toStdString();
        entry.lastOpenedTimestamp = static_cast<int64_t>(obj->getProperty("lastOpenedTimestamp"));
        entry.existsOnDisk = FileSystem::fileExists(entry.filePath);
        list.push_back(std::move(entry));
    }

    model.setEntries(std::move(list));
    return model;
}

bool PreferencesRepository::saveRecentProjects(const domain::RecentProjectsModel& recent) {
    juce::Array<juce::var> arr;
    for (const auto& entry : recent.getEntries()) {
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();
        obj->setProperty("name", juce::String(entry.name));
        obj->setProperty("filePath", juce::String(entry.filePath));
        obj->setProperty("lastOpenedTimestamp", static_cast<juce::int64>(entry.lastOpenedTimestamp));
        arr.add(juce::var(obj.get()));
    }

    std::string json = juce::JSON::toString(juce::var(arr), false).toStdString();
    return FileSystem::writeTextAtomic(recentsFilePath_, json);
}

} // namespace sonora::infrastructure
