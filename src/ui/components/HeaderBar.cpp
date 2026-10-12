#include "ui/components/HeaderBar.h"
#include "ui/components/TransportBar.h"
#include "ui/theme/SonoraTheme.h"

namespace sonora::ui::components {

HeaderBar::HeaderBar(app::AppContext& context)
    : context_(context) {
    context_.addProjectListener(this);
    context_.getNavigation().addListener(this);

    transportBar_ = std::make_unique<TransportBar>(context_);
    addAndMakeVisible(*transportBar_);

    // Project Dropdown Button
    projectButton_.setTooltip("Project Menu");
    projectButton_.setTitle("Project Menu");
    projectButton_.setDescription("Open project management and navigation menu");
    projectButton_.onClick = [this] { showProjectMenu(); };
    addAndMakeVisible(projectButton_);

    // Tool Selector buttons
    auto setupTool = [this](juce::TextButton& btn, app::ToolType tool, const juce::String& tip) {
        btn.setTooltip(tip);
        btn.setTitle(tip);
        btn.setDescription(tip);
        btn.onClick = [this, tool] { context_.setActiveTool(tool); };
        addAndMakeVisible(btn);
    };

    setupTool(toolPointerButton_, app::ToolType::Pointer, "Pointer / Select Tool [T]");
    setupTool(toolScissorsButton_, app::ToolType::Scissors, "Scissors / Split Tool [S]");
    setupTool(toolPencilButton_, app::ToolType::Pencil, "Pencil / Draw Tool [P]");
    setupTool(toolEraserButton_, app::ToolType::Eraser, "Eraser Tool [D]");

    // Panel Toggles
    toggleInspectorButton_.setTooltip("Toggle Inspector Panel [I]");
    toggleInspectorButton_.setTitle("Inspector Panel");
    toggleInspectorButton_.setDescription("Toggle Inspector Panel visibility");
    toggleInspectorButton_.onClick = [this] { context_.toggleInspector(); };
    addAndMakeVisible(toggleInspectorButton_);

    toggleLibraryButton_.setTooltip("Toggle Library & Media Browser [Y]");
    toggleLibraryButton_.setTitle("Library & Media Browser");
    toggleLibraryButton_.setDescription("Toggle Library and Browser visibility");
    toggleLibraryButton_.onClick = [this] { context_.toggleLibrary(); };
    addAndMakeVisible(toggleLibraryButton_);

    toggleMixerButton_.setTooltip("Toggle Channel Strip Mixer [X]");
    toggleMixerButton_.setTitle("Mixer Console");
    toggleMixerButton_.setDescription("Toggle Channel Strip Mixer Console");
    toggleMixerButton_.onClick = [this] { context_.toggleLowerEditorTab(app::LowerEditorTab::Mixer); };
    addAndMakeVisible(toggleMixerButton_);

    togglePianoRollButton_.setTooltip("Toggle Piano Roll Editor [P]");
    togglePianoRollButton_.setTitle("Piano Roll Editor");
    togglePianoRollButton_.setDescription("Toggle MIDI Piano Roll Editor");
    togglePianoRollButton_.onClick = [this] { context_.toggleLowerEditorTab(app::LowerEditorTab::PianoRoll); };
    addAndMakeVisible(togglePianoRollButton_);

    toggleAudioEditorButton_.setTooltip("Toggle Audio Editor [E]");
    toggleAudioEditorButton_.setTitle("Audio Editor");
    toggleAudioEditorButton_.setDescription("Toggle Audio Waveform Sample Editor");
    toggleAudioEditorButton_.onClick = [this] { context_.toggleLowerEditorTab(app::LowerEditorTab::AudioEditor); };
    addAndMakeVisible(toggleAudioEditorButton_);

    // Master Volume Mini-Slider
    masterVolumeSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    masterVolumeSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    masterVolumeSlider_.setRange(0.0, 1.5, 0.01);
    masterVolumeSlider_.setValue(1.0);
    masterVolumeSlider_.setTooltip("Master Volume Output Level");
    masterVolumeSlider_.setTitle("Master Output Volume");
    masterVolumeSlider_.setDescription("Adjust global master output volume level");
    masterVolumeSlider_.onValueChange = [this] {
        if (auto* p = context_.getActiveProject()) {
            p->setMasterVolume(static_cast<float>(masterVolumeSlider_.getValue()));
        }
    };
    addAndMakeVisible(masterVolumeSlider_);

    // Command palette and Settings
    commandPaletteButton_.setTooltip("Quick Command Palette [Ctrl+K]");
    commandPaletteButton_.onClick = [this] {
        if (context_.onRequestCommandPalette) context_.onRequestCommandPalette();
    };
    addAndMakeVisible(commandPaletteButton_);

    preferencesButton_.setTooltip("Application Preferences [Ctrl+,]");
    preferencesButton_.onClick = [this] {
        if (context_.onRequestPreferencesDialog) context_.onRequestPreferencesDialog();
    };
    addAndMakeVisible(preferencesButton_);

    // Connect context callbacks
    context_.onToolChanged = [this](app::ToolType t) { updateToolButtons(t); };
    context_.onInspectorVisibilityChanged = [this](bool) { updatePanelButtons(); };
    context_.onLibraryVisibilityChanged = [this](bool) { updatePanelButtons(); };
    context_.onLowerEditorTabChanged = [this](app::LowerEditorTab) { updatePanelButtons(); };

    updateToolButtons(context_.getActiveTool());
    updatePanelButtons();
}

HeaderBar::~HeaderBar() {
    context_.getNavigation().removeListener(this);
    context_.removeProjectListener(this);
}

void HeaderBar::onProjectChanged(const domain::Project* project) {
    if (project) {
        projectName_ = project->getName();
        projectButton_.setButtonText(project->getName() + " ▾");
        masterVolumeSlider_.setValue(project->getMasterVolume(), juce::dontSendNotification);
        masterVolumeSlider_.setEnabled(true);
    } else {
        projectName_ = "No Project Open";
        projectButton_.setButtonText("ARCLINE ▾");
        masterVolumeSlider_.setEnabled(false);
    }
    updatePanelButtons();
    repaint();
}

void HeaderBar::onWorkspaceChanged(domain::WorkspaceType /*newType*/, domain::WorkspaceType /*oldType*/) {
    updatePanelButtons();
    repaint();
}

void HeaderBar::updateToolButtons(app::ToolType tool) {
    toolPointerButton_.setToggleState(tool == app::ToolType::Pointer, juce::dontSendNotification);
    toolScissorsButton_.setToggleState(tool == app::ToolType::Scissors, juce::dontSendNotification);
    toolPencilButton_.setToggleState(tool == app::ToolType::Pencil, juce::dontSendNotification);
    toolEraserButton_.setToggleState(tool == app::ToolType::Eraser, juce::dontSendNotification);
}

void HeaderBar::updatePanelButtons() {
    toggleInspectorButton_.setToggleState(context_.isInspectorVisible(), juce::dontSendNotification);
    toggleLibraryButton_.setToggleState(context_.isLibraryVisible(), juce::dontSendNotification);

    auto tab = context_.getLowerEditorTab();
    toggleMixerButton_.setToggleState(tab == app::LowerEditorTab::Mixer, juce::dontSendNotification);
    togglePianoRollButton_.setToggleState(tab == app::LowerEditorTab::PianoRoll, juce::dontSendNotification);
    toggleAudioEditorButton_.setToggleState(tab == app::LowerEditorTab::AudioEditor, juce::dontSendNotification);
}

void HeaderBar::showProjectMenu() {
    juce::PopupMenu menu;
    menu.addItem(1, "New Project... [Ctrl+N]");
    menu.addItem(2, "Open Project... [Ctrl+O]");
    menu.addSeparator();
    menu.addItem(3, "Save Project [Ctrl+S]", context_.hasActiveProject());
    menu.addItem(4, "Close Project [Ctrl+W]", context_.hasActiveProject());

    menu.showMenuAsync(juce::PopupMenu::Options{}.withTargetComponent(&projectButton_),
        [this](int result) {
            if (result == 1 && context_.onRequestNewProjectDialog) context_.onRequestNewProjectDialog();
            else if (result == 2 && context_.onRequestOpenProjectDialog) context_.onRequestOpenProjectDialog();
            else if (result == 3) {
                std::string err;
                context_.saveActiveProject(err);
            }
            else if (result == 4) context_.closeProject();
        });
}

void HeaderBar::paint(juce::Graphics& g) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    g.fillAll(c.panelBackground);

    // Bottom divider line
    g.setColour(c.border);
    g.drawHorizontalLine(getHeight() - 1, 0.0f, static_cast<float>(getWidth()));

    // Master volume small speaker icon / label
    auto sliderBounds = masterVolumeSlider_.getBounds();
    g.setColour(c.textSecondary);
    g.setFont(theme.getCaptionFont(10.0f));
    g.drawText("VOL", sliderBounds.getX() - 26, sliderBounds.getY(), 24, sliderBounds.getHeight(),
               juce::Justification::centredRight, false);
}

void HeaderBar::resized() {
    auto bounds = getLocalBounds().reduced(6, 6);
    int totalW = getWidth();
    bool compact = (totalW < 1150);

    // Left Section
    int projW = compact ? 110 : 130;
    projectButton_.setBounds(bounds.removeFromLeft(projW));
    bounds.removeFromLeft(6);

    toggleInspectorButton_.setBounds(bounds.removeFromLeft(26));
    bounds.removeFromLeft(2);
    toggleLibraryButton_.setBounds(bounds.removeFromLeft(compact ? 56 : 64));
    bounds.removeFromLeft(8);

    // Tool palette buttons
    toolPointerButton_.setBounds(bounds.removeFromLeft(26));
    bounds.removeFromLeft(2);
    toolScissorsButton_.setBounds(bounds.removeFromLeft(26));
    bounds.removeFromLeft(2);
    toolPencilButton_.setBounds(bounds.removeFromLeft(26));
    bounds.removeFromLeft(2);
    toolEraserButton_.setBounds(bounds.removeFromLeft(26));
    bounds.removeFromLeft(10);

    // Right Section
    preferencesButton_.setBounds(bounds.removeFromRight(26));
    bounds.removeFromRight(4);
    commandPaletteButton_.setBounds(bounds.removeFromRight(32));
    bounds.removeFromRight(8);

    masterVolumeSlider_.setBounds(bounds.removeFromRight(compact ? 60 : 75));
    bounds.removeFromRight(26); // Gap for VOL label

    int editorBtnW = compact ? 52 : 72;
    toggleAudioEditorButton_.setBounds(bounds.removeFromRight(editorBtnW));
    bounds.removeFromRight(2);
    togglePianoRollButton_.setBounds(bounds.removeFromRight(editorBtnW));
    bounds.removeFromRight(2);
    toggleMixerButton_.setBounds(bounds.removeFromRight(compact ? 46 : 54));
    bounds.removeFromRight(10);

    // Center remainder: Transport Bar
    transportBar_->setBounds(bounds);
}

} // namespace sonora::ui::components
