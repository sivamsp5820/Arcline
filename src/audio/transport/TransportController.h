#pragma once

#include "future_audio/ITransportController.h"
#include "audio/transport/AudioClock.h"
#include <atomic>
#include <memory>
#include <functional>

namespace arcline::audio {

/**
 * @brief Thread-safe transport controller bridging UI playback commands and audio thread clock.
 */
class TransportController : public future_audio::ITransportController {
public:
    TransportController();
    ~TransportController() override = default;

    // Command methods (UI or application thread callable)
    void play() override;
    void stop() override;
    void record() override;
    void pause() override;
    void returnToStart();
    void locateToSample(int64_t sampleIndex) override;
    void locateToTick(int64_t tick) override;

    void setLooping(bool loopEnabled) override;
    void setLoopRange(int64_t startTick, int64_t endTick) override;

    // Real-time audio callback methods (Audio thread only)
    void prepareToPlay(double sampleRate, int maxBlockSize);
    void advanceClockInCallback(int numSamples);

    // Timing and state queries (Lock-free, callable from any thread)
    future_audio::TransportState getState() const noexcept override;
    bool isPlaying() const noexcept override;
    bool isRecording() const noexcept override;
    bool isLooping() const noexcept override;

    int64_t getCurrentSamplePosition() const noexcept override;
    int64_t getCurrentTickPosition() const noexcept override;
    double getCurrentTimeSeconds() const noexcept override;

    future_audio::MusicalPosition getMusicalPosition() const noexcept override;

    void setTempoBpm(double bpm) override;
    double getTempoBpm() const noexcept override;

    void getTimeSignature(int& numerator, int& denominator) const noexcept override;
    void setTimeSignature(int numerator, int denominator) override;
    int getTimeSigNumerator() const noexcept;
    int getTimeSigDenominator() const noexcept;

    AudioClock& getClock() noexcept { return clock_; }
    const AudioClock& getClock() const noexcept { return clock_; }

    void addStateChangeListener(std::function<void(future_audio::TransportState)> listener);

private:
    void notifyStateChanged(future_audio::TransportState newState);

    AudioClock clock_;

    std::atomic<future_audio::TransportState> state_{future_audio::TransportState::Stopped};
    std::atomic<bool> isLooping_{false};

    // Pending locate command for atomic consumption on audio thread
    std::atomic<int64_t> pendingLocateSample_{-1};

    std::vector<std::function<void(future_audio::TransportState)>> listeners_;
};

} // namespace arcline::audio
