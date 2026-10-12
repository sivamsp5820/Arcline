#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "app/AppContext.h"

namespace sonora::ui::components {

class ChannelStripComponent : public juce::Component {
public:
    ChannelStripComponent(app::AppContext& context, bool isMaster);
    ~ChannelStripComponent() override = default;

    void setTrack(std::shared_ptr<domain::Track> track);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void updateControls();

    app::AppContext& context_;
    bool isMaster_{false};
    std::shared_ptr<domain::Track> track_;

    juce::Label titleLabel_;
    juce::TextButton muteButton_{"M"};
    juce::TextButton soloButton_{"S"};
    juce::TextButton armButton_{"R"};
    juce::Slider panKnob_;
    juce::Slider faderSlider_;
    juce::Label dbReadout_;

    // Insert effect slot buttons
    juce::TextButton insertSlots_[4];
    juce::TextButton sendSlots_[2];
    juce::Slider sendKnobs_[2];
};

class InspectorPanel : public juce::Component,
                       public app::AppContext::ProjectListener {
public:
    explicit InspectorPanel(app::AppContext& context);
    ~InspectorPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // AppContext::ProjectListener
    void onProjectChanged(const domain::Project* project) override;
    void onSelectionChanged() override;
    void onTransportChanged() override {}

private:
    void updateInspector();

    app::AppContext& context_;

    // Top: Region & Track Parameters box
    juce::Label headerLabel_{"INSPECTOR", "INSPECTOR"};
    juce::Label regionTitleLabel_{"Region: None", "Region: None"};
    juce::ToggleButton muteRegionToggle_{"Mute"};
    juce::ToggleButton loopRegionToggle_{"Loop"};
    juce::ComboBox quantizeCombo_;
    juce::Slider gainSlider_;
    juce::Label gainLabel_{"Gain", "Gain"};

    // Bottom: Dual Channel Strips (Selected Track + Master)
    std::unique_ptr<ChannelStripComponent> selectedTrackStrip_;
    std::unique_ptr<ChannelStripComponent> masterStrip_;
};

} // namespace sonora::ui::components
