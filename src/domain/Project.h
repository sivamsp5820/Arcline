#pragma once

#include "domain/Track.h"
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <chrono>

namespace sonora::domain {

class Project {
public:
    static constexpr int kCurrentSchemaVersion = 2;
    static constexpr double kMinTempoBpm = 20.0;
    static constexpr double kMaxTempoBpm = 999.0;
    static constexpr double kDefaultTempoBpm = 120.0;
    static constexpr double kDefaultSampleRate = 48000.0;
    static constexpr int64_t kTicksPerBeat = 960;

    Project();
    Project(std::string name, std::string filePath);

    const std::string& getId() const noexcept { return id_; }
    void setId(std::string id) { id_ = std::move(id); }

    const std::string& getName() const noexcept { return name_; }
    void setName(std::string name) { name_ = std::move(name); }

    const std::string& getFilePath() const noexcept { return filePath_; }
    void setFilePath(std::string path) { filePath_ = std::move(path); }

    double getSampleRate() const noexcept { return sampleRate_; }
    void setSampleRate(double rate) noexcept { sampleRate_ = rate; }

    double getTempoBpm() const noexcept { return tempoBpm_; }
    bool setTempoBpm(double bpm) noexcept;

    int getTimeSigNumerator() const noexcept { return timeSigNumerator_; }
    int getTimeSigDenominator() const noexcept { return timeSigDenominator_; }
    bool setTimeSignature(int numerator, int denominator) noexcept;

    const std::string& getKeySignature() const noexcept { return keySignature_; }
    void setKeySignature(std::string key) { keySignature_ = std::move(key); }

    int getSchemaVersion() const noexcept { return schemaVersion_; }
    void setSchemaVersion(int version) noexcept { schemaVersion_ = version; }

    int64_t getCreatedTimestamp() const noexcept { return createdTimestamp_; }
    void setCreatedTimestamp(int64_t ts) noexcept { createdTimestamp_ = ts; }

    int64_t getModifiedTimestamp() const noexcept { return modifiedTimestamp_; }
    void setModifiedTimestamp(int64_t ts) noexcept { modifiedTimestamp_ = ts; }
    void updateModifiedTimestamp() noexcept;

    // Transport & Playhead State
    int64_t getPlayheadTick() const noexcept { return playheadTick_; }
    void setPlayheadTick(int64_t tick) noexcept { playheadTick_ = (tick >= 0 ? tick : 0); }

    int64_t getCycleStartTick() const noexcept { return cycleStartTick_; }
    int64_t getCycleEndTick() const noexcept { return cycleEndTick_; }
    bool isCycleActive() const noexcept { return isCycleActive_; }
    void setCycleRange(int64_t startTick, int64_t endTick) noexcept;
    void setCycleActive(bool active) noexcept { isCycleActive_ = active; }

    bool isPlaying() const noexcept { return isPlaying_; }
    void setPlaying(bool playing) noexcept { isPlaying_ = playing; }

    bool isRecording() const noexcept { return isRecording_; }
    void setRecording(bool rec) noexcept { isRecording_ = rec; }

    // Master bus
    float getMasterVolume() const noexcept { return masterVolume_; }
    void setMasterVolume(float vol) noexcept { masterVolume_ = vol; }

    float getMasterPan() const noexcept { return masterPan_; }
    void setMasterPan(float pan) noexcept { masterPan_ = pan; }

    // Track collection
    const std::vector<std::shared_ptr<Track>>& getTracks() const noexcept { return tracks_; }
    std::vector<std::shared_ptr<Track>>& getTracks() noexcept { return tracks_; }

    std::shared_ptr<Track> addTrack(TrackType type, const std::string& name = "");
    bool removeTrack(const std::string& trackId);
    std::shared_ptr<Track> getTrack(const std::string& trackId) const;
    std::shared_ptr<Track> duplicateTrack(const std::string& trackId);
    void moveTrack(size_t fromIndex, size_t toIndex);

    // Selection
    const std::string& getSelectedTrackId() const noexcept { return selectedTrackId_; }
    void selectTrack(const std::string& trackId);
    std::shared_ptr<Track> getSelectedTrack() const;

    const std::string& getSelectedRegionId() const noexcept { return selectedRegionId_; }
    void selectRegion(const std::string& regionId);
    std::shared_ptr<Region> getSelectedRegion() const;
    void clearRegionSelection();

    // Region Operations
    std::shared_ptr<Region> findRegion(const std::string& regionId) const;
    std::shared_ptr<Region> splitSelectedRegionAtPlayhead();
    bool deleteSelectedRegions();
    std::shared_ptr<Region> duplicateSelectedRegion();
    bool moveRegion(const std::string& regionId, const std::string& targetTrackId, int64_t newStartTick);

    // Populates standard demo tracks for initial testing
    void initializeDefaultArrangement();

    bool isValid() const noexcept;
    bool validate(std::string& outError) const;

private:
    std::string id_;
    std::string name_;
    std::string filePath_;
    double sampleRate_{kDefaultSampleRate};
    double tempoBpm_{kDefaultTempoBpm};
    int timeSigNumerator_{4};
    int timeSigDenominator_{4};
    std::string keySignature_{"C Major"};
    int schemaVersion_{kCurrentSchemaVersion};
    int64_t createdTimestamp_{0};
    int64_t modifiedTimestamp_{0};

    int64_t playheadTick_{0};
    int64_t cycleStartTick_{0};
    int64_t cycleEndTick_{kTicksPerBeat * 16}; // 4 bars cycle default
    bool isCycleActive_{false};
    bool isPlaying_{false};
    bool isRecording_{false};

    float masterVolume_{1.0f};
    float masterPan_{0.0f};

    std::string selectedTrackId_;
    std::string selectedRegionId_;
    std::vector<std::shared_ptr<Track>> tracks_;
};

} // namespace sonora::domain
