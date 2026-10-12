#pragma once

#include <cstdint>

namespace arcline::future_audio {

/**
 * @brief High-level transport states.
 */
enum class TransportState {
    Stopped,
    Playing,
    Recording,
    Paused
};

/**
 * @brief Musical time representation for display and sequencing.
 */
struct MusicalPosition {
    int bar = 1;
    int beat = 1;
    int division = 1;
    int tick = 0;
    int64_t totalTicks = 0;
    int64_t totalSamples = 0;
    double seconds = 0.0;
};

/**
 * @brief Extension point interface for the future sample-accurate transport controller (Phase 2+).
 * 
 * Boundaries and Contracts:
 * - Transport timing is sample-accurate and clocked directly from the audio rendering process.
 * - Transport state mutations (play, stop, locate) from UI are posted via lock-free atomic commands.
 * - UI updates query an atomic snapshot of current sample/musical position to eliminate jitter.
 * - Loop/cycle logic seamlessly wraps sample positions within the audio render block without audio glitches.
 */
class ITransportController {
public:
    virtual ~ITransportController() = default;

    virtual void play() = 0;
    virtual void stop() = 0;
    virtual void record() = 0;
    virtual void pause() = 0;
    virtual void locateToSample(int64_t sampleIndex) = 0;
    virtual void locateToTick(int64_t tick) = 0;

    virtual void setLooping(bool loopEnabled) = 0;
    virtual void setLoopRange(int64_t startTick, int64_t endTick) = 0;

    virtual TransportState getState() const noexcept = 0;
    virtual bool isPlaying() const noexcept = 0;
    virtual bool isRecording() const noexcept = 0;
    virtual bool isLooping() const noexcept = 0;

    virtual int64_t getCurrentSamplePosition() const noexcept = 0;
    virtual int64_t getCurrentTickPosition() const noexcept = 0;
    virtual double getCurrentTimeSeconds() const noexcept = 0;
    virtual MusicalPosition getMusicalPosition() const noexcept = 0;

    virtual double getTempoBpm() const noexcept = 0;
    virtual void setTempoBpm(double bpm) = 0;

    virtual void getTimeSignature(int& numerator, int& denominator) const noexcept = 0;
    virtual void setTimeSignature(int numerator, int denominator) = 0;
};

} // namespace arcline::future_audio

namespace sonora::future_audio {
    using TransportState = arcline::future_audio::TransportState;
    using MusicalPosition = arcline::future_audio::MusicalPosition;
    using ITransportController = arcline::future_audio::ITransportController;
}
