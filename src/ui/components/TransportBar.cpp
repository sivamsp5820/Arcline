#include "ui/components/TransportBar.h"
#include "ui/theme/SonoraTheme.h"
#include <iomanip>
#include <sstream>

namespace sonora::ui::components {

TransportBar::TransportBar(app::AppContext& context)
    : context_(context) {
    context_.addProjectListener(this);

    // Transport buttons setup
    auto setupButton = [this](juce::TextButton& btn, const juce::String& tip, auto onClick) {
        btn.setTooltip(tip);
        btn.setTitle(tip);
        btn.setDescription(tip);
        btn.onClick = onClick;
        addAndMakeVisible(btn);
    };

    setupButton(returnToZeroButton_, "Go to Beginning [Enter]", [this] { context_.returnToZero(); });
    setupButton(rewindButton_, "Rewind 1 Bar", [this] {
        if (auto* p = context_.getActiveProject()) {
            p->setPlayheadTick(p->getPlayheadTick() - domain::Project::kTicksPerBeat * 4);
            context_.notifyTransportChanged();
        }
    });
    setupButton(forwardButton_, "Fast Forward 1 Bar", [this] {
        if (auto* p = context_.getActiveProject()) {
            p->setPlayheadTick(p->getPlayheadTick() + domain::Project::kTicksPerBeat * 4);
            context_.notifyTransportChanged();
        }
    });
    setupButton(cycleButton_, "Cycle / Loop [C]", [this] { context_.toggleCycle(); });
    setupButton(stopButton_, "Stop [Space]", [this] { context_.stopTransport(); });
    setupButton(playButton_, "Play / Pause [Space]", [this] { context_.togglePlayPause(); });
    setupButton(recordButton_, "Record [R]", [this] { context_.toggleRecord(); });
    setupButton(metronomeButton_, "Metronome / Click", [this] {
        metronomeActive_ = !metronomeActive_;
        metronomeButton_.setToggleState(metronomeActive_, juce::dontSendNotification);
        repaint();
    });

    metronomeButton_.setToggleState(metronomeActive_, juce::dontSendNotification);

    updateDisplays();
    startTimerHz(30); // 30 FPS playback clock
}

TransportBar::~TransportBar() {
    stopTimer();
    context_.removeProjectListener(this);
}

void TransportBar::onProjectChanged(const domain::Project* /*project*/) {
    updateDisplays();
}

void TransportBar::onTransportChanged() {
    updateDisplays();
}

void TransportBar::timerCallback() {
    auto* project = context_.getActiveProject();
    if (!project || !project->isPlaying()) return;

    // Advance playhead according to tempo and 30Hz frame time
    // ticksPerSec = (bpm / 60.0) * kTicksPerBeat
    double ticksPerSec = (project->getTempoBpm() / 60.0) * domain::Project::kTicksPerBeat;
    int64_t ticksDelta = static_cast<int64_t>(ticksPerSec / 30.0);
    if (ticksDelta <= 0) ticksDelta = 16;

    int64_t current = project->getPlayheadTick();
    int64_t next = current + ticksDelta;

    if (project->isCycleActive() && project->getCycleEndTick() > project->getCycleStartTick()) {
        if (next >= project->getCycleEndTick()) {
            next = project->getCycleStartTick();
        }
    }

    project->setPlayheadTick(next);
    context_.notifyTransportChanged();
}

void TransportBar::updateDisplays() {
    auto* project = context_.getActiveProject();
    bool hasProject = (project != nullptr);

    returnToZeroButton_.setEnabled(hasProject);
    rewindButton_.setEnabled(hasProject);
    forwardButton_.setEnabled(hasProject);
    cycleButton_.setEnabled(hasProject);
    stopButton_.setEnabled(hasProject);
    playButton_.setEnabled(hasProject);
    recordButton_.setEnabled(hasProject);
    metronomeButton_.setEnabled(hasProject);

    if (project) {
        cycleButton_.setToggleState(project->isCycleActive(), juce::dontSendNotification);
        playButton_.setToggleState(project->isPlaying(), juce::dontSendNotification);
        recordButton_.setToggleState(project->isRecording(), juce::dontSendNotification);
    } else {
        cycleButton_.setToggleState(false, juce::dontSendNotification);
        playButton_.setToggleState(false, juce::dontSendNotification);
        recordButton_.setToggleState(false, juce::dontSendNotification);
    }

    repaint();
}

void TransportBar::paint(juce::Graphics& g) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();
    auto* project = context_.getActiveProject();

    // 1. Draw Logic Pro X Center LCD Display Box
    if (!lcdBounds_.isEmpty()) {
        g.setColour(c.lcdBackground);
        g.fillRoundedRectangle(lcdBounds_.toFloat(), 4.0f);

        g.setColour(c.lcdBorder);
        g.drawRoundedRectangle(lcdBounds_.toFloat(), 4.0f, 1.0f);

        if (project) {
            // Calculate Bar, Beat, Division, Tick
            int64_t totalTicks = project->getPlayheadTick();
            int64_t ticksPerBeat = domain::Project::kTicksPerBeat;
            int num = project->getTimeSigNumerator();
            int64_t ticksPerBar = ticksPerBeat * num;

            int64_t bar = (totalTicks / ticksPerBar) + 1;
            int64_t remainderInBar = totalTicks % ticksPerBar;
            int64_t beat = (remainderInBar / ticksPerBeat) + 1;
            int64_t remainderInBeat = remainderInBeat = remainderInBar % ticksPerBeat;
            int64_t div = (remainderInBeat / (ticksPerBeat / 4)) + 1;
            int64_t tick = remainderInBeat % (ticksPerBeat / 4);

            auto lcdArea = lcdBounds_.reduced(6, 4);

            // Left LCD section: Position (001 . 01 . 01 . 001)
            int posWidth = lcdArea.getWidth() * 52 / 100;
            auto posArea = lcdArea.removeFromLeft(posWidth);

            // Digital position
            char posBuf[64];
            snprintf(posBuf, sizeof(posBuf), "%03lld . %02lld . %02lld . %03lld",
                     static_cast<long long>(bar),
                     static_cast<long long>(beat),
                     static_cast<long long>(div),
                     static_cast<long long>(tick));

            g.setFont(theme.getLcdFont(15.0f));
            g.setColour(c.lcdText);
            g.drawText(posBuf, posArea.removeFromTop(posArea.getHeight() - 11), juce::Justification::centred, false);

            g.setFont(theme.getCaptionFont(9.0f));
            g.setColour(c.lcdSubtext);
            g.drawText("BAR   BEAT   DIV   TICK", posArea, juce::Justification::centred, false);

            // Subtle vertical LCD divider line
            g.setColour(c.borderSubtle);
            g.drawVerticalLine(lcdArea.getX(), static_cast<float>(lcdBounds_.getY() + 4),
                               static_cast<float>(lcdBounds_.getBottom() - 4));
            lcdArea.removeFromLeft(6);

            // Middle LCD section: Tempo BPM
            int tempoWidth = lcdArea.getWidth() * 45 / 100;
            auto tempoArea = lcdArea.removeFromLeft(tempoWidth);

            char bpmBuf[32];
            snprintf(bpmBuf, sizeof(bpmBuf), "%.4f", project->getTempoBpm());

            g.setFont(theme.getLcdFont(14.0f));
            g.setColour(c.lcdAccent);
            g.drawText(bpmBuf, tempoArea.removeFromTop(tempoArea.getHeight() - 11), juce::Justification::centred, false);

            g.setFont(theme.getCaptionFont(9.0f));
            g.setColour(c.lcdSubtext);
            g.drawText("TEMPO BPM", tempoArea, juce::Justification::centred, false);

            // Subtle vertical LCD divider
            g.setColour(c.borderSubtle);
            g.drawVerticalLine(lcdArea.getX(), static_cast<float>(lcdBounds_.getY() + 4),
                               static_cast<float>(lcdBounds_.getBottom() - 4));
            lcdArea.removeFromLeft(6);

            // Right LCD section: Signature & Key
            char sigBuf[32];
            snprintf(sigBuf, sizeof(sigBuf), "%d/%d  %s",
                     project->getTimeSigNumerator(),
                     project->getTimeSigDenominator(),
                     project->getKeySignature().c_str());

            g.setFont(theme.getLcdFont(12.5f));
            g.setColour(c.lcdText);
            g.drawText(sigBuf, lcdArea.removeFromTop(lcdArea.getHeight() - 11), juce::Justification::centred, false);

            g.setFont(theme.getCaptionFont(9.0f));
            g.setColour(c.lcdSubtext);
            g.drawText("SIGNATURE / KEY", lcdArea, juce::Justification::centred, false);
        } else {
            g.setFont(theme.getBodyFont(13.0f));
            g.setColour(c.textMuted);
            g.drawText("No Active Project Loaded", lcdBounds_, juce::Justification::centred, false);
        }
    }
}

void TransportBar::resized() {
    auto bounds = getLocalBounds().reduced(2, 6);
    int totalW = getWidth();
    bool tight = (totalW < 450);

    // Left: Transport buttons
    int btnW = tight ? 24 : 28;

    returnToZeroButton_.setBounds(bounds.removeFromLeft(btnW));
    bounds.removeFromLeft(2);
    rewindButton_.setBounds(bounds.removeFromLeft(btnW));
    bounds.removeFromLeft(2);
    forwardButton_.setBounds(bounds.removeFromLeft(btnW));
    bounds.removeFromLeft(tight ? 2 : 4);
    cycleButton_.setBounds(bounds.removeFromLeft(btnW));
    bounds.removeFromLeft(tight ? 3 : 6);
    stopButton_.setBounds(bounds.removeFromLeft(btnW));
    bounds.removeFromLeft(2);
    playButton_.setBounds(bounds.removeFromLeft(btnW + (tight ? 2 : 4)));
    bounds.removeFromLeft(2);
    recordButton_.setBounds(bounds.removeFromLeft(btnW));
    bounds.removeFromLeft(tight ? 4 : 8);

    // Right: Metronome button
    int metroW = tight ? 45 : 90;
    metronomeButton_.setBounds(bounds.removeFromRight(metroW));
    bounds.removeFromRight(tight ? 4 : 8);

    // Center remainder: Logic LCD Display
    lcdBounds_ = bounds;
}

} // namespace sonora::ui::components
