#include "domain/Project.h"
#include <juce_core/juce_core.h>
#include <algorithm>

namespace sonora::domain {

Project::Project()
    : id_(juce::Uuid().toString().toStdString()) {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    createdTimestamp_ = now;
    modifiedTimestamp_ = now;
}

Project::Project(std::string name, std::string filePath)
    : Project() {
    name_ = std::move(name);
    filePath_ = std::move(filePath);
}

bool Project::setTempoBpm(double bpm) noexcept {
    if (bpm >= kMinTempoBpm && bpm <= kMaxTempoBpm) {
        tempoBpm_ = bpm;
        updateModifiedTimestamp();
        return true;
    }
    return false;
}

bool Project::setTimeSignature(int numerator, int denominator) noexcept {
    if (numerator >= 1 && numerator <= 32 &&
        (denominator == 2 || denominator == 4 || denominator == 8 || denominator == 16)) {
        timeSigNumerator_ = numerator;
        timeSigDenominator_ = denominator;
        updateModifiedTimestamp();
        return true;
    }
    return false;
}

void Project::setCycleRange(int64_t startTick, int64_t endTick) noexcept {
    if (startTick >= 0 && endTick > startTick) {
        cycleStartTick_ = startTick;
        cycleEndTick_ = endTick;
    }
}

void Project::updateModifiedTimestamp() noexcept {
    modifiedTimestamp_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

std::shared_ptr<Track> Project::addTrack(TrackType type, const std::string& name) {
    std::string trackId = "track_" + juce::Uuid().toString().toStdString().substr(0, 8);
    std::string trackName = name;
    if (trackName.empty()) {
        int idx = static_cast<int>(tracks_.size()) + 1;
        trackName = std::string(getTrackTypeName(type)) + " " + std::to_string(idx);
    }

    auto track = std::make_shared<Track>(trackId, trackName, type);
    tracks_.push_back(track);
    if (selectedTrackId_.empty()) {
        selectedTrackId_ = trackId;
    }
    updateModifiedTimestamp();
    return track;
}

bool Project::removeTrack(const std::string& trackId) {
    auto it = std::find_if(tracks_.begin(), tracks_.end(),
        [&trackId](const std::shared_ptr<Track>& t) { return t && t->getId() == trackId; });
    if (it != tracks_.end()) {
        tracks_.erase(it);
        if (selectedTrackId_ == trackId) {
            selectedTrackId_ = tracks_.empty() ? "" : tracks_.front()->getId();
        }
        updateModifiedTimestamp();
        return true;
    }
    return false;
}

std::shared_ptr<Track> Project::getTrack(const std::string& trackId) const {
    for (const auto& t : tracks_) {
        if (t && t->getId() == trackId) return t;
    }
    return nullptr;
}

std::shared_ptr<Track> Project::duplicateTrack(const std::string& trackId) {
    auto src = getTrack(trackId);
    if (!src) return nullptr;

    auto newTrack = addTrack(src->getType(), src->getName() + " Copy");
    newTrack->setVolume(src->getVolume());
    newTrack->setPan(src->getPan());
    newTrack->setColor(src->getColor());

    // Duplicate regions
    for (const auto& r : src->getRegions()) {
        if (!r) continue;
        std::string newRegId = "reg_" + juce::Uuid().toString().toStdString().substr(0, 8);
        auto newReg = std::make_shared<Region>(newRegId, newTrack->getId(), r->getName(),
                                               r->getType(), r->getStartTick(), r->getLengthTicks());
        newReg->setColor(r->getColor());
        newReg->setGainDb(r->getGainDb());
        newReg->setNotes(r->getNotes());
        newReg->setWaveformPeaks(r->getWaveformPeaks());
        newTrack->addRegion(std::move(newReg));
    }

    updateModifiedTimestamp();
    return newTrack;
}

void Project::moveTrack(size_t fromIndex, size_t toIndex) {
    if (fromIndex < tracks_.size() && toIndex < tracks_.size() && fromIndex != toIndex) {
        auto t = tracks_[fromIndex];
        tracks_.erase(tracks_.begin() + fromIndex);
        tracks_.insert(tracks_.begin() + toIndex, t);
        updateModifiedTimestamp();
    }
}

void Project::selectTrack(const std::string& trackId) {
    selectedTrackId_ = trackId;
}

std::shared_ptr<Track> Project::getSelectedTrack() const {
    return getTrack(selectedTrackId_);
}

void Project::selectRegion(const std::string& regionId) {
    selectedRegionId_ = regionId;
    for (auto& t : tracks_) {
        if (!t) continue;
        for (auto& r : t->getRegions()) {
            if (r) {
                bool isTarget = (r->getId() == regionId);
                r->setSelected(isTarget);
                if (isTarget) {
                    selectedTrackId_ = t->getId();
                }
            }
        }
    }
}

std::shared_ptr<Region> Project::getSelectedRegion() const {
    return findRegion(selectedRegionId_);
}

void Project::clearRegionSelection() {
    selectedRegionId_.clear();
    for (auto& t : tracks_) {
        if (!t) continue;
        for (auto& r : t->getRegions()) {
            if (r) r->setSelected(false);
        }
    }
}

std::shared_ptr<Region> Project::findRegion(const std::string& regionId) const {
    if (regionId.empty()) return nullptr;
    for (const auto& t : tracks_) {
        if (!t) continue;
        for (const auto& r : t->getRegions()) {
            if (r && r->getId() == regionId) return r;
        }
    }
    return nullptr;
}

std::shared_ptr<Region> Project::splitSelectedRegionAtPlayhead() {
    auto region = getSelectedRegion();
    if (!region) return nullptr;

    auto track = getTrack(region->getTrackId());
    if (!track) return nullptr;

    std::string newId = "reg_" + juce::Uuid().toString().toStdString().substr(0, 8);
    auto newRightRegion = track->splitRegion(region->getId(), playheadTick_, newId);
    if (newRightRegion) {
        selectRegion(newRightRegion->getId());
        updateModifiedTimestamp();
    }
    return newRightRegion;
}

bool Project::deleteSelectedRegions() {
    if (selectedRegionId_.empty()) return false;
    for (auto& t : tracks_) {
        if (t && t->removeRegion(selectedRegionId_)) {
            selectedRegionId_.clear();
            updateModifiedTimestamp();
            return true;
        }
    }
    return false;
}

std::shared_ptr<Region> Project::duplicateSelectedRegion() {
    auto r = getSelectedRegion();
    if (!r) return nullptr;
    auto t = getTrack(r->getTrackId());
    if (!t) return nullptr;

    std::string newId = "reg_" + juce::Uuid().toString().toStdString().substr(0, 8);
    int64_t nextStart = r->getEndTick();
    auto newReg = std::make_shared<Region>(newId, t->getId(), r->getName() + " (Copy)",
                                           r->getType(), nextStart, r->getLengthTicks());
    newReg->setColor(r->getColor());
    newReg->setGainDb(r->getGainDb());
    newReg->setNotes(r->getNotes());
    newReg->setWaveformPeaks(r->getWaveformPeaks());
    t->addRegion(newReg);
    selectRegion(newId);
    updateModifiedTimestamp();
    return newReg;
}

bool Project::moveRegion(const std::string& regionId, const std::string& targetTrackId, int64_t newStartTick) {
    auto reg = findRegion(regionId);
    if (!reg) return false;

    auto currentTrack = getTrack(reg->getTrackId());
    auto targetTrack = getTrack(targetTrackId);
    if (!currentTrack || !targetTrack) return false;

    if (currentTrack->getId() != targetTrack->getId()) {
        // Move across tracks
        currentTrack->removeRegion(regionId);
        reg->setTrackId(targetTrack->getId());
        reg->setStartTick(newStartTick);
        targetTrack->addRegion(reg);
    } else {
        // Move within track
        reg->setStartTick(newStartTick);
    }

    updateModifiedTimestamp();
    return true;
}

void Project::initializeDefaultArrangement() {
    tracks_.clear();

    // 1. Audio Track: Lead Vocal
    auto voxTrack = addTrack(TrackType::Audio, "Audio 1 (Lead Vocal)");
    voxTrack->setColor(0xff2e86ab); // Logic Audio Blue
    auto voxReg1 = std::make_shared<Region>(
        "reg_vox_01", voxTrack->getId(), "Lead Vocal Verse", RegionType::Audio, 0, kTicksPerBeat * 16);
    voxReg1->generateDemoWaveform(160, 4.2f);
    voxTrack->addRegion(voxReg1);

    auto voxReg2 = std::make_shared<Region>(
        "reg_vox_02", voxTrack->getId(), "Lead Vocal Chorus", RegionType::Audio, kTicksPerBeat * 16, kTicksPerBeat * 16);
    voxReg2->generateDemoWaveform(160, 5.8f);
    voxTrack->addRegion(voxReg2);

    // 2. Instrument Track: Classic Electric Piano
    auto pianoTrack = addTrack(TrackType::SoftwareInstrument, "Inst 1 (Classic EP)");
    pianoTrack->setColor(0xff3a7d44); // Logic MIDI Green
    auto pianoReg = std::make_shared<Region>(
        "reg_piano_01", pianoTrack->getId(), "EP Chords", RegionType::Midi, 0, kTicksPerBeat * 32);
    // Add real MIDI chords
    pianoReg->addNote({60, 0, 960 * 2, 95});    // C3
    pianoReg->addNote({64, 0, 960 * 2, 90});    // E3
    pianoReg->addNote({67, 0, 960 * 2, 92});    // G3
    pianoReg->addNote({62, 960 * 2, 960 * 2, 98}); // D3
    pianoReg->addNote({65, 960 * 2, 960 * 2, 94}); // F3
    pianoReg->addNote({69, 960 * 2, 960 * 2, 96}); // A3
    pianoReg->addNote({57, 960 * 4, 960 * 2, 92}); // A2
    pianoReg->addNote({60, 960 * 4, 960 * 2, 88}); // C3
    pianoReg->addNote({64, 960 * 4, 960 * 2, 90}); // E3
    pianoReg->addNote({53, 960 * 6, 960 * 2, 95}); // F2
    pianoReg->addNote({57, 960 * 6, 960 * 2, 91}); // A2
    pianoReg->addNote({60, 960 * 6, 960 * 2, 93}); // C3
    pianoTrack->addRegion(pianoReg);

    // 3. Instrument Track: Analog Bass
    auto bassTrack = addTrack(TrackType::SoftwareInstrument, "Inst 2 (Analog Bass)");
    bassTrack->setColor(0xff705898); // Slate Purple
    auto bassReg = std::make_shared<Region>(
        "reg_bass_01", bassTrack->getId(), "Bassline Riff", RegionType::Midi, 0, kTicksPerBeat * 32);
    for (int b = 0; b < 8; ++b) {
        bassReg->addNote({36, b * 960 * 4, 480, 110});        // C1
        bassReg->addNote({36, b * 960 * 4 + 720, 240, 105});
        bassReg->addNote({38, b * 960 * 4 + 960, 480, 108});  // D1
        bassReg->addNote({43, b * 960 * 4 + 1920, 960, 115}); // G1
    }
    bassTrack->addRegion(bassReg);

    // 4. Audio Track: Drum Loop
    auto drumTrack = addTrack(TrackType::Audio, "Audio 2 (Drum Loop)");
    drumTrack->setColor(0xffb85d3b); // Coral Ochre
    auto drumReg = std::make_shared<Region>(
        "reg_drum_01", drumTrack->getId(), "Studio Drum Groove 120", RegionType::Audio, 0, kTicksPerBeat * 32);
    drumReg->generateDemoWaveform(200, 8.0f);
    drumTrack->addRegion(drumReg);

    selectedTrackId_ = voxTrack->getId();
    selectedRegionId_ = voxReg1->getId();
    voxReg1->setSelected(true);
}

bool Project::isValid() const noexcept {
    return !name_.empty() && tempoBpm_ >= kMinTempoBpm && tempoBpm_ <= kMaxTempoBpm;
}

bool Project::validate(std::string& outError) const {
    if (name_.empty()) {
        outError = "Project name cannot be empty.";
        return false;
    }
    if (tempoBpm_ < kMinTempoBpm || tempoBpm_ > kMaxTempoBpm) {
        outError = "Tempo BPM is out of valid range (20 - 999).";
        return false;
    }
    return true;
}

} // namespace sonora::domain
