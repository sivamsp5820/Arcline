#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "app/AppContext.h"
#include "ui/components/InspectorPanel.h"

namespace sonora::ui::components {

class MixerConsoleComponent : public juce::Component {
public:
    explicit MixerConsoleComponent(app::AppContext& context);
    ~MixerConsoleComponent() override = default;

    void updateMixer();
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    app::AppContext& context_;
    std::vector<std::unique_ptr<ChannelStripComponent>> channelStrips_;
    std::unique_ptr<ChannelStripComponent> masterStrip_;
};

class PianoRollComponent : public juce::Component {
public:
    explicit PianoRollComponent(app::AppContext& context);
    ~PianoRollComponent() override = default;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void resized() override;

private:
    app::AppContext& context_;
    double pixelsPerTick_{0.12};
    int scrollOffsetTicks_{0};
};

class AudioEditorComponent : public juce::Component {
public:
    explicit AudioEditorComponent(app::AppContext& context);
    ~AudioEditorComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    app::AppContext& context_;
    juce::TextButton normalizeButton_{"Normalize"};
    juce::TextButton reverseButton_{"Reverse Waveform"};
};

class LowerEditorPanel : public juce::Component,
                         public app::AppContext::ProjectListener {
public:
    explicit LowerEditorPanel(app::AppContext& context);
    ~LowerEditorPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // ProjectListener
    void onProjectChanged(const domain::Project* project) override;
    void onSelectionChanged() override;
    void onTransportChanged() override {}

    void updateTabVisibility();

private:

    app::AppContext& context_;

    // Tab bar at top
    juce::TextButton tabMixerButton_{"Mixer"};
    juce::TextButton tabPianoRollButton_{"Piano Roll"};
    juce::TextButton tabAudioEditorButton_{"Audio Editor"};
    juce::TextButton closePanelButton_{"✕"};

    std::unique_ptr<MixerConsoleComponent> mixerConsole_;
    std::unique_ptr<PianoRollComponent> pianoRoll_;
    std::unique_ptr<AudioEditorComponent> audioEditor_;
};

} // namespace sonora::ui::components
