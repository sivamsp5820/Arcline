#include "ui/dialogs/PreferencesDialog.h"
#include "ui/theme/SonoraTheme.h"

namespace sonora::ui::dialogs {

PreferencesDialog::PreferencesDialog(app::AppContext& context)
    : context_(context) {
    auto& theme = theme::SonoraTheme::getInstance();
    const auto& c = theme.getColors();
    const auto& prefs = context_.getPreferences();

    titleLabel_.setFont(theme.getHeadingFont(18.0f));
    titleLabel_.setColour(juce::Label::textColourId, c.textPrimary);
    titleLabel_.setText("Application Preferences", juce::dontSendNotification);
    addAndMakeVisible(titleLabel_);

    // Theme mode
    themeLabel_.setFont(theme.getCaptionFont(11.0f));
    themeLabel_.setColour(juce::Label::textColourId, c.textMuted);
    themeLabel_.setText("APPEARANCE THEME", juce::dontSendNotification);
    addAndMakeVisible(themeLabel_);

    themeCombo_.addItem("Dark Studio (Recommended)", 1);
    themeCombo_.addItem("High Contrast Dark", 2);
    themeCombo_.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(themeCombo_);

    // UI Scale
    scaleLabel_.setFont(theme.getCaptionFont(11.0f));
    scaleLabel_.setColour(juce::Label::textColourId, c.textMuted);
    scaleLabel_.setText("DISPLAY SCALING", juce::dontSendNotification);
    addAndMakeVisible(scaleLabel_);

    scaleCombo_.addItem("100% (Standard)", 1);
    scaleCombo_.addItem("125% (Medium Display)", 2);
    scaleCombo_.addItem("150% (High-DPI / 4K)", 3);
    scaleCombo_.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(scaleCombo_);

    // Toggles
    confirmOnExitToggle_.setToggleState(prefs.getConfirmOnExit(), juce::dontSendNotification);
    addAndMakeVisible(confirmOnExitToggle_);

    autoSaveToggle_.setToggleState(prefs.getAutoSaveEnabled(), juce::dontSendNotification);
    addAndMakeVisible(autoSaveToggle_);

    // Buttons
    clearRecentsBtn_.onClick = [this] {
        context_.getRecentProjects().clear();
        context_.saveRecentProjects();
        infoLabel_.setText("Recent projects list cleared successfully.", juce::dontSendNotification);
    };
    addAndMakeVisible(clearRecentsBtn_);

    resetDefaultsBtn_.onClick = [this] { resetDefaults(); };
    addAndMakeVisible(resetDefaultsBtn_);

    saveBtn_.setColour(juce::TextButton::buttonColourId, c.selectionBlue);
    saveBtn_.setColour(juce::TextButton::textColourOffId, c.textPrimary);
    saveBtn_.onClick = [this] { saveAndApply(); };
    addAndMakeVisible(saveBtn_);

    cancelBtn_.onClick = [this] { if (onDismiss) onDismiss(); };
    addAndMakeVisible(cancelBtn_);

    infoLabel_.setFont(theme.getCaptionFont(11.0f));
    infoLabel_.setColour(juce::Label::textColourId, c.textSecondary);
    infoLabel_.setText("ARCLINE v0.1.0-alpha • Shape Your Sound. • Settings saved locally.", juce::dontSendNotification);
    addAndMakeVisible(infoLabel_);
}

void PreferencesDialog::saveAndApply() {
    auto& prefs = context_.getPreferences();
    prefs.setConfirmOnExit(confirmOnExitToggle_.getToggleState());
    prefs.setAutoSaveEnabled(autoSaveToggle_.getToggleState());

    if (scaleCombo_.getSelectedId() == 2) {
        prefs.setUiScaleFactor(1.25);
    } else if (scaleCombo_.getSelectedId() == 3) {
        prefs.setUiScaleFactor(1.5);
    } else {
        prefs.setUiScaleFactor(1.0);
    }

    context_.savePreferences();
    if (onDismiss) onDismiss();
}

void PreferencesDialog::resetDefaults() {
    auto& prefs = context_.getPreferences();
    prefs.resetToDefaults();
    confirmOnExitToggle_.setToggleState(prefs.getConfirmOnExit(), juce::dontSendNotification);
    autoSaveToggle_.setToggleState(prefs.getAutoSaveEnabled(), juce::dontSendNotification);
    scaleCombo_.setSelectedId(1, juce::dontSendNotification);
    infoLabel_.setText("Preferences reset to defaults.", juce::dontSendNotification);
}

bool PreferencesDialog::keyPressed(const juce::KeyPress& key) {
    if (key == juce::KeyPress::escapeKey) {
        if (onDismiss) onDismiss();
        return true;
    }
    return false;
}

void PreferencesDialog::paint(juce::Graphics& g) {
    auto& theme = theme::SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    auto bounds = getLocalBounds().toFloat();
    g.setColour(c.surfaceElevated);
    g.fillRoundedRectangle(bounds, theme::SonoraTheme::kRadiusLarge);

    g.setColour(c.border);
    g.drawRoundedRectangle(bounds.reduced(0.5f), theme::SonoraTheme::kRadiusLarge, 1.0f);
}

void PreferencesDialog::resized() {
    auto area = getLocalBounds().reduced(24, 20);

    titleLabel_.setBounds(area.removeFromTop(28));
    area.removeFromTop(16);

    themeLabel_.setBounds(area.removeFromTop(16));
    themeCombo_.setBounds(area.removeFromTop(32));
    area.removeFromTop(12);

    scaleLabel_.setBounds(area.removeFromTop(16));
    scaleCombo_.setBounds(area.removeFromTop(32));
    area.removeFromTop(14);

    confirmOnExitToggle_.setBounds(area.removeFromTop(24));
    area.removeFromTop(8);
    autoSaveToggle_.setBounds(area.removeFromTop(24));
    area.removeFromTop(16);

    auto actionRow = area.removeFromTop(32);
    clearRecentsBtn_.setBounds(actionRow.removeFromLeft(200));
    resetDefaultsBtn_.setBounds(actionRow.removeFromLeft(140).translated(10, 0));
    area.removeFromTop(14);

    infoLabel_.setBounds(area.removeFromTop(20));
    area.removeFromTop(16);

    auto btnRow = area.removeFromTop(34);
    saveBtn_.setBounds(btnRow.removeFromRight(130));
    btnRow.removeFromRight(10);
    cancelBtn_.setBounds(btnRow.removeFromRight(80));
}

} // namespace sonora::ui::dialogs
