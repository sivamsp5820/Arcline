#include "app/AppContext.h"
#include "infrastructure/FileSystem.h"
#include "infrastructure/ProjectRepository.h"
#include "core/Logger.h"
#include <algorithm>

namespace sonora::app {

AppContext::AppContext() {
    preferences_ = prefsRepo_.loadPreferences();
    recentProjects_ = prefsRepo_.loadRecentProjects();
    recentProjects_.validatePaths([](const std::string& p) {
        return infrastructure::FileSystem::fileExists(p);
    });

    isInspectorVisible_ = preferences_.isInspectorVisible();
    isLibraryVisible_ = preferences_.isLibraryVisible();
    if (preferences_.getLowerEditorTab() == "mixer") {
        lowerEditorTab_ = LowerEditorTab::Mixer;
    } else if (preferences_.getLowerEditorTab() == "piano_roll") {
        lowerEditorTab_ = LowerEditorTab::PianoRoll;
    } else if (preferences_.getLowerEditorTab() == "audio_editor") {
        lowerEditorTab_ = LowerEditorTab::AudioEditor;
    } else if (preferences_.getLowerEditorTab() == "none") {
        lowerEditorTab_ = LowerEditorTab::None;
    }

    registerDefaultCommands();
}

AppContext::~AppContext() {
    savePreferences();
    saveRecentProjects();
}

bool AppContext::createNewProject(const std::string& name, const std::string& parentDir, std::string& outError) {
    return createNewProjectFromTemplate("Audio Recording", name, parentDir, outError);
}

bool AppContext::createNewProjectFromTemplate(const std::string& templateName, const std::string& name,
                                              const std::string& parentDir, std::string& outError) {
    if (name.empty()) {
        outError = "Project name cannot be empty.";
        return false;
    }
    if (parentDir.empty()) {
        outError = "Target directory cannot be empty.";
        return false;
    }
    if (!infrastructure::FileSystem::directoryExists(parentDir)) {
        if (!infrastructure::FileSystem::createDirectories(parentDir)) {
            outError = "Could not create target directory: " + parentDir;
            return false;
        }
    }

    std::string projectDir = infrastructure::FileSystem::combinePath(parentDir, name);
    if (!infrastructure::FileSystem::createDirectories(projectDir)) {
        outError = "Could not create project folder: " + projectDir;
        return false;
    }

    std::string projectFile = infrastructure::FileSystem::combinePath(
        projectDir, name + std::string(infrastructure::ProjectRepository::kProjectFileExtension));

    if (infrastructure::FileSystem::fileExists(projectFile)) {
        outError = "A project file already exists at this destination.";
        return false;
    }

    domain::Project project(name, projectFile);

    if (templateName == "Empty Project") {
        // Starts with single empty audio track
        project.addTrack(domain::TrackType::Audio, "Audio 1");
    } else if (templateName == "Electronic / Beat Production") {
        project.setTempoBpm(124.0);
        auto t1 = project.addTrack(domain::TrackType::SoftwareInstrument, "Drums (Electronic Kit)");
        t1->setColor(0xffb85d3b);
        auto t2 = project.addTrack(domain::TrackType::SoftwareInstrument, "Bass (Sub Synth)");
        t2->setColor(0xff705898);
        auto t3 = project.addTrack(domain::TrackType::SoftwareInstrument, "Lead (Retro Synth)");
        t3->setColor(0xff3a7d44);
        auto t4 = project.addTrack(domain::TrackType::Audio, "Audio (SFX / Riser)");
        t4->setColor(0xff2e86ab);
    } else if (templateName == "Songwriter") {
        project.setTempoBpm(105.0);
        auto t1 = project.addTrack(domain::TrackType::SoftwareInstrument, "Grand Piano");
        t1->setColor(0xff3a7d44);
        auto t2 = project.addTrack(domain::TrackType::Audio, "Acoustic Guitar");
        t2->setColor(0xffb0892a);
        auto t3 = project.addTrack(domain::TrackType::Audio, "Lead Vocal");
        t3->setColor(0xff2e86ab);
        auto t4 = project.addTrack(domain::TrackType::SoftwareInstrument, "String Ensemble");
        t4->setColor(0xff705898);
    } else {
        // Default "Audio Recording" with complete demo arrangement
        project.initializeDefaultArrangement();
    }

    if (!infrastructure::ProjectRepository::saveToFile(project, projectFile, outError)) {
        return false;
    }

    activeProject_ = project;
    undoHistory_.clear();
    recentProjects_.addOrPromote(name, projectFile);
    saveRecentProjects();

    notifyProjectChanged();
    navigation_.navigateTo(domain::WorkspaceType::Arrangement);
    core::Logger::getInstance().info("Created new project: " + name + " from template: " + templateName);
    return true;
}

bool AppContext::openProject(const std::string& filePath, std::string& outError) {
    auto loaded = infrastructure::ProjectRepository::loadFromFile(filePath, outError);
    if (!loaded) {
        core::Logger::getInstance().warn("Failed to open project: " + outError);
        return false;
    }

    activeProject_ = *loaded;
    undoHistory_.clear();
    if (activeProject_->getTracks().empty()) {
        activeProject_->initializeDefaultArrangement();
    }

    recentProjects_.addOrPromote(activeProject_->getName(), filePath);
    saveRecentProjects();

    notifyProjectChanged();
    navigation_.navigateTo(domain::WorkspaceType::Arrangement);
    core::Logger::getInstance().info("Opened project: " + activeProject_->getName());
    return true;
}

void AppContext::closeProject() {
    if (activeProject_) {
        core::Logger::getInstance().info("Closed project: " + activeProject_->getName());
        activeProject_.reset();
        undoHistory_.clear();
        notifyProjectChanged();
        navigation_.navigateTo(domain::WorkspaceType::StartStudio);
    }
}

bool AppContext::undo() {
    bool ok = undoHistory_.undo();
    if (ok) {
        notifyProjectChanged();
    }
    return ok;
}

bool AppContext::redo() {
    bool ok = undoHistory_.redo();
    if (ok) {
        notifyProjectChanged();
    }
    return ok;
}

bool AppContext::saveActiveProject(std::string& outError) {
    if (!activeProject_) {
        outError = "No active project to save.";
        return false;
    }
    return infrastructure::ProjectRepository::saveToFile(*activeProject_, activeProject_->getFilePath(), outError);
}

void AppContext::savePreferences() {
    prefsRepo_.savePreferences(preferences_);
}

void AppContext::saveRecentProjects() {
    prefsRepo_.saveRecentProjects(recentProjects_);
}

void AppContext::addProjectListener(ProjectListener* listener) {
    if (listener && std::find(projectListeners_.begin(), projectListeners_.end(), listener) == projectListeners_.end()) {
        projectListeners_.push_back(listener);
    }
}

void AppContext::removeProjectListener(ProjectListener* listener) {
    projectListeners_.erase(std::remove(projectListeners_.begin(), projectListeners_.end(), listener),
                           projectListeners_.end());
}

void AppContext::notifyProjectChanged() {
    const domain::Project* p = getActiveProject();
    for (auto* l : projectListeners_) {
        if (l) l->onProjectChanged(p);
    }
}

void AppContext::notifySelectionChanged() {
    for (auto* l : projectListeners_) {
        if (l) l->onSelectionChanged();
    }
}

void AppContext::notifyTransportChanged() {
    for (auto* l : projectListeners_) {
        if (l) l->onTransportChanged();
    }
}

void AppContext::setActiveTool(ToolType tool) noexcept {
    activeTool_ = tool;
    if (onToolChanged) onToolChanged(tool);
}

void AppContext::toggleInspector() noexcept {
    isInspectorVisible_ = !isInspectorVisible_;
    preferences_.setInspectorVisible(isInspectorVisible_);
    if (onInspectorVisibilityChanged) onInspectorVisibilityChanged(isInspectorVisible_);
}

void AppContext::toggleLibrary() noexcept {
    isLibraryVisible_ = !isLibraryVisible_;
    preferences_.setLibraryVisible(isLibraryVisible_);
    if (onLibraryVisibilityChanged) onLibraryVisibilityChanged(isLibraryVisible_);
}

void AppContext::setLowerEditorTab(LowerEditorTab tab) noexcept {
    lowerEditorTab_ = tab;
    switch (tab) {
        case LowerEditorTab::Mixer: preferences_.setLowerEditorTab("mixer"); break;
        case LowerEditorTab::PianoRoll: preferences_.setLowerEditorTab("piano_roll"); break;
        case LowerEditorTab::AudioEditor: preferences_.setLowerEditorTab("audio_editor"); break;
        case LowerEditorTab::None: preferences_.setLowerEditorTab("none"); break;
    }
    if (onLowerEditorTabChanged) onLowerEditorTabChanged(lowerEditorTab_);
}

void AppContext::toggleLowerEditorTab(LowerEditorTab tab) noexcept {
    if (lowerEditorTab_ == tab) {
        setLowerEditorTab(LowerEditorTab::None);
    } else {
        setLowerEditorTab(tab);
    }
}

void AppContext::togglePlayPause() {
    if (activeProject_) {
        bool playing = !activeProject_->isPlaying();
        activeProject_->setPlaying(playing);
        notifyTransportChanged();
    }
}

void AppContext::stopTransport() {
    if (activeProject_) {
        if (activeProject_->isPlaying()) {
            activeProject_->setPlaying(false);
        } else {
            activeProject_->setPlayheadTick(0);
        }
        activeProject_->setRecording(false);
        notifyTransportChanged();
    }
}

void AppContext::toggleCycle() {
    if (activeProject_) {
        activeProject_->setCycleActive(!activeProject_->isCycleActive());
        notifyTransportChanged();
    }
}

void AppContext::toggleRecord() {
    if (activeProject_) {
        bool rec = !activeProject_->isRecording();
        activeProject_->setRecording(rec);
        if (rec) activeProject_->setPlaying(true);
        notifyTransportChanged();
    }
}

void AppContext::returnToZero() {
    if (activeProject_) {
        activeProject_->setPlayheadTick(0);
        notifyTransportChanged();
    }
}

void AppContext::registerDefaultCommands() {
    commandRegistry_.registerCommand({
        "app.new_project", "New Project...", "Project", "Ctrl+N",
        [this] { if (onRequestNewProjectDialog) onRequestNewProjectDialog(); },
        nullptr
    });

    commandRegistry_.registerCommand({
        "app.open_project", "Open Project...", "Project", "Ctrl+O",
        [this] { if (onRequestOpenProjectDialog) onRequestOpenProjectDialog(); },
        nullptr
    });

    commandRegistry_.registerCommand({
        "app.save_project", "Save Project", "Project", "Ctrl+S",
        [this] {
            std::string err;
            saveActiveProject(err);
        },
        [this] { return hasActiveProject(); }
    });

    commandRegistry_.registerCommand({
        "app.close_project", "Close Project", "Project", "Ctrl+W",
        [this] { closeProject(); },
        [this] { return hasActiveProject(); }
    });

    commandRegistry_.registerCommand({
        "nav.arrangement", "Navigate: Arrangement", "Workspace", "Alt+1",
        [this] { navigation_.navigateTo(domain::WorkspaceType::Arrangement); },
        nullptr
    });

    commandRegistry_.registerCommand({
        "nav.start_studio", "Navigate: Start Studio", "Workspace", "Alt+2",
        [this] { navigation_.navigateTo(domain::WorkspaceType::StartStudio); },
        nullptr
    });

    commandRegistry_.registerCommand({
        "view.toggle_inspector", "Toggle Inspector Panel", "View", "I",
        [this] { toggleInspector(); },
        nullptr
    });

    commandRegistry_.registerCommand({
        "view.toggle_library", "Toggle Library Browser", "View", "Y",
        [this] { toggleLibrary(); },
        nullptr
    });

    commandRegistry_.registerCommand({
        "view.toggle_mixer", "Toggle Mixer Console", "View", "X",
        [this] { toggleLowerEditorTab(LowerEditorTab::Mixer); },
        nullptr
    });

    commandRegistry_.registerCommand({
        "view.toggle_piano_roll", "Toggle Piano Roll Editor", "View", "P",
        [this] { toggleLowerEditorTab(LowerEditorTab::PianoRoll); },
        nullptr
    });

    commandRegistry_.registerCommand({
        "view.toggle_audio_editor", "Toggle Audio Editor", "View", "E",
        [this] { toggleLowerEditorTab(LowerEditorTab::AudioEditor); },
        nullptr
    });

    commandRegistry_.registerCommand({
        "transport.play_stop", "Play / Stop", "Transport", "Space",
        [this] { togglePlayPause(); },
        [this] { return hasActiveProject(); }
    });

    commandRegistry_.registerCommand({
        "transport.return_to_zero", "Return to Beginning", "Transport", "Enter",
        [this] { returnToZero(); },
        [this] { return hasActiveProject(); }
    });

    commandRegistry_.registerCommand({
        "transport.cycle", "Toggle Cycle Mode", "Transport", "C",
        [this] { toggleCycle(); },
        [this] { return hasActiveProject(); }
    });

    commandRegistry_.registerCommand({
        "transport.record", "Toggle Record", "Transport", "R",
        [this] { toggleRecord(); },
        [this] { return hasActiveProject(); }
    });

    commandRegistry_.registerCommand({
        "edit.split_region", "Split Region at Playhead", "Edit", "Ctrl+T",
        [this] {
            if (activeProject_) {
                activeProject_->splitSelectedRegionAtPlayhead();
                notifyProjectChanged();
            }
        },
        [this] { return hasActiveProject() && activeProject_->getSelectedRegion() != nullptr; }
    });

    commandRegistry_.registerCommand({
        "edit.delete_selection", "Delete Selected", "Edit", "Delete",
        [this] {
            if (activeProject_) {
                activeProject_->deleteSelectedRegions();
                notifyProjectChanged();
            }
        },
        [this] { return hasActiveProject() && activeProject_->getSelectedRegion() != nullptr; }
    });

    commandRegistry_.registerCommand({
        "edit.duplicate_selection", "Duplicate Selected Region", "Edit", "Ctrl+D",
        [this] {
            if (activeProject_) {
                activeProject_->duplicateSelectedRegion();
                notifyProjectChanged();
            }
        },
        [this] { return hasActiveProject() && activeProject_->getSelectedRegion() != nullptr; }
    });

    commandRegistry_.registerCommand({
        "edit.undo", "Undo", "Edit", "Ctrl+Z",
        [this] { undo(); },
        [this] { return canUndo(); }
    });

    commandRegistry_.registerCommand({
        "edit.redo", "Redo", "Edit", "Ctrl+Y",
        [this] { redo(); },
        [this] { return canRedo(); }
    });

    commandRegistry_.registerCommand({
        "view.command_palette", "Open Command Palette", "View", "Ctrl+K",
        [this] { if (onRequestCommandPalette) onRequestCommandPalette(); },
        nullptr
    });

    commandRegistry_.registerCommand({
        "app.preferences", "Preferences...", "Application", "Ctrl+,",
        [this] { if (onRequestPreferencesDialog) onRequestPreferencesDialog(); },
        nullptr
    });
}

} // namespace sonora::app
