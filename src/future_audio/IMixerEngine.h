#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace arcline::future_audio {

struct ChannelMeterValues {
    float peakLeft = -100.0f;
    float peakRight = -100.0f;
    float rmsLeft = -100.0f;
    float rmsRight = -100.0f;
    bool clipLeft = false;
    bool clipRight = false;
};

/**
 * @brief Extension point interface for multitrack mixing, routing, and summing (Phase 5).
 * 
 * Boundaries and Contracts:
 * - Deterministic, topological execution of track processing -> send buses -> master bus.
 * - Smooth parameter interpolation (de-zippering) on all faders and pan knobs to avoid clicks.
 * - Solo logic implements Solo-In-Place (SIP) and non-destructive AFL/PFL solo buses.
 * - Lock-free metering output: meter snapshots are pushed to a ring buffer consumed by the UI thread at 60 Hz.
 * - No dynamic memory reallocation during active audio playback.
 */
class IMixerEngine {
public:
    virtual ~IMixerEngine() = default;

    virtual void setTrackGain(const std::string& trackId, float linearGain) = 0;
    virtual void setTrackPan(const std::string& trackId, float panPosition) = 0; // -1.0 (Left) to +1.0 (Right)
    virtual void setTrackMuted(const std::string& trackId, bool isMuted) = 0;
    virtual void setTrackSoloed(const std::string& trackId, bool isSoloed) = 0;

    virtual void setMasterGain(float linearGain) = 0;
    virtual void setMasterPan(float panPosition) = 0;

    virtual void setSendLevel(const std::string& sourceTrackId, const std::string& busId, float sendLevel) = 0;

    virtual ChannelMeterValues getTrackMeterValues(const std::string& trackId) const noexcept = 0;
    virtual ChannelMeterValues getMasterMeterValues() const noexcept = 0;
};

} // namespace arcline::future_audio

namespace sonora::future_audio {
    using ChannelMeterValues = arcline::future_audio::ChannelMeterValues;
    using IMixerEngine = arcline::future_audio::IMixerEngine;
}
