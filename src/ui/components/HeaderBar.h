#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "app/AppContext.h"
#include "app/NavigationController.h"

namespace sonora::ui::components {

class TransportBar;

class HeaderBar : public juce::Component,
                  public app::AppContext::ProjectListener,
                  public app::NavigationController::Listener {
public:
    explicit HeaderBar(app::AppContext& context);
    ~HeaderBar() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Listeners
    void onProjectChanged(const domain::Project* project) override;
    void onSelectionChanged() override {}
    void onTransportChanged() override {}
    void onWorkspaceChanged(domain::WorkspaceType newType, domain::WorkspaceType oldType) override;

private:
    void showProjectMenu();
    void updateToolButtons(app::ToolType tool);
    void updatePanelButtons();

    app::AppContext& context_;
    std::unique_ptr<TransportBar> transportBar_;

    // Left group
    juce::TextButton projectButton_{"Project ▾"};
    juce::TextButton toolPointerButton_{"↖"};
    juce::TextButton toolScissorsButton_{"✂"};
    juce::TextButton toolPencilButton_{"✎"};
    juce::TextButton toolEraserButton_{"⌫"};
    juce::TextButton toggleInspectorButton_{"i"};
    juce::TextButton toggleLibraryButton_{"Library"};

    // Right group
    juce::TextButton toggleMixerButton_{"Mixer"};
    juce::TextButton togglePianoRollButton_{"Piano Roll"};
    juce::TextButton toggleAudioEditorButton_{"Audio Editor"};
    juce::Slider masterVolumeSlider_;
    juce::TextButton commandPaletteButton_{"⌘K"};
    juce::TextButton preferencesButton_{"⚙"};

    juce::String projectName_{"No Project Open"};
};

} // namespace sonora::ui::components
