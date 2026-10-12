#include "ui/workspaces/ArrangementView.h"
#include "ui/theme/SonoraTheme.h"
#include <cmath>

namespace sonora::ui::workspaces {

using sonora::ui::theme::SonoraTheme;

// ============================================================================
// ArrangementRulerComponent
// ============================================================================

ArrangementRulerComponent::ArrangementRulerComponent(app::AppContext& context, double& pixelsPerTick, int& horizontalScrollOffset)
    : context_(context), pixelsPerTick_(pixelsPerTick), horizontalScrollOffset_(horizontalScrollOffset) {}

void ArrangementRulerComponent::paint(juce::Graphics& g) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();
    auto* project = context_.getActiveProject();

    g.fillAll(c.panelBackground);

    // Bottom border
    g.setColour(c.border);
    g.drawHorizontalLine(getHeight() - 1, 0.0f, static_cast<float>(getWidth()));

    if (!project) return;

    int64_t ticksPerBeat = domain::Project::kTicksPerBeat;
    int num = project->getTimeSigNumerator();
    int64_t ticksPerBar = ticksPerBeat * num;
    double pixelsPerBar = ticksPerBar * pixelsPerTick_;

    if (pixelsPerBar <= 5.0) return;

    // Draw Bar numbers & beat ticks
    int totalBars = 200;
    for (int bar = 1; bar <= totalBars; ++bar) {
        int64_t barTick = (bar - 1) * ticksPerBar;
        float barX = static_cast<float>(barTick * pixelsPerTick_ - horizontalScrollOffset_);

        if (barX >= -100.0f && barX <= getWidth() + 100) {
            // Major bar line
            g.setColour(c.border);
            g.drawVerticalLine(static_cast<int>(barX), 12.0f, static_cast<float>(getHeight()));

            // Bar number label
            g.setFont(theme.getCaptionFont(10.0f));
            g.setColour(c.textSecondary);
            g.drawText(juce::String(bar), static_cast<int>(barX) + 4, 1, 40, 14, juce::Justification::centredLeft, false);

            // Beat subdivision ticks
            if (pixelsPerBar > 40.0) {
                for (int beat = 1; beat < num; ++beat) {
                    float beatX = static_cast<float>((barTick + beat * ticksPerBeat) * pixelsPerTick_ - horizontalScrollOffset_);
                    g.setColour(c.borderSubtle);
                    g.drawVerticalLine(static_cast<int>(beatX), 18.0f, static_cast<float>(getHeight()));
                }
            }
        }
    }

    // Logic Pro X Amber Cycle Range Locator Strip (top 8px)
    if (project->isCycleActive()) {
        float startX = static_cast<float>(project->getCycleStartTick() * pixelsPerTick_ - horizontalScrollOffset_);
        float endX = static_cast<float>(project->getCycleEndTick() * pixelsPerTick_ - horizontalScrollOffset_);
        float cycleW = endX - startX;

        if (cycleW > 1.0f) {
            juce::Rectangle<float> cycleRect(startX, 0.0f, cycleW, 10.0f);
            g.setColour(c.cycleYellow.withAlpha(0.85f));
            g.fillRoundedRectangle(cycleRect, 2.0f);

            g.setColour(juce::Colours::white);
            g.drawRoundedRectangle(cycleRect, 2.0f, 1.0f);
        }
    }

    // Playhead Scrubber Triangle Marker
    float playX = static_cast<float>(project->getPlayheadTick() * pixelsPerTick_ - horizontalScrollOffset_);
    juce::Path triangle;
    triangle.startNewSubPath(playX - 6.0f, 8.0f);
    triangle.lineTo(playX + 6.0f, 8.0f);
    triangle.lineTo(playX, static_cast<float>(getHeight() - 1));
    triangle.closeSubPath();

    g.setColour(juce::Colours::white);
    g.fillPath(triangle);
}

void ArrangementRulerComponent::mouseDown(const juce::MouseEvent& e) {
    auto* project = context_.getActiveProject();
    if (!project) return;

    int64_t clickedTick = static_cast<int64_t>((e.x + horizontalScrollOffset_) / pixelsPerTick_);
    if (clickedTick < 0) clickedTick = 0;

    // Check if clicked near cycle handles
    if (e.y <= 12 && project->isCycleActive()) {
        float startX = static_cast<float>(project->getCycleStartTick() * pixelsPerTick_ - horizontalScrollOffset_);
        float endX = static_cast<float>(project->getCycleEndTick() * pixelsPerTick_ - horizontalScrollOffset_);

        if (std::abs(e.x - startX) <= 6.0f) {
            isDraggingCycleStart_ = true;
            return;
        } else if (std::abs(e.x - endX) <= 6.0f) {
            isDraggingCycleEnd_ = true;
            return;
        } else if (e.x > startX && e.x < endX) {
            isDraggingCycleBody_ = true;
            dragCycleStartTick_ = clickedTick;
            return;
        }
    }

    // Otherwise, move playhead
    project->setPlayheadTick(clickedTick);
    context_.notifyTransportChanged();
}

void ArrangementRulerComponent::mouseDrag(const juce::MouseEvent& e) {
    auto* project = context_.getActiveProject();
    if (!project) return;

    int64_t currentTick = static_cast<int64_t>((e.x + horizontalScrollOffset_) / pixelsPerTick_);
    if (currentTick < 0) currentTick = 0;

    if (isDraggingCycleStart_) {
        project->setCycleRange(currentTick, project->getCycleEndTick());
        repaint();
    } else if (isDraggingCycleEnd_) {
        project->setCycleRange(project->getCycleStartTick(), currentTick);
        repaint();
    } else if (isDraggingCycleBody_) {
        int64_t delta = currentTick - dragCycleStartTick_;
        int64_t len = project->getCycleEndTick() - project->getCycleStartTick();
        int64_t newStart = project->getCycleStartTick() + delta;
        if (newStart < 0) newStart = 0;
        project->setCycleRange(newStart, newStart + len);
        dragCycleStartTick_ = currentTick;
        repaint();
    } else {
        project->setPlayheadTick(currentTick);
        context_.notifyTransportChanged();
    }
}

void ArrangementRulerComponent::mouseDoubleClick(const juce::MouseEvent& /*e*/) {
    context_.toggleCycle();
}

// ============================================================================
// TrackHeaderListComponent
// ============================================================================

TrackHeaderListComponent::TrackHeaderListComponent(app::AppContext& context)
    : context_(context) {
    addAudioTrackButton_.setTooltip("Add New Audio Track");
    addAudioTrackButton_.onClick = [this] {
        if (auto* p = context_.getActiveProject()) {
            p->addTrack(domain::TrackType::Audio);
            context_.notifyProjectChanged();
        }
    };
    addAndMakeVisible(addAudioTrackButton_);

    addInstrumentTrackButton_.setTooltip("Add New Software Instrument Track");
    addInstrumentTrackButton_.onClick = [this] {
        if (auto* p = context_.getActiveProject()) {
            p->addTrack(domain::TrackType::SoftwareInstrument);
            context_.notifyProjectChanged();
        }
    };
    addAndMakeVisible(addInstrumentTrackButton_);
}

void TrackHeaderListComponent::updateTracks() {
    headerControls_.clear();
    auto* project = context_.getActiveProject();
    if (!project) {
        repaint();
        return;
    }

    for (auto& track : project->getTracks()) {
        if (!track) continue;

        HeaderControls hc;
        hc.track = track;

        // Label
        hc.nameLabel = std::make_unique<juce::Label>("", track->getName());
        hc.nameLabel->setFont(SonoraTheme::getInstance().getLabelFont(11.5f));
        hc.nameLabel->setColour(juce::Label::textColourId, SonoraTheme::getInstance().getColors().textPrimary);
        hc.nameLabel->setEditable(true);
        hc.nameLabel->onTextChange = [this, t = track.get(), l = hc.nameLabel.get()] {
            t->setName(l->getText().toStdString());
            context_.notifyProjectChanged();
        };
        addAndMakeVisible(*hc.nameLabel);

        // Mute
        hc.muteButton = std::make_unique<juce::TextButton>("M");
        hc.muteButton->setTooltip("Mute");
        hc.muteButton->setToggleState(track->isMuted(), juce::dontSendNotification);
        hc.muteButton->onClick = [this, t = track.get(), b = hc.muteButton.get()] {
            t->setMuted(!t->isMuted());
            b->setToggleState(t->isMuted(), juce::dontSendNotification);
            context_.notifyProjectChanged();
        };
        addAndMakeVisible(*hc.muteButton);

        // Solo
        hc.soloButton = std::make_unique<juce::TextButton>("S");
        hc.soloButton->setTooltip("Solo");
        hc.soloButton->setToggleState(track->isSoloed(), juce::dontSendNotification);
        hc.soloButton->onClick = [this, t = track.get(), b = hc.soloButton.get()] {
            t->setSoloed(!t->isSoloed());
            b->setToggleState(t->isSoloed(), juce::dontSendNotification);
            context_.notifyProjectChanged();
        };
        addAndMakeVisible(*hc.soloButton);

        // Arm
        hc.armButton = std::make_unique<juce::TextButton>("R");
        hc.armButton->setTooltip("Record Enable");
        hc.armButton->setToggleState(track->isArmed(), juce::dontSendNotification);
        hc.armButton->onClick = [this, t = track.get(), b = hc.armButton.get()] {
            t->setArmed(!t->isArmed());
            b->setToggleState(t->isArmed(), juce::dontSendNotification);
            context_.notifyProjectChanged();
        };
        addAndMakeVisible(*hc.armButton);

        // Mini Vol Slider
        hc.volSlider = std::make_unique<juce::Slider>();
        hc.volSlider->setSliderStyle(juce::Slider::LinearHorizontal);
        hc.volSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        hc.volSlider->setRange(0.0, 1.5, 0.01);
        hc.volSlider->setValue(track->getVolume());
        hc.volSlider->onValueChange = [t = track.get(), s = hc.volSlider.get()] {
            t->setVolume(static_cast<float>(s->getValue()));
        };
        addAndMakeVisible(*hc.volSlider);

        // Pan Knob
        hc.panKnob = std::make_unique<juce::Slider>();
        hc.panKnob->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        hc.panKnob->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        hc.panKnob->setRange(-1.0, 1.0, 0.01);
        hc.panKnob->setValue(track->getPan());
        hc.panKnob->onValueChange = [t = track.get(), k = hc.panKnob.get()] {
            t->setPan(static_cast<float>(k->getValue()));
        };
        addAndMakeVisible(*hc.panKnob);

        headerControls_.push_back(std::move(hc));
    }

    resized();
    repaint();
}

void TrackHeaderListComponent::paint(juce::Graphics& g) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();
    auto* project = context_.getActiveProject();

    g.fillAll(c.panelBackground);

    // Right vertical line dividing track headers from lanes
    g.setColour(c.border);
    g.drawVerticalLine(getWidth() - 1, 0.0f, static_cast<float>(getHeight()));

    // Top tool strip background
    g.setColour(c.surface);
    g.fillRect(0, 0, getWidth(), 30);
    g.setColour(c.border);
    g.drawHorizontalLine(29, 0.0f, static_cast<float>(getWidth()));

    if (!project) return;

    int y = 30;
    for (const auto& hc : headerControls_) {
        if (!hc.track) continue;
        int trackH = hc.track->getHeight();
        bool isSelected = (project->getSelectedTrackId() == hc.track->getId());

        // Header card background
        juce::Rectangle<float> headerRect(0.0f, static_cast<float>(y), static_cast<float>(getWidth() - 1), static_cast<float>(trackH));
        g.setColour(isSelected ? c.surfaceElevated : c.surface);
        g.fillRect(headerRect);

        // Left color stripe
        g.setColour(juce::Colour(hc.track->getColor()));
        g.fillRect(0.0f, static_cast<float>(y), 5.0f, static_cast<float>(trackH));

        // Bottom track divider line
        g.setColour(c.border);
        g.drawHorizontalLine(y + trackH - 1, 0.0f, static_cast<float>(getWidth()));

        // Track icon
        g.setFont(theme.getLabelFont(12.0f));
        g.setColour(c.textSecondary);
        const char* iconStr = (hc.track->getType() == domain::TrackType::Audio) ? "∿" : "🎹";
        g.drawText(iconStr, 10, y + 6, 16, 16, juce::Justification::centred, false);

        y += trackH;
    }
}

void TrackHeaderListComponent::resized() {
    auto bounds = getLocalBounds();
    auto topBar = bounds.removeFromTop(30).reduced(4, 3);

    addAudioTrackButton_.setBounds(topBar.removeFromLeft(66));
    topBar.removeFromLeft(4);
    addInstrumentTrackButton_.setBounds(topBar.removeFromLeft(96));

    int y = 30;
    for (auto& hc : headerControls_) {
        if (!hc.track) continue;
        int trackH = hc.track->getHeight();
        auto row = juce::Rectangle<int>(0, y, getWidth() - 1, trackH).reduced(4, 4);

        // Top line: icon, name
        auto topLine = row.removeFromTop(22);
        topLine.removeFromLeft(24); // space for color stripe & icon
        hc.nameLabel->setBounds(topLine);

        // Middle line: M, S, R buttons + Pan
        auto midLine = row.removeFromTop(22);
        midLine.removeFromLeft(24);
        hc.muteButton->setBounds(midLine.removeFromLeft(22));
        midLine.removeFromLeft(2);
        hc.soloButton->setBounds(midLine.removeFromLeft(22));
        midLine.removeFromLeft(2);
        hc.armButton->setBounds(midLine.removeFromLeft(22));
        midLine.removeFromLeft(8);
        hc.panKnob->setBounds(midLine.removeFromLeft(22));

        // Bottom line: Vol Slider
        auto botLine = row.removeFromTop(18);
        botLine.removeFromLeft(24);
        hc.volSlider->setBounds(botLine);

        y += trackH;
    }
}

void TrackHeaderListComponent::mouseDown(const juce::MouseEvent& e) {
    if (e.y < 30) return;
    auto* project = context_.getActiveProject();
    if (!project) return;

    int y = 30;
    for (const auto& hc : headerControls_) {
        if (!hc.track) continue;
        int trackH = hc.track->getHeight();
        if (e.y >= y && e.y < y + trackH) {
            project->selectTrack(hc.track->getId());
            context_.notifySelectionChanged();

            // Right click context menu
            if (e.mods.isPopupMenu()) {
                juce::PopupMenu menu;
                menu.addItem(1, "Duplicate Track");
                menu.addItem(2, "Delete Track");
                menu.showMenuAsync(juce::PopupMenu::Options{}, [this, tId = hc.track->getId()](int res) {
                    if (res == 1) {
                        if (auto* p = context_.getActiveProject()) {
                            p->duplicateTrack(tId);
                            context_.notifyProjectChanged();
                        }
                    } else if (res == 2) {
                        if (auto* p = context_.getActiveProject()) {
                            p->removeTrack(tId);
                            context_.notifyProjectChanged();
                        }
                    }
                });
            }
            break;
        }
        y += trackH;
    }
}

// ============================================================================
// TimelineLanesComponent
// ============================================================================

TimelineLanesComponent::TimelineLanesComponent(app::AppContext& context, double& pixelsPerTick,
                                               int& horizontalScrollOffset, int& verticalScrollOffset)
    : context_(context), pixelsPerTick_(pixelsPerTick),
      horizontalScrollOffset_(horizontalScrollOffset), verticalScrollOffset_(verticalScrollOffset) {}

void TimelineLanesComponent::paint(juce::Graphics& g) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();
    auto* project = context_.getActiveProject();

    g.fillAll(c.timelineBackground);

    if (!project) return;

    int64_t ticksPerBeat = domain::Project::kTicksPerBeat;
    int num = project->getTimeSigNumerator();
    int64_t ticksPerBar = ticksPerBeat * num;

    // Draw Track Lanes Background & Horizontal divider lines
    int currentY = 0;
    for (const auto& track : project->getTracks()) {
        if (!track) continue;
        int trackH = track->getHeight();

        // Lane background
        g.setColour(c.timelineBackground);
        g.fillRect(0, currentY, getWidth(), trackH);

        // Horizontal lane border
        g.setColour(c.borderSubtle);
        g.drawHorizontalLine(currentY + trackH - 1, 0.0f, static_cast<float>(getWidth()));

        // Vertical Bar Grid lines
        for (int bar = 1; bar <= 200; ++bar) {
            float barX = static_cast<float>((bar - 1) * ticksPerBar * pixelsPerTick_ - horizontalScrollOffset_);
            if (barX >= -10.0f && barX <= getWidth() + 10.0f) {
                g.setColour(c.borderSubtle.withAlpha(0.6f));
                g.drawVerticalLine(static_cast<int>(barX), static_cast<float>(currentY), static_cast<float>(currentY + trackH));
            }
        }

        // Draw Regions for this Track
        for (const auto& region : track->getRegions()) {
            if (!region) continue;

            float regX = static_cast<float>(region->getStartTick() * pixelsPerTick_ - horizontalScrollOffset_);
            float regW = static_cast<float>(region->getLengthTicks() * pixelsPerTick_);
            float regY = static_cast<float>(currentY + 2);
            float regH = static_cast<float>(trackH - 4);

            if (regX + regW < 0.0f || regX > getWidth()) continue;

            juce::Rectangle<float> regBounds(regX, regY, regW, regH);

            // Region body fill
            juce::Colour regColor(region->getColor());
            if (region->isMuted()) regColor = regColor.darker(0.6f);

            g.setColour(regColor);
            g.fillRoundedRectangle(regBounds, 4.0f);

            // Region header bar
            auto regHeader = regBounds.removeFromTop(16.0f);
            g.setColour(regColor.darker(0.2f));
            g.fillRoundedRectangle(regHeader, 4.0f);

            // Region title
            g.setFont(theme.getLabelFont(10.5f));
            g.setColour(juce::Colours::white);
            g.drawText(region->getName(), regHeader.reduced(4.0f, 0.0f).toNearestInt(),
                       juce::Justification::centredLeft, true);

            // Draw Region Payload: Real Waveforms for Audio, Real Notes for MIDI!
            auto payloadArea = regBounds.reduced(2.0f);

            if (region->getType() == domain::RegionType::Audio) {
                // Waveform rendering
                const auto& peaks = region->getWaveformPeaks();
                if (!peaks.empty() && payloadArea.getWidth() > 2.0f) {
                    juce::Path wavePath;
                    float midY = payloadArea.getCentreY();
                    float maxAmp = payloadArea.getHeight() * 0.45f;
                    size_t count = peaks.size();

                    wavePath.startNewSubPath(payloadArea.getX(), midY);
                    for (size_t pi = 0; pi < count; ++pi) {
                        float ptX = payloadArea.getX() + (static_cast<float>(pi) / static_cast<float>(count)) * payloadArea.getWidth();
                        float ptY = midY - peaks[pi].second * maxAmp;
                        wavePath.lineTo(ptX, ptY);
                    }
                    for (int pi = static_cast<int>(count) - 1; pi >= 0; --pi) {
                        float ptX = payloadArea.getX() + (static_cast<float>(pi) / static_cast<float>(count)) * payloadArea.getWidth();
                        float ptY = midY - peaks[pi].first * maxAmp;
                        wavePath.lineTo(ptX, ptY);
                    }
                    wavePath.closeSubPath();

                    g.setColour(juce::Colour(0xffe4f0fb).withAlpha(0.8f));
                    g.fillPath(wavePath);
                }
            } else if (region->getType() == domain::RegionType::Midi) {
                // Note-pattern preview rendering
                const auto& notes = region->getNotes();
                for (const auto& note : notes) {
                    float noteX = payloadArea.getX() + static_cast<float>(note.startTick) * static_cast<float>(pixelsPerTick_);
                    float noteW = static_cast<float>(note.lengthTicks) * static_cast<float>(pixelsPerTick_);
                    if (noteW < 3.0f) noteW = 3.0f;

                    // Invert pitch vertically (higher notes near top)
                    float normPitch = static_cast<float>(note.pitch - 30) / 60.0f;
                    if (normPitch < 0.0f) normPitch = 0.0f;
                    if (normPitch > 1.0f) normPitch = 1.0f;
                    float noteY = payloadArea.getBottom() - normPitch * payloadArea.getHeight();

                    juce::Rectangle<float> noteRect(noteX, noteY, noteW, 3.5f);
                    g.setColour(juce::Colour(0xffc5e1a5));
                    g.fillRoundedRectangle(noteRect, 1.0f);
                }
            }

            // Selection border
            if (region->isSelected()) {
                g.setColour(juce::Colours::white);
                g.drawRoundedRectangle(juce::Rectangle<float>(regX, regY, regW, regH), 4.0f, 1.5f);
            } else {
                g.setColour(regColor.darker(0.4f));
                g.drawRoundedRectangle(juce::Rectangle<float>(regX, regY, regW, regH), 4.0f, 1.0f);
            }
        }

        currentY += trackH;
    }

    // Draw Global Full-Height Playhead Line
    float playheadX = static_cast<float>(project->getPlayheadTick() * pixelsPerTick_ - horizontalScrollOffset_);
    g.setColour(juce::Colours::white);
    g.drawVerticalLine(static_cast<int>(playheadX), 0.0f, static_cast<float>(getHeight()));
}

void TimelineLanesComponent::mouseDown(const juce::MouseEvent& e) {
    auto* project = context_.getActiveProject();
    if (!project) return;

    int64_t clickedTick = static_cast<int64_t>((e.x + horizontalScrollOffset_) / pixelsPerTick_);
    if (clickedTick < 0) clickedTick = 0;

    int currentY = 0;
    std::shared_ptr<domain::Region> clickedRegion = nullptr;
    std::shared_ptr<domain::Track> clickedTrack = nullptr;

    for (const auto& track : project->getTracks()) {
        if (!track) continue;
        int trackH = track->getHeight();

        if (e.y >= currentY && e.y < currentY + trackH) {
            clickedTrack = track;
            for (const auto& region : track->getRegions()) {
                if (!region) continue;
                float rx = static_cast<float>(region->getStartTick() * pixelsPerTick_ - horizontalScrollOffset_);
                float rw = static_cast<float>(region->getLengthTicks() * pixelsPerTick_);
                if (e.x >= rx && e.x <= rx + rw) {
                    clickedRegion = region;
                    break;
                }
            }
            break;
        }
        currentY += trackH;
    }

    auto activeTool = context_.getActiveTool();

    if (clickedRegion) {
        if (activeTool == app::ToolType::Scissors) {
            // Scissors Tool: split region on click
            clickedTrack->splitRegion(clickedRegion->getId(), clickedTick,
                                      "reg_" + juce::Uuid().toString().toStdString().substr(0, 8));
            context_.notifyProjectChanged();
            return;
        } else if (activeTool == app::ToolType::Eraser) {
            // Eraser Tool: delete region on click
            clickedTrack->removeRegion(clickedRegion->getId());
            context_.notifyProjectChanged();
            return;
        }

        // Pointer Tool: Select and initiate drag
        project->selectRegion(clickedRegion->getId());
        context_.notifySelectionChanged();

        float rx = static_cast<float>(clickedRegion->getStartTick() * pixelsPerTick_ - horizontalScrollOffset_);
        float rw = static_cast<float>(clickedRegion->getLengthTicks() * pixelsPerTick_);

        draggingRegionId_ = clickedRegion->getId();
        dragInitialStartTick_ = clickedRegion->getStartTick();
        dragInitialLengthTicks_ = clickedRegion->getLengthTicks();
        dragInitialMouseX_ = e.x;

        if (std::abs(e.x - rx) <= 6.0f) {
            dragMode_ = DragMode::TrimLeft;
        } else if (std::abs(e.x - (rx + rw)) <= 6.0f) {
            dragMode_ = DragMode::TrimRight;
        } else {
            dragMode_ = DragMode::MoveRegion;
        }

        // Double-click opens corresponding editor
        if (e.getNumberOfClicks() == 2) {
            if (clickedRegion->getType() == domain::RegionType::Midi) {
                context_.setLowerEditorTab(app::LowerEditorTab::PianoRoll);
            } else {
                context_.setLowerEditorTab(app::LowerEditorTab::AudioEditor);
            }
        }
    } else {
        // Clicked empty lane: clear selection, set playhead
        project->clearRegionSelection();
        if (clickedTrack) project->selectTrack(clickedTrack->getId());
        project->setPlayheadTick(clickedTick);
        context_.notifyTransportChanged();
    }
}

void TimelineLanesComponent::mouseDrag(const juce::MouseEvent& e) {
    if (dragMode_ == DragMode::None || draggingRegionId_.empty()) return;
    auto* project = context_.getActiveProject();
    if (!project) return;

    auto region = project->findRegion(draggingRegionId_);
    if (!region) return;

    int dx = e.x - dragInitialMouseX_;
    int64_t tickDelta = static_cast<int64_t>(dx / pixelsPerTick_);

    // Snap to nearest beat
    int64_t snapTicks = domain::Project::kTicksPerBeat;
    tickDelta = (tickDelta / snapTicks) * snapTicks;

    if (dragMode_ == DragMode::MoveRegion) {
        int64_t newStart = dragInitialStartTick_ + tickDelta;
        if (newStart < 0) newStart = 0;
        region->setStartTick(newStart);
        repaint();
    } else if (dragMode_ == DragMode::TrimRight) {
        int64_t newLen = dragInitialLengthTicks_ + tickDelta;
        if (newLen >= snapTicks / 4) {
            region->setLengthTicks(newLen);
            repaint();
        }
    } else if (dragMode_ == DragMode::TrimLeft) {
        int64_t newStart = dragInitialStartTick_ + tickDelta;
        int64_t newLen = dragInitialLengthTicks_ - tickDelta;
        if (newStart >= 0 && newLen >= snapTicks / 4) {
            region->setStartTick(newStart);
            region->setLengthTicks(newLen);
            repaint();
        }
    }
}

void TimelineLanesComponent::mouseUp(const juce::MouseEvent& /*e*/) {
    if (dragMode_ != DragMode::None) {
        dragMode_ = DragMode::None;
        draggingRegionId_.clear();
        context_.notifyProjectChanged();
    }
}

void TimelineLanesComponent::mouseMove(const juce::MouseEvent& e) {
    auto* project = context_.getActiveProject();
    if (!project) return;

    // Check if hovering near region trim edges
    for (const auto& track : project->getTracks()) {
        if (!track) continue;
        for (const auto& region : track->getRegions()) {
            if (!region) continue;
            float rx = static_cast<float>(region->getStartTick() * pixelsPerTick_ - horizontalScrollOffset_);
            float rw = static_cast<float>(region->getLengthTicks() * pixelsPerTick_);
            if (std::abs(e.x - rx) <= 5.0f || std::abs(e.x - (rx + rw)) <= 5.0f) {
                setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
                return;
            }
        }
    }
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

// ============================================================================
// ArrangementView Implementation
// ============================================================================

ArrangementView::ArrangementView(app::AppContext& context)
    : context_(context) {
    context_.addProjectListener(this);

    trackHeaders_ = std::make_unique<TrackHeaderListComponent>(context_);
    addAndMakeVisible(*trackHeaders_);

    ruler_ = std::make_unique<ArrangementRulerComponent>(context_, pixelsPerTick_, horizontalScrollOffset_);
    addAndMakeVisible(*ruler_);

    timelineLanes_ = std::make_unique<TimelineLanesComponent>(context_, pixelsPerTick_,
                                                             horizontalScrollOffset_, verticalScrollOffset_);
    addAndMakeVisible(*timelineLanes_);

    horizontalScrollBar_.addListener(this);
    horizontalScrollBar_.setRangeLimits(0.0, 50000.0);
    horizontalScrollBar_.setCurrentRange(0.0, 2000.0);
    addAndMakeVisible(horizontalScrollBar_);

    zoomSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    zoomSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    zoomSlider_.setRange(0.02, 0.35, 0.01);
    zoomSlider_.setValue(pixelsPerTick_);
    zoomSlider_.setTooltip("Timeline Horizontal Zoom");
    zoomSlider_.onValueChange = [this] {
        pixelsPerTick_ = zoomSlider_.getValue();
        repaint();
    };
    addAndMakeVisible(zoomSlider_);

    updateContent();
}

ArrangementView::~ArrangementView() {
    context_.removeProjectListener(this);
}

void ArrangementView::onProjectChanged(const domain::Project* /*project*/) {
    updateContent();
}

void ArrangementView::onSelectionChanged() {
    trackHeaders_->repaint();
    timelineLanes_->repaint();
}

void ArrangementView::onTransportChanged() {
    ruler_->repaint();
    timelineLanes_->repaint();
}

void ArrangementView::scrollBarMoved(juce::ScrollBar* scrollBar, double newRangeStart) {
    if (scrollBar == &horizontalScrollBar_) {
        horizontalScrollOffset_ = static_cast<int>(newRangeStart);
        ruler_->repaint();
        timelineLanes_->repaint();
    }
}

void ArrangementView::updateContent() {
    trackHeaders_->updateTracks();
    ruler_->repaint();
    timelineLanes_->repaint();
}

void ArrangementView::paint(juce::Graphics& g) {
    auto& theme = SonoraTheme::getInstance();
    g.fillAll(theme.getColors().background);
}

void ArrangementView::resized() {
    auto bounds = getLocalBounds();

    int headerWidth = 240;
    int rulerHeight = 30;
    int scrollBarH = 14;

    // Bottom scrollbar & zoom
    auto botBar = bounds.removeFromBottom(scrollBarH);
    zoomSlider_.setBounds(botBar.removeFromRight(80));
    botBar.removeFromRight(4);
    botBar.removeFromLeft(headerWidth);
    horizontalScrollBar_.setBounds(botBar);

    // Left track headers
    trackHeaders_->setBounds(bounds.removeFromLeft(headerWidth));

    // Top horizontal ruler
    ruler_->setBounds(bounds.removeFromTop(rulerHeight));

    // Remainder: Timeline Lanes
    timelineLanes_->setBounds(bounds);
}

} // namespace sonora::ui::workspaces
