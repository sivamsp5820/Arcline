#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "app/AppContext.h"
#include "app/NavigationController.h"
#include "ui/components/HeaderBar.h"
#include "ui/components/InspectorPanel.h"
#include "ui/components/LowerEditorPanel.h"
#include "ui/components/LibraryPanel.h"
#include "ui/workspaces/WorkspaceView.h"
#include "ui/dialogs/CommandPaletteOverlay.h"
#include "ui/dialogs/NewProjectDialog.h"
#include "ui/dialogs/PreferencesDialog.h"
#include <memory>
#include <unordered_map>

namespace sonora::ui::components {

class MainLayoutComponent : public juce::Component,
                            public app::NavigationController::Listener {
public:
    explicit MainLayoutComponent(app::AppContext& context);
    ~MainLayoutComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;

    void onWorkspaceChanged(domain::WorkspaceType newType, domain::WorkspaceType oldType) override;

    void showCommandPalette();
    void showNewProjectDialog();
    void showPreferencesDialog();
    void showOpenProjectDialog();
    void hideAllDialogs();

private:
    workspaces::WorkspaceView* getOrCreateWorkspaceView(domain::WorkspaceType type);
    void updateWorkspaceView(domain::WorkspaceType type);

    app::AppContext& context_;

    std::unique_ptr<HeaderBar> headerBar_;
    std::unique_ptr<InspectorPanel> inspectorPanel_;
    std::unique_ptr<LowerEditorPanel> lowerEditorPanel_;
    std::unique_ptr<LibraryPanel> libraryPanel_;

    juce::Component workspaceContainer_;
    std::unordered_map<int, std::unique_ptr<workspaces::WorkspaceView>> workspaceViews_;
    workspaces::WorkspaceView* currentView_{nullptr};

    std::unique_ptr<dialogs::CommandPaletteOverlay> commandPalette_;
    std::unique_ptr<dialogs::NewProjectDialog> newProjectDialog_;
    std::unique_ptr<dialogs::PreferencesDialog> preferencesDialog_;

    std::unique_ptr<juce::FileChooser> fileChooser_;
};

} // namespace sonora::ui::components
