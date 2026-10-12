#include "ui/components/InspectorPanel.h"
#include "ui/theme/SonoraTheme.h"
#include <iomanip>
#include <sstream>

namespace sonora::ui::components {

// ============================================================================
// ChannelStripComponent Implementation
// ============================================================================

ChannelStripComponent::ChannelStripComponent(app::AppContext& context, bool isMaster)
    : context_(context), isMaster_(isMaster) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    titleLabel_.setFont(theme.getLabelFont(11.0f));
    titleLabel_.setColour(juce::Label::textColourId, c.textPrimary);
    titleLabel_.setJustificationType(juce::Justification::centred);
    titleLabel_.setText(isMaster ? "Stereo Out" : "Track", juce::dontSendNotification);
    addAndMakeVisible(titleLabel_);

    // Insert effect slots (Phase 6 VST3 Plugin Hosting)
    for (int i = 0; i < 4; ++i) {
        insertSlots_[i].setButtonText("- Empty -");
        insertSlots_[i].setTooltip("Insert Effect Slot " + juce::String(i + 1) + " [VST3 Hosting - Phase 6]");
        insertSlots_[i].setEnabled(false);
        addAndMakeVisible(insertSlots_[i]);
    }

    // Sends (Phase 5 Mixer Bus Routing)
    if (!isMaster) {
        sendSlots_[0].setButtonText("- Bus Send -");
        sendSlots_[1].setButtonText("- Bus Send -");
        sendSlots_[0].setTooltip("Auxiliary Bus Send 1 [Mixer Routing - Phase 5]");
        sendSlots_[1].setTooltip("Auxiliary Bus Send 2 [Mixer Routing - Phase 5]");
        sendSlots_[0].setEnabled(false);
        sendSlots_[1].setEnabled(false);
        addAndMakeVisible(sendSlots_[0]);
        addAndMakeVisible(sendSlots_[1]);

        for (int i = 0; i < 2; ++i) {
            sendKnobs_[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            sendKnobs_[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            sendKnobs_[i].setRange(0.0, 1.0, 0.01);
            sendKnobs_[i].setValue(0.0);
            sendKnobs_[i].setEnabled(false);
            sendKnobs_[i].setTooltip("Aux Send Level [Phase 5]");
            addAndMakeVisible(sendKnobs_[i]);
        }
    }

    // Pan Knob
    panKnob_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    panKnob_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    panKnob_.setRange(-1.0, 1.0, 0.01);
    panKnob_.setValue(0.0);
    panKnob_.setTooltip("Stereo Pan Balance");
    panKnob_.onValueChange = [this] {
        if (track_) {
            track_->setPan(static_cast<float>(panKnob_.getValue()));
        } else if (isMaster_) {
            if (auto* p = context_.getActiveProject()) {
                p->setMasterPan(static_cast<float>(panKnob_.getValue()));
            }
        }
    };
    addAndMakeVisible(panKnob_);

    // Fader
    faderSlider_.setSliderStyle(juce::Slider::LinearVertical);
    faderSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    faderSlider_.setRange(0.0, 1.5, 0.01);
    faderSlider_.setValue(1.0);
    faderSlider_.setTooltip("Channel Volume Level");
    faderSlider_.onValueChange = [this] {
        float vol = static_cast<float>(faderSlider_.getValue());
        if (track_) {
            track_->setVolume(vol);
        } else if (isMaster_) {
            if (auto* p = context_.getActiveProject()) {
                p->setMasterVolume(vol);
            }
        }
        // Update dB readout
        float db = (vol > 0.0001f) ? (20.0f * std::log10(vol)) : -60.0f;
        char dbBuf[16];
        if (db <= -59.0f) snprintf(dbBuf, sizeof(dbBuf), "-inf dB");
        else snprintf(dbBuf, sizeof(dbBuf), "%.1f dB", db);
        dbReadout_.setText(dbBuf, juce::dontSendNotification);
    };
    addAndMakeVisible(faderSlider_);

    dbReadout_.setFont(theme.getCaptionFont(10.0f));
    dbReadout_.setColour(juce::Label::textColourId, c.textSecondary);
    dbReadout_.setJustificationType(juce::Justification::centred);
    dbReadout_.setText("0.0 dB", juce::dontSendNotification);
    addAndMakeVisible(dbReadout_);

    // Mute, Solo, Arm Buttons
    if (!isMaster) {
        muteButton_.setTooltip("Mute Track [M]");
        muteButton_.onClick = [this] {
            if (track_) {
                track_->setMuted(!track_->isMuted());
                muteButton_.setToggleState(track_->isMuted(), juce::dontSendNotification);
            }
        };
        addAndMakeVisible(muteButton_);

        soloButton_.setTooltip("Solo Track [S]");
        soloButton_.onClick = [this] {
            if (track_) {
                track_->setSoloed(!track_->isSoloed());
                soloButton_.setToggleState(track_->isSoloed(), juce::dontSendNotification);
            }
        };
        addAndMakeVisible(soloButton_);

        armButton_.setTooltip("Record Enable [R]");
        armButton_.onClick = [this] {
            if (track_) {
                track_->setArmed(!track_->isArmed());
                armButton_.setToggleState(track_->isArmed(), juce::dontSendNotification);
            }
        };
        addAndMakeVisible(armButton_);
    }
}

void ChannelStripComponent::setTrack(std::shared_ptr<domain::Track> track) {
    track_ = track;
    updateControls();
}

void ChannelStripComponent::updateControls() {
    if (track_) {
        titleLabel_.setText(track_->getName(), juce::dontSendNotification);
        panKnob_.setValue(track_->getPan(), juce::dontSendNotification);
        faderSlider_.setValue(track_->getVolume(), juce::dontSendNotification);
        muteButton_.setToggleState(track_->isMuted(), juce::dontSendNotification);
        soloButton_.setToggleState(track_->isSoloed(), juce::dontSendNotification);
        armButton_.setToggleState(track_->isArmed(), juce::dontSendNotification);
        setEnabled(true);
    } else if (isMaster_) {
        if (auto* p = context_.getActiveProject()) {
            titleLabel_.setText("Stereo Out", juce::dontSendNotification);
            panKnob_.setValue(p->getMasterPan(), juce::dontSendNotification);
            faderSlider_.setValue(p->getMasterVolume(), juce::dontSendNotification);
            setEnabled(true);
        } else {
            setEnabled(false);
        }
    } else {
        titleLabel_.setText("No Track", juce::dontSendNotification);
        setEnabled(false);
    }
    repaint();
}

void ChannelStripComponent::paint(juce::Graphics& g) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    // Strip background
    g.setColour(c.surface);
    g.fillRoundedRectangle(bounds, 3.0f);
    g.setColour(c.border);
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

    // EQ Thumbnail curve area at top
    auto eqArea = bounds.removeFromTop(42.0f).reduced(4.0f, 3.0f);
    g.setColour(juce::Colour(0xff121417));
    g.fillRoundedRectangle(eqArea, 2.0f);
    g.setColour(c.borderSubtle);
    g.drawRoundedRectangle(eqArea, 2.0f, 1.0f);

    // Draw stylized blue EQ curve
    juce::Path eqPath;
    float eqW = eqArea.getWidth();
    float eqH = eqArea.getHeight();
    float eqMidY = eqArea.getY() + eqH * 0.5f;

    eqPath.startNewSubPath(eqArea.getX(), eqMidY);
    eqPath.quadraticTo(eqArea.getX() + eqW * 0.25f, eqMidY - (isMaster_ ? 4.0f : 8.0f),
                       eqArea.getX() + eqW * 0.5f, eqMidY + (isMaster_ ? 2.0f : 3.0f));
    eqPath.quadraticTo(eqArea.getX() + eqW * 0.75f, eqMidY - (isMaster_ ? 6.0f : 10.0f),
                       eqArea.getRight(), eqMidY);
    g.setColour(c.selectionBlue);
    g.strokePath(eqPath, juce::PathStrokeType(1.5f));

    // Stereo Peak Meter beside Fader
    auto faderBounds = faderSlider_.getBounds();
    int meterX = faderBounds.getRight() + 2;
    int meterW = 6;
    int meterH = faderBounds.getHeight();
    int meterY = faderBounds.getY();

    if (meterX + meterW <= getWidth() - 2) {
        // Meter background
        g.setColour(juce::Colour(0xff121417));
        g.fillRect(meterX, meterY, meterW, meterH);

        // Subdued static peak display (not fake live animated audio)
        float level = track_ ? (track_->getVolume() / 1.5f) : 0.65f;
        int filledH = static_cast<int>(meterH * level * 0.7f);
        int filledY = meterY + meterH - filledH;

        juce::ColourGradient meterGrad(juce::Colour(0xff27ae60), static_cast<float>(meterX), static_cast<float>(meterY + meterH),
                                       juce::Colour(0xfff39c12), static_cast<float>(meterX), static_cast<float>(meterY), false);
        g.setGradientFill(meterGrad);
        g.fillRect(meterX, filledY, meterW, filledH);

        g.setColour(c.borderSubtle);
        g.drawRect(meterX, meterY, meterW, meterH, 1);
    }
}

void ChannelStripComponent::resized() {
    auto bounds = getLocalBounds().reduced(4, 4);

    // Title label at top
    titleLabel_.setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(44); // Space for EQ thumbnail

    // Inserts (4 slots)
    for (int i = 0; i < 4; ++i) {
        insertSlots_[i].setBounds(bounds.removeFromTop(18));
        bounds.removeFromTop(2);
    }
    bounds.removeFromTop(4);

    // Sends (if track)
    if (!isMaster_) {
        for (int i = 0; i < 2; ++i) {
            auto sendRow = bounds.removeFromTop(20);
            sendSlots_[i].setBounds(sendRow.removeFromLeft(sendRow.getWidth() - 24));
            sendKnobs_[i].setBounds(sendRow);
            bounds.removeFromTop(2);
        }
        bounds.removeFromTop(4);
    }

    // Pan knob
    auto panRow = bounds.removeFromTop(28);
    panKnob_.setBounds(panRow.withSizeKeepingCentre(28, 28));
    bounds.removeFromTop(4);

    // Buttons: Mute, Solo, Arm
    if (!isMaster_) {
        auto btnRow = bounds.removeFromTop(22);
        int btnW = btnRow.getWidth() / 3;
        muteButton_.setBounds(btnRow.removeFromLeft(btnW).reduced(1, 0));
        soloButton_.setBounds(btnRow.removeFromLeft(btnW).reduced(1, 0));
        armButton_.setBounds(btnRow.reduced(1, 0));
        bounds.removeFromTop(4);
    }

    // Fader & Meter
    dbReadout_.setBounds(bounds.removeFromBottom(16));
    bounds.removeFromBottom(2);

    faderSlider_.setBounds(bounds.removeFromLeft(bounds.getWidth() - 10));
}

// ============================================================================
// InspectorPanel Implementation
// ============================================================================

InspectorPanel::InspectorPanel(app::AppContext& context)
    : context_(context) {
    context_.addProjectListener(this);

    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    headerLabel_.setFont(theme.getHeadingFont(12.0f));
    headerLabel_.setColour(juce::Label::textColourId, c.textSecondary);
    addAndMakeVisible(headerLabel_);

    regionTitleLabel_.setFont(theme.getLabelFont(11.5f));
    regionTitleLabel_.setColour(juce::Label::textColourId, c.textPrimary);
    addAndMakeVisible(regionTitleLabel_);

    muteRegionToggle_.setButtonText("Mute Region");
    muteRegionToggle_.onClick = [this] {
        if (auto* p = context_.getActiveProject()) {
            if (auto r = p->getSelectedRegion()) {
                r->setMuted(muteRegionToggle_.getToggleState());
                context_.notifyProjectChanged();
            }
        }
    };
    addAndMakeVisible(muteRegionToggle_);

    loopRegionToggle_.setButtonText("Loop Region");
    loopRegionToggle_.onClick = [this] {
        if (auto* p = context_.getActiveProject()) {
            if (auto r = p->getSelectedRegion()) {
                r->setLooped(loopRegionToggle_.getToggleState());
                context_.notifyProjectChanged();
            }
        }
    };
    addAndMakeVisible(loopRegionToggle_);

    quantizeCombo_.addItem("Quantize: Off", 1);
    quantizeCombo_.addItem("Quantize: 1/4 Note", 2);
    quantizeCombo_.addItem("Quantize: 1/8 Note", 3);
    quantizeCombo_.addItem("Quantize: 1/16 Note", 4);
    quantizeCombo_.setSelectedId(1, juce::dontSendNotification);
    quantizeCombo_.onChange = [this] {
        int id = quantizeCombo_.getSelectedId();
        if (auto* p = context_.getActiveProject()) {
            if (auto r = p->getSelectedRegion()) {
                if (r->getType() == domain::RegionType::Midi) {
                    int64_t grid = 0;
                    if (id == 2) grid = 960;
                    else if (id == 3) grid = 480;
                    else if (id == 4) grid = 240;
                    if (grid > 0) {
                        r->quantizeNotes(grid);
                        context_.notifyProjectChanged();
                    }
                }
            }
        }
    };
    addAndMakeVisible(quantizeCombo_);

    gainLabel_.setFont(theme.getCaptionFont(10.5f));
    gainLabel_.setColour(juce::Label::textColourId, c.textSecondary);
    addAndMakeVisible(gainLabel_);

    gainSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    gainSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    gainSlider_.setRange(-24.0, 12.0, 0.5);
    gainSlider_.setValue(0.0);
    gainSlider_.onValueChange = [this] {
        if (auto* p = context_.getActiveProject()) {
            if (auto r = p->getSelectedRegion()) {
                r->setGainDb(static_cast<float>(gainSlider_.getValue()));
                context_.notifyProjectChanged();
            }
        }
    };
    addAndMakeVisible(gainSlider_);

    // Dual channel strips
    selectedTrackStrip_ = std::make_unique<ChannelStripComponent>(context_, false);
    masterStrip_ = std::make_unique<ChannelStripComponent>(context_, true);
    addAndMakeVisible(*selectedTrackStrip_);
    addAndMakeVisible(*masterStrip_);

    updateInspector();
}

InspectorPanel::~InspectorPanel() {
    context_.removeProjectListener(this);
}

void InspectorPanel::onProjectChanged(const domain::Project* /*project*/) {
    updateInspector();
}

void InspectorPanel::onSelectionChanged() {
    updateInspector();
}

void InspectorPanel::updateInspector() {
    auto* project = context_.getActiveProject();
    if (project) {
        auto region = project->getSelectedRegion();
        if (region) {
            regionTitleLabel_.setText(region->getName(), juce::dontSendNotification);
            muteRegionToggle_.setToggleState(region->isMuted(), juce::dontSendNotification);
            loopRegionToggle_.setToggleState(region->isLooped(), juce::dontSendNotification);
            gainSlider_.setValue(region->getGainDb(), juce::dontSendNotification);
            muteRegionToggle_.setEnabled(true);
            loopRegionToggle_.setEnabled(true);
            quantizeCombo_.setEnabled(region->getType() == domain::RegionType::Midi);
            gainSlider_.setEnabled(true);
        } else {
            regionTitleLabel_.setText("No Region Selected", juce::dontSendNotification);
            muteRegionToggle_.setEnabled(false);
            loopRegionToggle_.setEnabled(false);
            quantizeCombo_.setEnabled(false);
            gainSlider_.setEnabled(false);
        }

        selectedTrackStrip_->setTrack(project->getSelectedTrack());
        masterStrip_->setTrack(nullptr); // Master strip
    } else {
        regionTitleLabel_.setText("No Project", juce::dontSendNotification);
        muteRegionToggle_.setEnabled(false);
        loopRegionToggle_.setEnabled(false);
        quantizeCombo_.setEnabled(false);
        gainSlider_.setEnabled(false);
        selectedTrackStrip_->setTrack(nullptr);
        masterStrip_->setTrack(nullptr);
    }
    repaint();
}

void InspectorPanel::paint(juce::Graphics& g) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    g.fillAll(c.panelBackground);

    // Right vertical border dividing inspector from multitrack timeline
    g.setColour(c.border);
    g.drawVerticalLine(getWidth() - 1, 0.0f, static_cast<float>(getHeight()));

    // Region parameters section outline
    auto regionBox = juce::Rectangle<float>(6.0f, 26.0f, static_cast<float>(getWidth() - 12), 126.0f);
    g.setColour(c.surface);
    g.fillRoundedRectangle(regionBox, 3.0f);
    g.setColour(c.border);
    g.drawRoundedRectangle(regionBox, 3.0f, 1.0f);
}

void InspectorPanel::resized() {
    auto bounds = getLocalBounds().reduced(6, 6);

    headerLabel_.setBounds(bounds.removeFromTop(20));

    // Region Parameters box (height ~126px)
    auto regionBox = bounds.removeFromTop(126).reduced(6, 6);
    regionTitleLabel_.setBounds(regionBox.removeFromTop(20));
    regionBox.removeFromTop(4);

    auto togglesRow = regionBox.removeFromTop(22);
    muteRegionToggle_.setBounds(togglesRow.removeFromLeft(togglesRow.getWidth() / 2));
    loopRegionToggle_.setBounds(togglesRow);
    regionBox.removeFromTop(4);

    quantizeCombo_.setBounds(regionBox.removeFromTop(24));
    regionBox.removeFromTop(6);

    auto gainRow = regionBox.removeFromTop(20);
    gainLabel_.setBounds(gainRow.removeFromLeft(36));
    gainSlider_.setBounds(gainRow);

    bounds.removeFromTop(10);

    // Bottom remainder: Dual Channel Strips side-by-side
    int stripWidth = (bounds.getWidth() - 4) / 2;
    selectedTrackStrip_->setBounds(bounds.removeFromLeft(stripWidth));
    bounds.removeFromLeft(4);
    masterStrip_->setBounds(bounds.removeFromLeft(stripWidth));
}

} // namespace sonora::ui::components
