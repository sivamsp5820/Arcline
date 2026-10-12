#include "ui/components/LowerEditorPanel.h"
#include "ui/theme/SonoraTheme.h"
#include <cmath>

namespace sonora::ui::components {

// ============================================================================
// MixerConsoleComponent
// ============================================================================

MixerConsoleComponent::MixerConsoleComponent(app::AppContext& context)
    : context_(context) {
    masterStrip_ = std::make_unique<ChannelStripComponent>(context_, true);
    addAndMakeVisible(*masterStrip_);
    updateMixer();
}

void MixerConsoleComponent::updateMixer() {
    channelStrips_.clear();
    auto* project = context_.getActiveProject();
    if (!project) return;

    for (const auto& track : project->getTracks()) {
        if (!track) continue;
        auto strip = std::make_unique<ChannelStripComponent>(context_, false);
        strip->setTrack(track);
        addAndMakeVisible(*strip);
        channelStrips_.push_back(std::move(strip));
    }
    masterStrip_->setTrack(nullptr);
    resized();
    repaint();
}

void MixerConsoleComponent::paint(juce::Graphics& g) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();
    g.fillAll(c.panelBackground);

    // Separator line before master strip
    if (masterStrip_ && masterStrip_->isVisible()) {
        int masterX = masterStrip_->getX() - 4;
        g.setColour(c.border);
        g.drawVerticalLine(masterX, 4.0f, static_cast<float>(getHeight() - 4));
    }
}

void MixerConsoleComponent::resized() {
    auto bounds = getLocalBounds().reduced(6, 6);
    int stripW = 86;

    // Master strip on far right
    if (masterStrip_) {
        masterStrip_->setBounds(bounds.removeFromRight(stripW));
        bounds.removeFromRight(8); // Gap
    }

    // Track channel strips
    for (auto& strip : channelStrips_) {
        if (strip) {
            strip->setBounds(bounds.removeFromLeft(stripW));
            bounds.removeFromLeft(4);
        }
    }
}

// ============================================================================
// PianoRollComponent
// ============================================================================

PianoRollComponent::PianoRollComponent(app::AppContext& context)
    : context_(context) {}

void PianoRollComponent::paint(juce::Graphics& g) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();
    auto* project = context_.getActiveProject();

    g.fillAll(c.timelineBackground);

    int keyWidth = 54;
    int noteRowHeight = 12;
    int minPitch = 36; // C2
    int maxPitch = 84; // C6
    int totalPitches = maxPitch - minPitch + 1;

    // 1. Draw piano keys on left
    g.setColour(c.surfaceElevated);
    g.fillRect(0, 0, keyWidth, getHeight());
    g.setColour(c.border);
    g.drawVerticalLine(keyWidth - 1, 0.0f, static_cast<float>(getHeight()));

    for (int p = maxPitch; p >= minPitch; --p) {
        int rowY = (maxPitch - p) * noteRowHeight;
        bool isBlackKey = (p % 12 == 1 || p % 12 == 3 || p % 12 == 6 || p % 12 == 8 || p % 12 == 10);

        // Key on left
        g.setColour(isBlackKey ? juce::Colour(0xff181a1f) : juce::Colour(0xff2d3138));
        g.fillRect(0, rowY, keyWidth - 1, noteRowHeight);
        g.setColour(c.borderSubtle);
        g.drawHorizontalLine(rowY + noteRowHeight - 1, 0.0f, static_cast<float>(keyWidth - 1));

        // Pitch label on C keys
        if (p % 12 == 0) {
            g.setFont(theme.getCaptionFont(9.0f));
            g.setColour(c.textSecondary);
            int octave = (p / 12) - 1;
            g.drawText("C" + juce::String(octave), 4, rowY, 30, noteRowHeight, juce::Justification::centredLeft, false);
        }

        // Grid row on right
        g.setColour(isBlackKey ? juce::Colour(0xff1b1d22) : juce::Colour(0xff22252b));
        g.fillRect(keyWidth, rowY, getWidth() - keyWidth, noteRowHeight);
        g.setColour(c.borderSubtle.withAlpha(0.4f));
        g.drawHorizontalLine(rowY + noteRowHeight - 1, static_cast<float>(keyWidth), static_cast<float>(getWidth()));
    }

    if (!project) return;

    // 2. Draw vertical beat grid lines
    int64_t ticksPerBeat = domain::Project::kTicksPerBeat;
    for (int b = 0; b < 100; ++b) {
        float beatX = static_cast<float>(keyWidth + b * ticksPerBeat * pixelsPerTick_);
        if (beatX >= keyWidth && beatX <= getWidth()) {
            g.setColour((b % 4 == 0) ? c.border : c.borderSubtle.withAlpha(0.5f));
            g.drawVerticalLine(static_cast<int>(beatX), 0.0f, static_cast<float>(totalPitches * noteRowHeight));
        }
    }

    // 3. Draw MIDI notes for selected region or first MIDI region
    std::shared_ptr<domain::Region> midiRegion = project->getSelectedRegion();
    if (!midiRegion || midiRegion->getType() != domain::RegionType::Midi) {
        for (const auto& t : project->getTracks()) {
            if (!t) continue;
            for (const auto& r : t->getRegions()) {
                if (r && r->getType() == domain::RegionType::Midi) {
                    midiRegion = r;
                    break;
                }
            }
            if (midiRegion) break;
        }
    }

    if (midiRegion) {
        for (const auto& note : midiRegion->getNotes()) {
            if (note.pitch < minPitch || note.pitch > maxPitch) continue;

            int rowY = (maxPitch - note.pitch) * noteRowHeight;
            float noteX = static_cast<float>(keyWidth + note.startTick * pixelsPerTick_);
            float noteW = static_cast<float>(note.lengthTicks * pixelsPerTick_);
            if (noteW < 4.0f) noteW = 4.0f;

            juce::Rectangle<float> noteBounds(noteX, static_cast<float>(rowY + 1), noteW, static_cast<float>(noteRowHeight - 2));

            // Green MIDI note block
            g.setColour(c.trackMidiGreen.brighter(0.2f));
            g.fillRoundedRectangle(noteBounds, 2.0f);
            g.setColour(juce::Colours::white);
            g.drawRoundedRectangle(noteBounds, 2.0f, 1.0f);
        }
    }
}

void PianoRollComponent::mouseDown(const juce::MouseEvent& e) {
    int keyWidth = 54;
    int noteRowHeight = 12;
    int minPitch = 36;
    int maxPitch = 84;

    if (e.x < keyWidth) return;

    auto* project = context_.getActiveProject();
    if (!project) return;

    auto region = project->getSelectedRegion();
    if (!region || region->getType() != domain::RegionType::Midi) return;

    int pitch = maxPitch - (e.y / noteRowHeight);
    if (pitch < minPitch || pitch > maxPitch) return;

    int64_t tick = static_cast<int64_t>((e.x - keyWidth) / pixelsPerTick_);
    int64_t quantTick = (tick / 480) * 480; // snap to 8th note

    region->addNote({pitch, quantTick, 480, 100});
    context_.notifyProjectChanged();
    repaint();
}

void PianoRollComponent::resized() {}

// ============================================================================
// AudioEditorComponent
// ============================================================================

AudioEditorComponent::AudioEditorComponent(app::AppContext& context)
    : context_(context) {
    normalizeButton_.setTooltip("Normalize Audio Peak to 0.0 dB");
    normalizeButton_.onClick = [this] {
        if (auto* p = context_.getActiveProject()) {
            if (auto r = p->getSelectedRegion()) {
                r->setGainDb(0.0f);
                context_.notifyProjectChanged();
            }
        }
    };
    addAndMakeVisible(normalizeButton_);

    reverseButton_.setTooltip("Reverse Audio Region Samples");
    reverseButton_.onClick = [this] {
        // Toggle reverse flag or mirror waveform
        if (auto* p = context_.getActiveProject()) {
            if (auto r = p->getSelectedRegion()) {
                auto peaks = r->getWaveformPeaks();
                std::reverse(peaks.begin(), peaks.end());
                r->setWaveformPeaks(std::move(peaks));
                context_.notifyProjectChanged();
            }
        }
    };
    addAndMakeVisible(reverseButton_);
}

void AudioEditorComponent::paint(juce::Graphics& g) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();
    auto* project = context_.getActiveProject();

    g.fillAll(c.timelineBackground);

    // Header info strip
    g.setColour(c.panelBackground);
    g.fillRect(0, 0, getWidth(), 28);
    g.setColour(c.border);
    g.drawHorizontalLine(27, 0.0f, static_cast<float>(getWidth()));

    std::shared_ptr<domain::Region> audioRegion = project ? project->getSelectedRegion() : nullptr;
    if (!audioRegion || audioRegion->getType() != domain::RegionType::Audio) {
        if (project) {
            for (const auto& t : project->getTracks()) {
                if (!t) continue;
                for (const auto& r : t->getRegions()) {
                    if (r && r->getType() == domain::RegionType::Audio) {
                        audioRegion = r;
                        break;
                    }
                }
                if (audioRegion) break;
            }
        }
    }

    g.setFont(theme.getLabelFont(11.5f));
    g.setColour(c.textPrimary);
    if (audioRegion) {
        g.drawText(juce::String("Audio Region: ") + audioRegion->getName() + " | Start: " +
                   juce::String(audioRegion->getStartTick()) + " ticks | Gain: " +
                   juce::String(audioRegion->getGainDb(), 1) + " dB",
                   12, 4, getWidth() - 260, 20, juce::Justification::centredLeft, false);

        // Detailed Large Waveform Display
        auto waveBounds = getLocalBounds().reduced(16, 36).toFloat();
        g.setColour(juce::Colour(0xff121417));
        g.fillRoundedRectangle(waveBounds, 4.0f);
        g.setColour(c.border);
        g.drawRoundedRectangle(waveBounds, 4.0f, 1.0f);

        // Center zero line
        float midY = waveBounds.getCentreY();
        g.setColour(c.borderSubtle);
        g.drawHorizontalLine(static_cast<int>(midY), waveBounds.getX(), waveBounds.getRight());

        const auto& peaks = audioRegion->getWaveformPeaks();
        if (!peaks.empty()) {
            juce::Path wavePath;
            float maxAmp = waveBounds.getHeight() * 0.44f;
            size_t count = peaks.size();

            wavePath.startNewSubPath(waveBounds.getX(), midY);
            for (size_t pi = 0; pi < count; ++pi) {
                float ptX = waveBounds.getX() + (static_cast<float>(pi) / static_cast<float>(count)) * waveBounds.getWidth();
                float ptY = midY - peaks[pi].second * maxAmp;
                wavePath.lineTo(ptX, ptY);
            }
            for (int pi = static_cast<int>(count) - 1; pi >= 0; --pi) {
                float ptX = waveBounds.getX() + (static_cast<float>(pi) / static_cast<float>(count)) * waveBounds.getWidth();
                float ptY = midY - peaks[pi].first * maxAmp;
                wavePath.lineTo(ptX, ptY);
            }
            wavePath.closeSubPath();

            g.setColour(c.trackAudioBlue.brighter(0.3f));
            g.fillPath(wavePath);
        }
    } else {
        g.drawText("No Audio Region Selected", 12, 4, 300, 20, juce::Justification::centredLeft, false);
    }
}

void AudioEditorComponent::resized() {
    auto topBar = getLocalBounds().removeFromTop(28).reduced(6, 3);
    reverseButton_.setBounds(topBar.removeFromRight(120));
    topBar.removeFromRight(6);
    normalizeButton_.setBounds(topBar.removeFromRight(90));
}

// ============================================================================
// LowerEditorPanel
// ============================================================================

LowerEditorPanel::LowerEditorPanel(app::AppContext& context)
    : context_(context) {
    context_.addProjectListener(this);

    auto setupTab = [this](juce::TextButton& btn, app::LowerEditorTab tab, const juce::String& tip) {
        btn.setTooltip(tip);
        btn.onClick = [this, tab] { context_.setLowerEditorTab(tab); };
        addAndMakeVisible(btn);
    };

    setupTab(tabMixerButton_, app::LowerEditorTab::Mixer, "Mixer Console [X]");
    setupTab(tabPianoRollButton_, app::LowerEditorTab::PianoRoll, "Piano Roll Editor [P]");
    setupTab(tabAudioEditorButton_, app::LowerEditorTab::AudioEditor, "Audio Sample Editor [E]");

    closePanelButton_.setTooltip("Close Lower Panel");
    closePanelButton_.onClick = [this] { context_.setLowerEditorTab(app::LowerEditorTab::None); };
    addAndMakeVisible(closePanelButton_);

    mixerConsole_ = std::make_unique<MixerConsoleComponent>(context_);
    pianoRoll_ = std::make_unique<PianoRollComponent>(context_);
    audioEditor_ = std::make_unique<AudioEditorComponent>(context_);

    addAndMakeVisible(*mixerConsole_);
    addChildComponent(*pianoRoll_);
    addChildComponent(*audioEditor_);

    context_.onLowerEditorTabChanged = [this](app::LowerEditorTab) {
        updateTabVisibility();
    };

    updateTabVisibility();
}

LowerEditorPanel::~LowerEditorPanel() {
    context_.removeProjectListener(this);
}

void LowerEditorPanel::onProjectChanged(const domain::Project* /*project*/) {
    mixerConsole_->updateMixer();
    pianoRoll_->repaint();
    audioEditor_->repaint();
}

void LowerEditorPanel::onSelectionChanged() {
    pianoRoll_->repaint();
    audioEditor_->repaint();
}

void LowerEditorPanel::updateTabVisibility() {
    auto tab = context_.getLowerEditorTab();
    tabMixerButton_.setToggleState(tab == app::LowerEditorTab::Mixer, juce::dontSendNotification);
    tabPianoRollButton_.setToggleState(tab == app::LowerEditorTab::PianoRoll, juce::dontSendNotification);
    tabAudioEditorButton_.setToggleState(tab == app::LowerEditorTab::AudioEditor, juce::dontSendNotification);

    mixerConsole_->setVisible(tab == app::LowerEditorTab::Mixer);
    pianoRoll_->setVisible(tab == app::LowerEditorTab::PianoRoll);
    audioEditor_->setVisible(tab == app::LowerEditorTab::AudioEditor);

    resized();
    repaint();
}

void LowerEditorPanel::paint(juce::Graphics& g) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    g.fillAll(c.panelBackground);

    // Top horizontal border dividing arrangement from lower editor
    g.setColour(c.border);
    g.drawHorizontalLine(0, 0.0f, static_cast<float>(getWidth()));

    // Tab bar dividing line
    g.drawHorizontalLine(28, 0.0f, static_cast<float>(getWidth()));
}

void LowerEditorPanel::resized() {
    auto bounds = getLocalBounds();

    // Tab bar at top (height 28)
    auto tabRow = bounds.removeFromTop(28).reduced(6, 3);
    tabMixerButton_.setBounds(tabRow.removeFromLeft(70));
    tabRow.removeFromLeft(4);
    tabPianoRollButton_.setBounds(tabRow.removeFromLeft(90));
    tabRow.removeFromLeft(4);
    tabAudioEditorButton_.setBounds(tabRow.removeFromLeft(96));

    closePanelButton_.setBounds(tabRow.removeFromRight(24));

    // Remainder: active tab content
    mixerConsole_->setBounds(bounds);
    pianoRoll_->setBounds(bounds);
    audioEditor_->setBounds(bounds);
}

} // namespace sonora::ui::components
