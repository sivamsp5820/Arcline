#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "app/AppContext.h"

namespace sonora::ui::components {

class TransportBar : public juce::Component,
                     public app::AppContext::ProjectListener,
                     private juce::Timer {
public:
    explicit TransportBar(app::AppContext& context);
    ~TransportBar() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // AppContext::ProjectListener
    void onProjectChanged(const domain::Project* project) override;
    void onSelectionChanged() override {}
    void onTransportChanged() override;

private:
    void timerCallback() override;
    void updateDisplays();

    app::AppContext& context_;

    // Transport buttons
    juce::TextButton returnToZeroButton_{"|<"};
    juce::TextButton rewindButton_{"<<"};
    juce::TextButton forwardButton_{">>"};
    juce::TextButton cycleButton_{"↻"};
    juce::TextButton stopButton_{"■"};
    juce::TextButton playButton_{"▶"};
    juce::TextButton recordButton_{"●"};
    juce::TextButton metronomeButton_{"▲ Metronome"};

    // LCD screen component area
    juce::Rectangle<int> lcdBounds_;
    bool metronomeActive_{true};
};

} // namespace sonora::ui::components
