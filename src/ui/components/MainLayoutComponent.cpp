#include "ui/components/MainLayoutComponent.h"
#include "ui/theme/SonoraTheme.h"
#include "ui/workspaces/StartStudioView.h"
#include "ui/workspaces/ArrangementView.h"
#include "infrastructure/ProjectRepository.h"
#include "infrastructure/FileSystem.h"

namespace sonora::ui::components {

MainLayoutComponent::MainLayoutComponent(app::AppContext& context)
    : context_(context) {
    setWantsKeyboardFocus(true);

    headerBar_ = std::make_unique<HeaderBar>(context_);
    addAndMakeVisible(*headerBar_);

    inspectorPanel_ = std::make_unique<InspectorPanel>(context_);
    addAndMakeVisible(*inspectorPanel_);

    lowerEditorPanel_ = std::make_unique<LowerEditorPanel>(context_);
    addAndMakeVisible(*lowerEditorPanel_);

    libraryPanel_ = std::make_unique<LibraryPanel>(context_);
    addAndMakeVisible(*libraryPanel_);

    addAndMakeVisible(workspaceContainer_);

    // Wire up context dialog requests
    context_.onRequestCommandPalette = [this] { showCommandPalette(); };
    context_.onRequestNewProjectDialog = [this] { showNewProjectDialog(); };
    context_.onRequestPreferencesDialog = [this] { showPreferencesDialog(); };
    context_.onRequestOpenProjectDialog = [this] { showOpenProjectDialog(); };

    // Dockable panel visibility change callbacks
    context_.onInspectorVisibilityChanged = [this](bool) { resized(); };
    context_.onLowerEditorTabChanged = [this](app::LowerEditorTab) {
        if (lowerEditorPanel_) lowerEditorPanel_->updateTabVisibility();
        resized();
    };

    context_.getNavigation().addListener(this);

    // Initial view
    updateWorkspaceView(context_.getNavigation().getCurrentWorkspace());
}

MainLayoutComponent::~MainLayoutComponent() {
    context_.getNavigation().removeListener(this);
}

workspaces::WorkspaceView* MainLayoutComponent::getOrCreateWorkspaceView(domain::WorkspaceType type) {
    int key = static_cast<int>(type);
    auto it = workspaceViews_.find(key);
    if (it != workspaceViews_.end()) {
        return it->second.get();
    }

    std::unique_ptr<workspaces::WorkspaceView> view;
    if (type == domain::WorkspaceType::StartStudio) {
        view = std::make_unique<workspaces::StartStudioView>(context_);
    } else {
        // Arrangement workspace
        view = std::make_unique<workspaces::ArrangementView>(context_);
    }

    auto* ptr = view.get();
    workspaceViews_[key] = std::move(view);
    return ptr;
}

void MainLayoutComponent::updateWorkspaceView(domain::WorkspaceType type) {
    if (currentView_) {
        currentView_->onDeactivated();
        currentView_->setVisible(false);
    }

    currentView_ = getOrCreateWorkspaceView(type);
    if (currentView_) {
        if (currentView_->getParentComponent() != &workspaceContainer_) {
            workspaceContainer_.addAndMakeVisible(currentView_);
        }
        currentView_->setVisible(true);
        currentView_->setBounds(workspaceContainer_.getLocalBounds());
        currentView_->onActivated();
    }

    resized();
}

void MainLayoutComponent::onWorkspaceChanged(domain::WorkspaceType newType, domain::WorkspaceType /*oldType*/) {
    updateWorkspaceView(newType);
}

void MainLayoutComponent::showCommandPalette() {
    if (!commandPalette_) {
        commandPalette_ = std::make_unique<dialogs::CommandPaletteOverlay>(context_);
        commandPalette_->onDismiss = [this] {
            commandPalette_->setVisible(false);
            grabKeyboardFocus();
        };
        addChildComponent(*commandPalette_);
    }
    commandPalette_->setBounds(getLocalBounds());
    commandPalette_->setVisible(true);
    commandPalette_->toFront(true);
}

void MainLayoutComponent::showNewProjectDialog() {
    if (!newProjectDialog_) {
        newProjectDialog_ = std::make_unique<dialogs::NewProjectDialog>(context_);
        newProjectDialog_->onDismiss = [this] {
            newProjectDialog_->setVisible(false);
            grabKeyboardFocus();
        };
        addChildComponent(*newProjectDialog_);
    }

    int dlgW = 460;
    int dlgH = 340;
    int dlgX = (getWidth() - dlgW) / 2;
    int dlgY = (getHeight() - dlgH) / 2;
    newProjectDialog_->setBounds(dlgX, dlgY, dlgW, dlgH);
    newProjectDialog_->setVisible(true);
    newProjectDialog_->toFront(true);
}

void MainLayoutComponent::showPreferencesDialog() {
    if (!preferencesDialog_) {
        preferencesDialog_ = std::make_unique<dialogs::PreferencesDialog>(context_);
        preferencesDialog_->onDismiss = [this] {
            preferencesDialog_->setVisible(false);
            grabKeyboardFocus();
        };
        addChildComponent(*preferencesDialog_);
    }

    int dlgW = 480;
    int dlgH = 380;
    int dlgX = (getWidth() - dlgW) / 2;
    int dlgY = (getHeight() - dlgH) / 2;
    preferencesDialog_->setBounds(dlgX, dlgY, dlgW, dlgH);
    preferencesDialog_->setVisible(true);
    preferencesDialog_->toFront(true);
}

void MainLayoutComponent::hideAllDialogs() {
    if (commandPalette_) commandPalette_->setVisible(false);
    if (newProjectDialog_) newProjectDialog_->setVisible(false);
    if (preferencesDialog_) preferencesDialog_->setVisible(false);
}

void MainLayoutComponent::showOpenProjectDialog() {
    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Open ARCLINE Project",
        juce::File(infrastructure::FileSystem::getDefaultProjectsDirectory()),
        "*.arcline;*.sonora");

    auto fileFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    fileChooser_->launchAsync(fileFlags, [this](const juce::FileChooser& chooser) {
        auto result = chooser.getResult();
        if (result.existsAsFile()) {
            std::string err;
            if (!context_.openProject(result.getFullPathName().toStdString(), err)) {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "Error Opening Project",
                    juce::String(err));
            }
        }
    });
}

bool MainLayoutComponent::keyPressed(const juce::KeyPress& key) {
    // Escape dismisses any open dialog
    if (key == juce::KeyPress::escapeKey) {
        if (commandPalette_ && commandPalette_->isVisible()) {
            commandPalette_->setVisible(false);
            return true;
        }
        if (newProjectDialog_ && newProjectDialog_->isVisible()) {
            newProjectDialog_->setVisible(false);
            return true;
        }
        if (preferencesDialog_ && preferencesDialog_->isVisible()) {
            preferencesDialog_->setVisible(false);
            return true;
        }
    }

    // Command palette: Ctrl+K / Cmd+K
    if (key == juce::KeyPress('k', juce::ModifierKeys::commandModifier, 0) ||
        key == juce::KeyPress('k', juce::ModifierKeys::ctrlModifier, 0)) {
        showCommandPalette();
        return true;
    }

    // New Project: Ctrl+N / Cmd+N
    if (key == juce::KeyPress('n', juce::ModifierKeys::commandModifier, 0) ||
        key == juce::KeyPress('n', juce::ModifierKeys::ctrlModifier, 0)) {
        showNewProjectDialog();
        return true;
    }

    // Open Project: Ctrl+O / Cmd+O
    if (key == juce::KeyPress('o', juce::ModifierKeys::commandModifier, 0) ||
        key == juce::KeyPress('o', juce::ModifierKeys::ctrlModifier, 0)) {
        showOpenProjectDialog();
        return true;
    }

    // Save Project: Ctrl+S / Cmd+S
    if (key == juce::KeyPress('s', juce::ModifierKeys::commandModifier, 0) ||
        key == juce::KeyPress('s', juce::ModifierKeys::ctrlModifier, 0)) {
        std::string err;
        context_.saveActiveProject(err);
        return true;
    }

    // Undo: Ctrl+Z / Cmd+Z
    if (key == juce::KeyPress('z', juce::ModifierKeys::commandModifier, 0) ||
        key == juce::KeyPress('z', juce::ModifierKeys::ctrlModifier, 0)) {
        context_.undo();
        return true;
    }

    // Redo: Ctrl+Shift+Z / Cmd+Shift+Z or Ctrl+Y
    if (key == juce::KeyPress('z', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier, 0) ||
        key == juce::KeyPress('z', juce::ModifierKeys::ctrlModifier | juce::ModifierKeys::shiftModifier, 0) ||
        key == juce::KeyPress('y', juce::ModifierKeys::ctrlModifier, 0)) {
        context_.redo();
        return true;
    }

    // Toggle Inspector: I / Cmd+I
    if (key == juce::KeyPress('i') ||
        key == juce::KeyPress('i', juce::ModifierKeys::commandModifier, 0) ||
        key == juce::KeyPress('i', juce::ModifierKeys::ctrlModifier, 0)) {
        context_.toggleInspector();
        return true;
    }

    // Toggle Library: Y / Cmd+Y
    if (key == juce::KeyPress('y') ||
        key == juce::KeyPress('y', juce::ModifierKeys::commandModifier, 0) ||
        key == juce::KeyPress('y', juce::ModifierKeys::ctrlModifier, 0)) {
        context_.toggleLibrary();
        return true;
    }

    // Toggle Mixer: X
    if (key == juce::KeyPress('x')) {
        context_.toggleLowerEditorTab(app::LowerEditorTab::Mixer);
        return true;
    }

    // Toggle Piano Roll: P
    if (key == juce::KeyPress('p')) {
        context_.toggleLowerEditorTab(app::LowerEditorTab::PianoRoll);
        return true;
    }

    // Toggle Audio Editor: E
    if (key == juce::KeyPress('e')) {
        context_.toggleLowerEditorTab(app::LowerEditorTab::AudioEditor);
        return true;
    }

    // Play / Stop: Space
    if (key == juce::KeyPress::spaceKey) {
        context_.togglePlayPause();
        return true;
    }

    // Return to zero: Enter
    if (key == juce::KeyPress::returnKey) {
        context_.returnToZero();
        return true;
    }

    // Cycle: C
    if (key == juce::KeyPress('c')) {
        context_.toggleCycle();
        return true;
    }

    // Record: R
    if (key == juce::KeyPress('r')) {
        context_.toggleRecord();
        return true;
    }

    // Split Region at Playhead: Ctrl+T / Cmd+T
    if (key == juce::KeyPress('t', juce::ModifierKeys::commandModifier, 0) ||
        key == juce::KeyPress('t', juce::ModifierKeys::ctrlModifier, 0)) {
        if (auto* p = context_.getActiveProject()) {
            p->splitSelectedRegionAtPlayhead();
            context_.notifyProjectChanged();
        }
        return true;
    }

    // Duplicate Region: Ctrl+D / Cmd+D
    if (key == juce::KeyPress('d', juce::ModifierKeys::commandModifier, 0) ||
        key == juce::KeyPress('d', juce::ModifierKeys::ctrlModifier, 0)) {
        if (auto* p = context_.getActiveProject()) {
            p->duplicateSelectedRegion();
            context_.notifyProjectChanged();
        }
        return true;
    }

    // Delete Region: Delete / Backspace
    if (key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey) {
        if (auto* p = context_.getActiveProject()) {
            p->deleteSelectedRegions();
            context_.notifyProjectChanged();
        }
        return true;
    }

    // Preferences: Ctrl+, / Cmd+,
    if (key == juce::KeyPress(',', juce::ModifierKeys::commandModifier, 0) ||
        key == juce::KeyPress(',', juce::ModifierKeys::ctrlModifier, 0)) {
        showPreferencesDialog();
        return true;
    }

    return false;
}

void MainLayoutComponent::paint(juce::Graphics& g) {
    auto& theme = theme::SonoraTheme::getInstance();
    g.fillAll(theme.getColors().background);
}

void MainLayoutComponent::resized() {
    auto area = getLocalBounds();

    // 1. Top Header Bar (Logic Pro X Control Bar, height 50px)
    int headerH = 50;
    headerBar_->setBounds(area.removeFromTop(headerH));

    bool isArrangement = (context_.getNavigation().getCurrentWorkspace() == domain::WorkspaceType::Arrangement);

    // Dockable panels only active when in Arrangement workspace
    if (isArrangement) {
        // 2. Left Inspector Panel (Dual Channel Strip + Region Parameters)
        bool showInspector = context_.isInspectorVisible();
        inspectorPanel_->setVisible(showInspector);
        if (showInspector) {
            int maxAllowedW = std::max(180, area.getWidth() / 3);
            int inspectorW = std::min(context_.getPreferences().getInspectorWidth(), maxAllowedW);
            inspectorPanel_->setBounds(area.removeFromLeft(inspectorW));
        }

        // 3. Right Library / Media Browser Panel
        bool showLibrary = context_.isLibraryVisible();
        libraryPanel_->setVisible(showLibrary);
        if (showLibrary) {
            int maxAllowedW = std::max(180, area.getWidth() / 3);
            int libraryW = std::min(context_.getPreferences().getLibraryWidth(), maxAllowedW);
            libraryPanel_->setBounds(area.removeFromRight(libraryW));
        }

        // 4. Bottom Lower Editor Panel (Mixer Console / Piano Roll / Audio Editor)
        bool showLower = (context_.getLowerEditorTab() != app::LowerEditorTab::None);
        lowerEditorPanel_->setVisible(showLower);
        if (showLower) {
            int maxAllowedH = std::max(150, area.getHeight() * 3 / 5);
            int lowerH = std::min(context_.getPreferences().getLowerEditorHeight(), maxAllowedH);
            lowerEditorPanel_->setBounds(area.removeFromBottom(lowerH));
        }
    } else {
        inspectorPanel_->setVisible(false);
        libraryPanel_->setVisible(false);
        lowerEditorPanel_->setVisible(false);
    }

    // 5. Center: Main Arrangement or Start Studio
    workspaceContainer_.setBounds(area);
    if (currentView_) {
        currentView_->setBounds(workspaceContainer_.getLocalBounds());
    }

    // Dialog Overlays
    if (commandPalette_ && commandPalette_->isVisible()) {
        commandPalette_->setBounds(getLocalBounds());
    }

    if (newProjectDialog_ && newProjectDialog_->isVisible()) {
        int dlgW = 460;
        int dlgH = 340;
        int dlgX = (getWidth() - dlgW) / 2;
        int dlgY = (getHeight() - dlgH) / 2;
        newProjectDialog_->setBounds(dlgX, dlgY, dlgW, dlgH);
    }

    if (preferencesDialog_ && preferencesDialog_->isVisible()) {
        int dlgW = 480;
        int dlgH = 380;
        int dlgX = (getWidth() - dlgW) / 2;
        int dlgY = (getHeight() - dlgH) / 2;
        preferencesDialog_->setBounds(dlgX, dlgY, dlgW, dlgH);
    }
}

} // namespace sonora::ui::components
