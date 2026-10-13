#pragma once

#include "domain/Project.h"
#include "domain/Preferences.h"
#include "domain/RecentProjects.h"
#include "domain/WorkspaceType.h"
#include "infrastructure/PreferencesRepository.h"
#include "app/NavigationController.h"
#include "core/CommandRegistry.h"
#include "domain/UndoHistory.h"
#include <optional>
#include <memory>
#include <vector>
#include <functional>

#include "audio/devices/AudioDeviceService.h"
#include "audio/transport/TransportController.h"
#include "audio/engine/AudioEngine.h"

namespace sonora::app {

enum class ToolType {
    Pointer,
    Scissors,
    Pencil,
    Eraser
};

enum class LowerEditorTab {
    None,
    Mixer,
    PianoRoll,
    AudioEditor
};

class AppContext {
public:
    class ProjectListener {
    public:
        virtual ~ProjectListener() = default;
        virtual void onProjectChanged(const domain::Project* project) = 0;
        virtual void onSelectionChanged() {}
        virtual void onTransportChanged() {}
    };

    AppContext();
    ~AppContext();

    bool hasActiveProject() const noexcept { return activeProject_.has_value(); }
    const domain::Project* getActiveProject() const noexcept {
        return activeProject_.has_value() ? &(*activeProject_) : nullptr;
    }
    domain::Project* getActiveProject() noexcept {
        return activeProject_.has_value() ? &(*activeProject_) : nullptr;
    }

    bool createNewProject(const std::string& name, const std::string& parentDir, std::string& outError);
    bool createNewProjectFromTemplate(const std::string& templateName, const std::string& name,
                                      const std::string& parentDir, std::string& outError);
    bool openProject(const std::string& filePath, std::string& outError);
    void closeProject();
    bool saveActiveProject(std::string& outError);

    NavigationController& getNavigation() noexcept { return navigation_; }
    core::CommandRegistry& getCommandRegistry() noexcept { return commandRegistry_; }
    domain::Preferences& getPreferences() noexcept { return preferences_; }
    domain::RecentProjectsModel& getRecentProjects() noexcept { return recentProjects_; }

    void savePreferences();
    void saveRecentProjects();

    void addProjectListener(ProjectListener* listener);
    void removeProjectListener(ProjectListener* listener);
    void notifyProjectChanged();
    void notifySelectionChanged();
    void notifyTransportChanged();

    // Tool selection
    ToolType getActiveTool() const noexcept { return activeTool_; }
    void setActiveTool(ToolType tool) noexcept;
    std::function<void(ToolType)> onToolChanged;

    // Inspector toggle
    bool isInspectorVisible() const noexcept { return isInspectorVisible_; }
    void toggleInspector() noexcept;
    std::function<void(bool)> onInspectorVisibilityChanged;

    // Library / Browser toggle
    bool isLibraryVisible() const noexcept { return isLibraryVisible_; }
    void toggleLibrary() noexcept;
    std::function<void(bool)> onLibraryVisibilityChanged;

    // Lower Panel (Mixer, Piano Roll, Audio Editor)
    LowerEditorTab getLowerEditorTab() const noexcept { return lowerEditorTab_; }
    void setLowerEditorTab(LowerEditorTab tab) noexcept;
    void toggleLowerEditorTab(LowerEditorTab tab) noexcept;
    std::function<void(LowerEditorTab)> onLowerEditorTabChanged;

    // Transport playback actions
    void togglePlayPause();
    void stopTransport();
    void toggleCycle();
    void toggleRecord();
    void returnToZero();

    // Undo / Redo
    domain::UndoHistory& getUndoHistory() noexcept { return undoHistory_; }
    bool undo();
    bool redo();
    bool canUndo() const noexcept { return undoHistory_.canUndo(); }
    bool canRedo() const noexcept { return undoHistory_.canRedo(); }

    // Dialog / overlay triggers
    std::function<void()> onRequestNewProjectDialog;
    std::function<void()> onRequestOpenProjectDialog;
    std::function<void()> onRequestPreferencesDialog;
    std::function<void()> onRequestCommandPalette;

    // Audio Subsystem (Phase 2 Real-Time Engine)
    std::shared_ptr<arcline::audio::AudioDeviceService> getAudioDeviceService() const noexcept { return audioDeviceService_; }
    std::shared_ptr<arcline::audio::TransportController> getTransportController() const noexcept { return transportController_; }
    std::shared_ptr<arcline::audio::AudioEngine> getAudioEngine() const noexcept { return audioEngine_; }

private:
    void registerDefaultCommands();

    std::optional<domain::Project> activeProject_;
    domain::Preferences preferences_;
    domain::RecentProjectsModel recentProjects_;
    infrastructure::PreferencesRepository prefsRepo_;
    NavigationController navigation_;
    core::CommandRegistry commandRegistry_;
    domain::UndoHistory undoHistory_;
    std::vector<ProjectListener*> projectListeners_;

    std::shared_ptr<arcline::audio::AudioDeviceService> audioDeviceService_;
    std::shared_ptr<arcline::audio::TransportController> transportController_;
    std::shared_ptr<arcline::audio::AudioEngine> audioEngine_;

    ToolType activeTool_{ToolType::Pointer};
    bool isInspectorVisible_{true};
    bool isLibraryVisible_{false};
    LowerEditorTab lowerEditorTab_{LowerEditorTab::Mixer};
};

} // namespace sonora::app

