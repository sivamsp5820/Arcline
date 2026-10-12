#include "domain/Region.h"
#include <cmath>
#include <algorithm>

namespace sonora::domain {

Region::Region(std::string id, std::string trackId, std::string name, RegionType type,
               int64_t startTick, int64_t lengthTicks)
    : id_(std::move(id)),
      trackId_(std::move(trackId)),
      name_(std::move(name)),
      type_(type),
      startTick_(startTick >= 0 ? startTick : 0),
      lengthTicks_(lengthTicks > 0 ? lengthTicks : kTicksPerBeat * 4) {
    if (type_ == RegionType::Audio) {
        colorArgb_ = 0xff2e86ab; // Logic Audio Blue
        generateDemoWaveform(120, 3.5f);
    } else {
        colorArgb_ = 0xff3a7d44; // Logic MIDI Green
    }
}

void Region::generateDemoWaveform(int numPoints, float frequency) {
    waveformPeaks_.clear();
    waveformPeaks_.reserve(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(numPoints);
        float envelope = std::sin(t * 3.14159265f); // Natural arc
        float wave = std::sin(t * frequency * 6.2831853f) * 0.7f +
                     std::sin(t * frequency * 2.3f * 6.2831853f) * 0.3f;
        float peak = std::abs(wave * envelope);
        if (peak > 1.0f) peak = 1.0f;
        waveformPeaks_.emplace_back(-peak, peak);
    }
}

void Region::addNote(const MidiNote& note) {
    notes_.push_back(note);
}

bool Region::removeNote(size_t index) {
    if (index < notes_.size()) {
        notes_.erase(notes_.begin() + index);
        return true;
    }
    return false;
}

void Region::quantizeNotes(int64_t gridTicks) {
    if (gridTicks <= 0) return;
    for (auto& n : notes_) {
        int64_t rem = n.startTick % gridTicks;
        if (rem < gridTicks / 2) {
            n.startTick -= rem;
        } else {
            n.startTick += (gridTicks - rem);
        }
    }
}

void Region::trimLeft(int64_t deltaTicks) {
    if (deltaTicks >= lengthTicks_ - kTicksPerBeat / 4) {
        // Prevent collapsing region to zero
        deltaTicks = lengthTicks_ - kTicksPerBeat / 4;
    }
    startTick_ += deltaTicks;
    offsetTicks_ += deltaTicks;
    lengthTicks_ -= deltaTicks;
    if (lengthTicks_ < kTicksPerBeat / 4) lengthTicks_ = kTicksPerBeat / 4;

    // Shift MIDI notes if MIDI
    if (type_ == RegionType::Midi) {
        for (auto& n : notes_) {
            n.startTick -= deltaTicks;
        }
        // Remove notes that are now before start
        notes_.erase(std::remove_if(notes_.begin(), notes_.end(),
            [](const MidiNote& n) { return n.startTick + n.lengthTicks <= 0; }), notes_.end());
    }
}

void Region::trimRight(int64_t deltaTicks) {
    lengthTicks_ += deltaTicks;
    if (lengthTicks_ < kTicksPerBeat / 4) {
        lengthTicks_ = kTicksPerBeat / 4;
    }
}

std::shared_ptr<Region> Region::split(int64_t splitTick, const std::string& newRegionId) {
    if (splitTick <= startTick_ || splitTick >= getEndTick()) {
        return nullptr; // Split position must be strictly inside the region
    }

    int64_t leftLength = splitTick - startTick_;
    int64_t rightLength = lengthTicks_ - leftLength;

    // Create right region
    auto rightRegion = std::make_shared<Region>(
        newRegionId, trackId_, name_ + " (Split)", type_, splitTick, rightLength);
    rightRegion->setColor(colorArgb_);
    rightRegion->setOffsetTicks(offsetTicks_ + leftLength);
    rightRegion->setGainDb(gainDb_);

    if (type_ == RegionType::Audio) {
        // Split waveform peaks proportionally
        if (!waveformPeaks_.empty()) {
            float ratio = static_cast<float>(leftLength) / static_cast<float>(lengthTicks_);
            size_t splitIdx = static_cast<size_t>(waveformPeaks_.size() * ratio);
            if (splitIdx < waveformPeaks_.size()) {
                std::vector<std::pair<float, float>> rightPeaks(
                    waveformPeaks_.begin() + splitIdx, waveformPeaks_.end());
                waveformPeaks_.resize(splitIdx);
                rightRegion->setWaveformPeaks(std::move(rightPeaks));
            }
        }
    } else if (type_ == RegionType::Midi) {
        // Partition notes between left and right
        std::vector<MidiNote> leftNotes;
        std::vector<MidiNote> rightNotes;
        int64_t splitLocalTick = splitTick - startTick_;

        for (const auto& note : notes_) {
            if (note.startTick < splitLocalTick) {
                if (note.startTick + note.lengthTicks > splitLocalTick) {
                    // Note spans across split: split the note!
                    MidiNote leftPart = note;
                    leftPart.lengthTicks = splitLocalTick - note.startTick;
                    leftNotes.push_back(leftPart);

                    MidiNote rightPart = note;
                    rightPart.startTick = 0;
                    rightPart.lengthTicks = (note.startTick + note.lengthTicks) - splitLocalTick;
                    rightNotes.push_back(rightPart);
                } else {
                    leftNotes.push_back(note);
                }
            } else {
                MidiNote shifted = note;
                shifted.startTick -= splitLocalTick;
                rightNotes.push_back(shifted);
            }
        }
        notes_ = std::move(leftNotes);
        rightRegion->setNotes(std::move(rightNotes));
    }

    // Truncate left region length
    lengthTicks_ = leftLength;

    return rightRegion;
}

bool Region::isValid() const noexcept {
    return !id_.empty() && !trackId_.empty() && lengthTicks_ > 0 && startTick_ >= 0;
}

} // namespace sonora::domain
