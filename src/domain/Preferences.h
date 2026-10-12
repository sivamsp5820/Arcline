#pragma once

#include <string>
#include <algorithm>

namespace sonora::domain {

class Preferences {
public:
    static constexpr int kCurrentSchemaVersion = 1;
    static constexpr const char* kConfigFormatId = "ARCLINE_PREFERENCES";
    static constexpr const char* kLegacyConfigFormatId = "SONORA_PREFERENCES";

    static constexpr double kMinScaleFactor = 0.5;
    static constexpr double kMaxScaleFactor = 2.0;

    static constexpr int kMinInspectorWidth = 180;
    static constexpr int kMaxInspectorWidth = 400;
    static constexpr int kDefaultInspectorWidth = 230;

    static constexpr int kMinLibraryWidth = 180;
    static constexpr int kMaxLibraryWidth = 450;
    static constexpr int kDefaultLibraryWidth = 250;

    static constexpr int kMinLowerEditorHeight = 160;
    static constexpr int kMaxLowerEditorHeight = 600;
    static constexpr int kDefaultLowerEditorHeight = 260;

    Preferences();

    int getSchemaVersion() const noexcept { return schemaVersion_; }
    void setSchemaVersion(int ver) noexcept { schemaVersion_ = ver; }

    const std::string& getThemeMode() const noexcept { return themeMode_; }
    void setThemeMode(std::string mode) { themeMode_ = std::move(mode); }

    const std::string& getDefaultProjectsDirectory() const noexcept { return defaultProjectsDir_; }
    void setDefaultProjectsDirectory(std::string dir) { defaultProjectsDir_ = std::move(dir); }

    bool getConfirmOnExit() const noexcept { return confirmOnExit_; }
    void setConfirmOnExit(bool confirm) noexcept { confirmOnExit_ = confirm; }

    int getMaxRecentProjects() const noexcept { return maxRecentProjects_; }
    void setMaxRecentProjects(int max) noexcept { maxRecentProjects_ = max > 0 ? max : 10; }

    double getUiScaleFactor() const noexcept { return uiScaleFactor_; }
    bool setUiScaleFactor(double scale) noexcept;

    bool getAutoSaveEnabled() const noexcept { return autoSaveEnabled_; }
    void setAutoSaveEnabled(bool enabled) noexcept { autoSaveEnabled_ = enabled; }

    int getAutoSaveIntervalMinutes() const noexcept { return autoSaveIntervalMinutes_; }
    void setAutoSaveIntervalMinutes(int mins) noexcept { autoSaveIntervalMinutes_ = mins > 0 ? mins : 5; }

    // Configurable panel layout settings
    int getInspectorWidth() const noexcept { return inspectorWidth_; }
    void setInspectorWidth(int width) noexcept {
        inspectorWidth_ = std::clamp(width, kMinInspectorWidth, kMaxInspectorWidth);
    }

    int getLibraryWidth() const noexcept { return libraryWidth_; }
    void setLibraryWidth(int width) noexcept {
        libraryWidth_ = std::clamp(width, kMinLibraryWidth, kMaxLibraryWidth);
    }

    int getLowerEditorHeight() const noexcept { return lowerEditorHeight_; }
    void setLowerEditorHeight(int height) noexcept {
        lowerEditorHeight_ = std::clamp(height, kMinLowerEditorHeight, kMaxLowerEditorHeight);
    }

    bool isInspectorVisible() const noexcept { return inspectorVisible_; }
    void setInspectorVisible(bool visible) noexcept { inspectorVisible_ = visible; }

    bool isLibraryVisible() const noexcept { return libraryVisible_; }
    void setLibraryVisible(bool visible) noexcept { libraryVisible_ = visible; }

    const std::string& getLowerEditorTab() const noexcept { return lowerEditorTab_; }
    void setLowerEditorTab(std::string tab) { lowerEditorTab_ = std::move(tab); }

    void resetToDefaults() noexcept;
    bool validate() const noexcept;

private:
    int schemaVersion_{kCurrentSchemaVersion};
    std::string themeMode_{"dark"};
    std::string defaultProjectsDir_;
    bool confirmOnExit_{true};
    int maxRecentProjects_{10};
    double uiScaleFactor_{1.0};
    bool autoSaveEnabled_{true};
    int autoSaveIntervalMinutes_{5};

    int inspectorWidth_{kDefaultInspectorWidth};
    int libraryWidth_{kDefaultLibraryWidth};
    int lowerEditorHeight_{kDefaultLowerEditorHeight};
    bool inspectorVisible_{true};
    bool libraryVisible_{false};
    std::string lowerEditorTab_{"mixer"};
};

} // namespace sonora::domain
