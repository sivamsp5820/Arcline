#include "domain/Preferences.h"

namespace sonora::domain {

Preferences::Preferences() {
    resetToDefaults();
}

bool Preferences::setUiScaleFactor(double scale) noexcept {
    if (scale < kMinScaleFactor || scale > kMaxScaleFactor) {
        return false;
    }
    uiScaleFactor_ = scale;
    return true;
}

void Preferences::resetToDefaults() noexcept {
    schemaVersion_ = kCurrentSchemaVersion;
    themeMode_ = "dark";
    defaultProjectsDir_.clear();
    confirmOnExit_ = true;
    maxRecentProjects_ = 10;
    uiScaleFactor_ = 1.0;
    autoSaveEnabled_ = true;
    autoSaveIntervalMinutes_ = 5;

    inspectorWidth_ = kDefaultInspectorWidth;
    libraryWidth_ = kDefaultLibraryWidth;
    lowerEditorHeight_ = kDefaultLowerEditorHeight;
    inspectorVisible_ = true;
    libraryVisible_ = false;
    lowerEditorTab_ = "mixer";
    sampleRate_ = 48000.0;
    bufferSize_ = 256;
}

bool Preferences::validate() const noexcept {
    if (themeMode_ != "dark" && themeMode_ != "light" && themeMode_ != "high_contrast") {
        return false;
    }
    if (maxRecentProjects_ <= 0 || maxRecentProjects_ > 50) {
        return false;
    }
    if (uiScaleFactor_ < kMinScaleFactor || uiScaleFactor_ > kMaxScaleFactor) {
        return false;
    }
    if (autoSaveIntervalMinutes_ <= 0 || autoSaveIntervalMinutes_ > 120) {
        return false;
    }
    if (inspectorWidth_ < kMinInspectorWidth || inspectorWidth_ > kMaxInspectorWidth) {
        return false;
    }
    if (libraryWidth_ < kMinLibraryWidth || libraryWidth_ > kMaxLibraryWidth) {
        return false;
    }
    if (lowerEditorHeight_ < kMinLowerEditorHeight || lowerEditorHeight_ > kMaxLowerEditorHeight) {
        return false;
    }
    return true;
}

} // namespace sonora::domain
