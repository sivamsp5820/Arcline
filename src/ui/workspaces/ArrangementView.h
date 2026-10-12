#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "app/AppContext.h"
#include "ui/workspaces/WorkspaceView.h"

namespace sonora::ui::workspaces {

class ArrangementRulerComponent : public juce::Component {
public:
    ArrangementRulerComponent(app::AppContext& context, double& pixelsPerTick, int& horizontalScrollOffset);
    ~ArrangementRulerComponent() override = default;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;

private:
    app::AppContext& context_;
    double& pixelsPerTick_;
    int& horizontalScrollOffset_;
    bool isDraggingCycleStart_{false};
    bool isDraggingCycleEnd_{false};
    bool isDraggingCycleBody_{false};
    int64_t dragCycleStartTick_{0};
};

class TrackHeaderListComponent : public juce::Component {
public:
    explicit TrackHeaderListComponent(app::AppContext& context);
    ~TrackHeaderListComponent() override = default;

    void updateTracks();
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    app::AppContext& context_;
    juce::TextButton addAudioTrackButton_{"+ Audio"};
    juce::TextButton addInstrumentTrackButton_{"+ Instrument"};

    struct HeaderControls {
        std::shared_ptr<domain::Track> track;
        std::unique_ptr<juce::Label> nameLabel;
        std::unique_ptr<juce::TextButton> muteButton;
        std::unique_ptr<juce::TextButton> soloButton;
        std::unique_ptr<juce::TextButton> armButton;
        std::unique_ptr<juce::Slider> volSlider;
        std::unique_ptr<juce::Slider> panKnob;
    };
    std::vector<HeaderControls> headerControls_;
};

class TimelineLanesComponent : public juce::Component {
public:
    TimelineLanesComponent(app::AppContext& context, double& pixelsPerTick,
                           int& horizontalScrollOffset, int& verticalScrollOffset);
    ~TimelineLanesComponent() override = default;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;

private:
    enum class DragMode { None, MoveRegion, TrimLeft, TrimRight };

    app::AppContext& context_;
    double& pixelsPerTick_;
    int& horizontalScrollOffset_;
    int& verticalScrollOffset_;

    DragMode dragMode_{DragMode::None};
    std::string draggingRegionId_;
    int64_t dragInitialStartTick_{0};
    int64_t dragInitialLengthTicks_{0};
    int dragInitialMouseX_{0};
};

class ArrangementView : public WorkspaceView,
                        public app::AppContext::ProjectListener,
                        public juce::ScrollBar::Listener {
public:
    explicit ArrangementView(app::AppContext& context);
    ~ArrangementView() override;

    domain::WorkspaceType getWorkspaceType() const noexcept override {
        return domain::WorkspaceType::Arrangement;
    }

    void paint(juce::Graphics& g) override;
    void resized() override;

    // ProjectListener
    void onProjectChanged(const domain::Project* project) override;
    void onSelectionChanged() override;
    void onTransportChanged() override;

    // ScrollBar::Listener
    void scrollBarMoved(juce::ScrollBar* scrollBar, double newRangeStart) override;

private:
    void updateContent();

    app::AppContext& context_;

    double pixelsPerTick_{0.08}; // Zoom: ~76.8 pixels per quarter note (960 ticks)
    int horizontalScrollOffset_{0};
    int verticalScrollOffset_{0};

    std::unique_ptr<TrackHeaderListComponent> trackHeaders_;
    std::unique_ptr<ArrangementRulerComponent> ruler_;
    std::unique_ptr<TimelineLanesComponent> timelineLanes_;

    juce::ScrollBar horizontalScrollBar_{false};
    juce::ScrollBar verticalScrollBar_{true};

    juce::Slider zoomSlider_;
};

} // namespace sonora::ui::workspaces
