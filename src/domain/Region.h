#pragma once

#include "domain/MidiNote.h"
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <utility>

namespace sonora::domain {

enum class RegionType {
    Audio,
    Midi
};

inline std::string_view getRegionTypeName(RegionType type) noexcept {
    switch (type) {
        case RegionType::Audio: return "Audio";
        case RegionType::Midi:  return "MIDI";
    }
    return "Unknown";
}

class Region {
public:
    static constexpr int64_t kTicksPerBeat = 960;

    Region(std::string id, std::string trackId, std::string name, RegionType type,
           int64_t startTick, int64_t lengthTicks);

    const std::string& getId() const noexcept { return id_; }
    void setId(std::string id) { id_ = std::move(id); }

    const std::string& getTrackId() const noexcept { return trackId_; }
    void setTrackId(std::string trackId) { trackId_ = std::move(trackId); }

    const std::string& getName() const noexcept { return name_; }
    void setName(std::string name) { name_ = std::move(name); }

    RegionType getType() const noexcept { return type_; }

    int64_t getStartTick() const noexcept { return startTick_; }
    void setStartTick(int64_t tick) noexcept { startTick_ = (tick >= 0 ? tick : 0); }

    int64_t getLengthTicks() const noexcept { return lengthTicks_; }
    void setLengthTicks(int64_t length) noexcept { lengthTicks_ = (length > 0 ? length : 1); }

    int64_t getEndTick() const noexcept { return startTick_ + lengthTicks_; }

    int64_t getOffsetTicks() const noexcept { return offsetTicks_; }
    void setOffsetTicks(int64_t offset) noexcept { offsetTicks_ = offset; }

    bool isMuted() const noexcept { return isMuted_; }
    void setMuted(bool muted) noexcept { isMuted_ = muted; }

    bool isLooped() const noexcept { return isLooped_; }
    void setLooped(bool looped) noexcept { isLooped_ = looped; }

    bool isSelected() const noexcept { return isSelected_; }
    void setSelected(bool selected) noexcept { isSelected_ = selected; }

    uint32_t getColor() const noexcept { return colorArgb_; }
    void setColor(uint32_t argb) noexcept { colorArgb_ = argb; }

    float getGainDb() const noexcept { return gainDb_; }
    void setGainDb(float gain) noexcept { gainDb_ = gain; }

    int64_t getFadeInTicks() const noexcept { return fadeInTicks_; }
    void setFadeInTicks(int64_t ticks) noexcept { fadeInTicks_ = ticks; }

    int64_t getFadeOutTicks() const noexcept { return fadeOutTicks_; }
    void setFadeOutTicks(int64_t ticks) noexcept { fadeOutTicks_ = ticks; }

    // Audio file path (portable media reference)
    const std::string& getAudioFilePath() const noexcept { return audioFilePath_; }
    void setAudioFilePath(std::string path) { audioFilePath_ = std::move(path); }

    // Waveform peak data (Audio)
    const std::vector<std::pair<float, float>>& getWaveformPeaks() const noexcept { return waveformPeaks_; }
    void setWaveformPeaks(std::vector<std::pair<float, float>> peaks) { waveformPeaks_ = std::move(peaks); }
    void generateDemoWaveform(int numPoints = 120, float frequency = 4.0f);

    // MIDI Note data (MIDI)
    const std::vector<MidiNote>& getNotes() const noexcept { return notes_; }
    std::vector<MidiNote>& getNotes() noexcept { return notes_; }
    void setNotes(std::vector<MidiNote> notes) { notes_ = std::move(notes); }
    void addNote(const MidiNote& note);
    bool removeNote(size_t index);
    void quantizeNotes(int64_t gridTicks);

    // Interactive editing operations
    void trimLeft(int64_t deltaTicks);
    void trimRight(int64_t deltaTicks);
    std::shared_ptr<Region> split(int64_t splitTick, const std::string& newRegionId);

    // Validation
    bool isValid() const noexcept;

private:
    std::string id_;
    std::string trackId_;
    std::string name_;
    RegionType type_;
    int64_t startTick_{0};
    int64_t lengthTicks_{kTicksPerBeat * 4}; // Default 1 bar
    int64_t offsetTicks_{0};
    bool isMuted_{false};
    bool isLooped_{false};
    bool isSelected_{false};
    uint32_t colorArgb_{0xff2e86ab}; // Default Logic blue
    float gainDb_{0.0f};
    int64_t fadeInTicks_{0};
    int64_t fadeOutTicks_{0};

    // Audio peak cache
    std::string audioFilePath_;
    std::vector<std::pair<float, float>> waveformPeaks_;

    // MIDI event store
    std::vector<MidiNote> notes_;
};

} // namespace sonora::domain
