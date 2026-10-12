#pragma once

#include "domain/Region.h"
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <string_view>

namespace sonora::domain {

enum class TrackType {
    Audio,
    SoftwareInstrument,
    AuxBus
};

inline std::string_view getTrackTypeName(TrackType type) noexcept {
    switch (type) {
        case TrackType::Audio:              return "Audio";
        case TrackType::SoftwareInstrument: return "Software Instrument";
        case TrackType::AuxBus:             return "Aux Bus";
    }
    return "Unknown";
}

enum class AutomationMode {
    Read,
    Touch,
    Latch,
    Write,
    Off
};

inline std::string_view getAutomationModeName(AutomationMode mode) noexcept {
    switch (mode) {
        case AutomationMode::Read:  return "Read";
        case AutomationMode::Touch: return "Touch";
        case AutomationMode::Latch: return "Latch";
        case AutomationMode::Write: return "Write";
        case AutomationMode::Off:   return "Off";
    }
    return "Off";
}

class Track {
public:
    static constexpr int kDefaultTrackHeight = 84;
    static constexpr int kMinTrackHeight = 44;
    static constexpr int kMaxTrackHeight = 240;

    Track(std::string id, std::string name, TrackType type);

    const std::string& getId() const noexcept { return id_; }
    void setId(std::string id) { id_ = std::move(id); }

    const std::string& getName() const noexcept { return name_; }
    void setName(std::string name) { name_ = std::move(name); }

    TrackType getType() const noexcept { return type_; }
    void setType(TrackType type) noexcept { type_ = type; }

    float getVolume() const noexcept { return volume_; }
    void setVolume(float vol) noexcept { volume_ = (vol < 0.0f ? 0.0f : (vol > 2.0f ? 2.0f : vol)); }

    float getPan() const noexcept { return pan_; }
    void setPan(float pan) noexcept { pan_ = (pan < -1.0f ? -1.0f : (pan > 1.0f ? 1.0f : pan)); }

    bool isMuted() const noexcept { return isMuted_; }
    void setMuted(bool muted) noexcept { isMuted_ = muted; }

    bool isSoloed() const noexcept { return isSoloed_; }
    void setSoloed(bool soloed) noexcept { isSoloed_ = soloed; }

    bool isArmed() const noexcept { return isArmed_; }
    void setArmed(bool armed) noexcept { isArmed_ = armed; }

    bool isInputMonitored() const noexcept { return isInputMonitored_; }
    void setInputMonitored(bool monitored) noexcept { isInputMonitored_ = monitored; }

    AutomationMode getAutomationMode() const noexcept { return automationMode_; }
    void setAutomationMode(AutomationMode mode) noexcept { automationMode_ = mode; }

    uint32_t getColor() const noexcept { return colorArgb_; }
    void setColor(uint32_t argb) noexcept { colorArgb_ = argb; }

    int getHeight() const noexcept { return height_; }
    void setHeight(int h) noexcept;

    // Region management
    const std::vector<std::shared_ptr<Region>>& getRegions() const noexcept { return regions_; }
    std::vector<std::shared_ptr<Region>>& getRegions() noexcept { return regions_; }
    void addRegion(std::shared_ptr<Region> region);
    bool removeRegion(const std::string& regionId);
    std::shared_ptr<Region> getRegion(const std::string& regionId) const;
    void clearRegions() noexcept { regions_.clear(); }

    std::shared_ptr<Region> splitRegion(const std::string& regionId, int64_t splitTick,
                                        const std::string& newRegionId);

    bool isValid() const noexcept;

private:
    std::string id_;
    std::string name_;
    TrackType type_;
    float volume_{1.0f}; // 1.0f corresponds to unity gain (0.0 dB)
    float pan_{0.0f};    // 0.0f is Center (-1.0f Left, +1.0f Right)
    bool isMuted_{false};
    bool isSoloed_{false};
    bool isArmed_{false};
    bool isInputMonitored_{false};
    AutomationMode automationMode_{AutomationMode::Read};
    uint32_t colorArgb_{0xff2e86ab};
    int height_{kDefaultTrackHeight};

    std::vector<std::shared_ptr<Region>> regions_;
};

} // namespace sonora::domain
