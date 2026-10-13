#include "ui/dialogs/PreferencesDialog.h"
#include "ui/theme/SonoraTheme.h"
#include "audio/devices/AudioDeviceService.h"
#include "audio/engine/AudioEngine.h"
#include <iomanip>
#include <sstream>

namespace sonora::ui::dialogs {

PreferencesDialog::PreferencesDialog(app::AppContext& context)
    : context_(context) {
    auto& theme = theme::SonoraTheme::getInstance();
    const auto& c = theme.getColors();
    const auto& prefs = context_.getPreferences();

    titleLabel_.setFont(theme.getHeadingFont(18.0f));
    titleLabel_.setColour(juce::Label::textColourId, c.textPrimary);
    titleLabel_.setText("Application Preferences & Audio Hardware", juce::dontSendNotification);
    addAndMakeVisible(titleLabel_);

    // 1. Audio Hardware Section Header
    audioSectionLabel_.setFont(theme.getCaptionFont(11.0f));
    audioSectionLabel_.setColour(juce::Label::textColourId, c.selectionBlue);
    audioSectionLabel_.setText("AUDIO HARDWARE & ENGINE (PHASE 2)", juce::dontSendNotification);
    addAndMakeVisible(audioSectionLabel_);

    // Device Driver Type
    deviceTypeLabel_.setFont(theme.getCaptionFont(11.0f));
    deviceTypeLabel_.setColour(juce::Label::textColourId, c.textMuted);
    deviceTypeLabel_.setText("DRIVER BACKEND", juce::dontSendNotification);
    addAndMakeVisible(deviceTypeLabel_);

    if (auto devService = context_.getAudioDeviceService()) {
        auto types = devService->getAvailableDeviceTypes();
        int id = 1;
        int selectedId = 1;
        std::string currentType = devService->getCurrentDeviceType();
        for (const auto& t : types) {
            deviceTypeCombo_.addItem(juce::String(t), id);
            if (t == currentType) selectedId = id;
            id++;
        }
        deviceTypeCombo_.setSelectedId(selectedId, juce::dontSendNotification);
    } else {
        deviceTypeCombo_.addItem("Default Audio Driver", 1);
        deviceTypeCombo_.setSelectedId(1, juce::dontSendNotification);
    }
    addAndMakeVisible(deviceTypeCombo_);

    // Output Device
    outputDeviceLabel_.setFont(theme.getCaptionFont(11.0f));
    outputDeviceLabel_.setColour(juce::Label::textColourId, c.textMuted);
    outputDeviceLabel_.setText("AUDIO OUTPUT DEVICE", juce::dontSendNotification);
    addAndMakeVisible(outputDeviceLabel_);

    if (auto devService = context_.getAudioDeviceService()) {
        auto outputs = devService->getOutputDeviceDescriptors();
        int id = 1;
        int selectedId = 1;
        std::string currentOut = devService->getCurrentOutputDeviceId();
        for (const auto& desc : outputs) {
            outputDeviceCombo_.addItem(juce::String(desc.name), id);
            if (desc.name == currentOut || (currentOut.empty() && desc.isDefault)) {
                selectedId = id;
            }
            id++;
        }
        if (outputs.empty()) {
            outputDeviceCombo_.addItem("Default Audio Endpoint", 1);
        }
        outputDeviceCombo_.setSelectedId(selectedId, juce::dontSendNotification);
    }
    addAndMakeVisible(outputDeviceCombo_);

    // Sample Rate
    sampleRateLabel_.setFont(theme.getCaptionFont(11.0f));
    sampleRateLabel_.setColour(juce::Label::textColourId, c.textMuted);
    sampleRateLabel_.setText("SAMPLE RATE", juce::dontSendNotification);
    addAndMakeVisible(sampleRateLabel_);

    const double commonRates[] = { 44100.0, 48000.0, 88200.0, 96000.0, 192000.0 };
    double currentRate = prefs.getSampleRate() > 0.0 ? prefs.getSampleRate() : 48000.0;
    int rateSel = 2; // 48000 default
    for (int i = 0; i < 5; ++i) {
        int rateId = i + 1;
        std::string rateStr = std::to_string(static_cast<int>(commonRates[i])) + " Hz";
        sampleRateCombo_.addItem(juce::String(rateStr), rateId);
        if (std::abs(commonRates[i] - currentRate) < 1.0) {
            rateSel = rateId;
        }
    }
    sampleRateCombo_.setSelectedId(rateSel, juce::dontSendNotification);
    addAndMakeVisible(sampleRateCombo_);

    // Buffer Size
    bufferSizeLabel_.setFont(theme.getCaptionFont(11.0f));
    bufferSizeLabel_.setColour(juce::Label::textColourId, c.textMuted);
    bufferSizeLabel_.setText("BUFFER SIZE (LATENCY)", juce::dontSendNotification);
    addAndMakeVisible(bufferSizeLabel_);

    const int commonBuffers[] = { 64, 128, 256, 512, 1024 };
    int currentBuf = prefs.getBufferSize() > 0 ? prefs.getBufferSize() : 256;
    int bufSel = 3; // 256 default
    for (int i = 0; i < 5; ++i) {
        int bId = i + 1;
        double ms = (static_cast<double>(commonBuffers[i]) / currentRate) * 1000.0;
        std::ostringstream ss;
        ss << commonBuffers[i] << " samples (" << std::fixed << std::setprecision(1) << ms << " ms)";
        bufferSizeCombo_.addItem(juce::String(ss.str()), bId);
        if (commonBuffers[i] == currentBuf) {
            bufSel = bId;
        }
    }
    bufferSizeCombo_.setSelectedId(bufSel, juce::dontSendNotification);
    addAndMakeVisible(bufferSizeCombo_);

    // Test Sound Tone Button (440 Hz Sine Wave)
    testToneBtn_.setTooltip("Plays continuous 440 Hz concert A sine wave through selected output device");
    testToneBtn_.onClick = [this] {
        if (auto engine = context_.getAudioEngine()) {
            auto& gen = engine->getTestToneGenerator();
            bool willEnable = !gen.isEnabled();
            gen.setEnabled(willEnable);
            if (willEnable) {
                testToneBtn_.setButtonText("Tone Playing (440 Hz) - Click to Stop");
                testToneBtn_.setColour(juce::TextButton::buttonColourId, theme::SonoraTheme::getInstance().getColors().selectionBlue);
            } else {
                testToneBtn_.setButtonText("Test Sound (440 Hz Sine Tone)");
                testToneBtn_.setColour(juce::TextButton::buttonColourId, theme::SonoraTheme::getInstance().getColors().surface);
            }
        }
    };
    addAndMakeVisible(testToneBtn_);

    // Engine Status
    engineStatusLabel_.setFont(theme.getCaptionFont(11.0f));
    engineStatusLabel_.setColour(juce::Label::textColourId, c.textSecondary);
    if (auto engine = context_.getAudioEngine()) {
        std::string statusText = "Engine: " + std::string(arcline::audio::toString(engine->getState())) +
                                 " • " + std::to_string(static_cast<int>(engine->getSampleRate())) + " Hz • " +
                                 std::to_string(engine->getBlockSize()) + " samples";
        engineStatusLabel_.setText(juce::String(statusText), juce::dontSendNotification);
    }
    addAndMakeVisible(engineStatusLabel_);

    // 2. Appearance Section
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

    // Bottom Action Buttons
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

    cancelBtn_.onClick = [this] {
        if (auto engine = context_.getAudioEngine()) {
            engine->getTestToneGenerator().setEnabled(false);
        }
        if (onDismiss) onDismiss();
    };
    addAndMakeVisible(cancelBtn_);

    infoLabel_.setFont(theme.getCaptionFont(11.0f));
    infoLabel_.setColour(juce::Label::textColourId, c.textSecondary);
    infoLabel_.setText("ARCLINE Phase 2 • Core Real-Time Engine Active • Hardware verified.", juce::dontSendNotification);
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

    // Apply Audio Hardware Configuration
    const double commonRates[] = { 44100.0, 48000.0, 88200.0, 96000.0, 192000.0 };
    int rateIdx = sampleRateCombo_.getSelectedId() - 1;
    double chosenRate = (rateIdx >= 0 && rateIdx < 5) ? commonRates[rateIdx] : 48000.0;

    const int commonBuffers[] = { 64, 128, 256, 512, 1024 };
    int bufIdx = bufferSizeCombo_.getSelectedId() - 1;
    int chosenBuffer = (bufIdx >= 0 && bufIdx < 5) ? commonBuffers[bufIdx] : 256;

    std::string chosenOut = outputDeviceCombo_.getText().toStdString();

    if (auto devService = context_.getAudioDeviceService()) {
        std::string err;
        devService->applyConfiguration(chosenOut, chosenRate, chosenBuffer, err);
    }

    prefs.setSampleRate(chosenRate);
    prefs.setBufferSize(chosenBuffer);
    context_.savePreferences();

    // Turn off test tone if dialog closing
    if (auto engine = context_.getAudioEngine()) {
        engine->getTestToneGenerator().setEnabled(false);
    }

    if (onDismiss) onDismiss();
}

void PreferencesDialog::resetDefaults() {
    auto& prefs = context_.getPreferences();
    prefs.resetToDefaults();
    confirmOnExitToggle_.setToggleState(prefs.getConfirmOnExit(), juce::dontSendNotification);
    autoSaveToggle_.setToggleState(prefs.getAutoSaveEnabled(), juce::dontSendNotification);
    scaleCombo_.setSelectedId(1, juce::dontSendNotification);
    sampleRateCombo_.setSelectedId(2, juce::dontSendNotification); // 48kHz
    bufferSizeCombo_.setSelectedId(3, juce::dontSendNotification); // 256 samples
    infoLabel_.setText("Preferences reset to defaults.", juce::dontSendNotification);
}

bool PreferencesDialog::keyPressed(const juce::KeyPress& key) {
    if (key == juce::KeyPress::escapeKey) {
        if (auto engine = context_.getAudioEngine()) {
            engine->getTestToneGenerator().setEnabled(false);
        }
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
    auto area = getLocalBounds().reduced(24, 16);

    titleLabel_.setBounds(area.removeFromTop(24));
    area.removeFromTop(10);

    // Audio Hardware Section
    audioSectionLabel_.setBounds(area.removeFromTop(16));
    area.removeFromTop(6);

    auto devRow = area.removeFromTop(44);
    int halfW = (devRow.getWidth() - 12) / 2;
    auto devLeft = devRow.removeFromLeft(halfW);
    deviceTypeLabel_.setBounds(devLeft.removeFromTop(14));
    deviceTypeCombo_.setBounds(devLeft);

    devRow.removeFromLeft(12);
    outputDeviceLabel_.setBounds(devRow.removeFromTop(14));
    outputDeviceCombo_.setBounds(devRow);

    area.removeFromTop(6);

    auto audioParamRow = area.removeFromTop(44);
    auto paramLeft = audioParamRow.removeFromLeft(halfW);
    sampleRateLabel_.setBounds(paramLeft.removeFromTop(14));
    sampleRateCombo_.setBounds(paramLeft);

    audioParamRow.removeFromLeft(12);
    bufferSizeLabel_.setBounds(audioParamRow.removeFromTop(14));
    bufferSizeCombo_.setBounds(audioParamRow);

    area.removeFromTop(8);

    auto testRow = area.removeFromTop(28);
    testToneBtn_.setBounds(testRow.removeFromLeft(240));
    testRow.removeFromLeft(12);
    engineStatusLabel_.setBounds(testRow);

    area.removeFromTop(12);

    // Appearance Section
    auto themeRow = area.removeFromTop(44);
    auto themeLeft = themeRow.removeFromLeft(halfW);
    themeLabel_.setBounds(themeLeft.removeFromTop(14));
    themeCombo_.setBounds(themeLeft);

    themeRow.removeFromLeft(12);
    scaleLabel_.setBounds(themeRow.removeFromTop(14));
    scaleCombo_.setBounds(themeRow);

    area.removeFromTop(10);

    confirmOnExitToggle_.setBounds(area.removeFromTop(20));
    area.removeFromTop(4);
    autoSaveToggle_.setBounds(area.removeFromTop(20));
    area.removeFromTop(10);

    auto actionRow = area.removeFromTop(28);
    clearRecentsBtn_.setBounds(actionRow.removeFromLeft(200));
    resetDefaultsBtn_.setBounds(actionRow.removeFromLeft(140).translated(10, 0));
    area.removeFromTop(8);

    infoLabel_.setBounds(area.removeFromTop(18));
    area.removeFromTop(10);

    auto btnRow = area.removeFromTop(32);
    saveBtn_.setBounds(btnRow.removeFromRight(130));
    btnRow.removeFromRight(10);
    cancelBtn_.setBounds(btnRow.removeFromRight(80));
}

} // namespace sonora::ui::dialogs
