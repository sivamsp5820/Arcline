#pragma once

#include "future_audio/ITransportController.h"
#include <atomic>
#include <cstdint>
#include <algorithm>
#include <cmath>

namespace arcline::audio {

/**
 * @brief High-resolution sample-accurate musical clock clocked directly by audio buffers.
 */
class AudioClock {
public:
    static constexpr int64_t kTicksPerBeat = 960; // Standard high-resolution MIDI tick base

    AudioClock() = default;

    void setSampleRate(double sampleRate) noexcept {
        sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
        updateTimingConstants();
    }

    double getSampleRate() const noexcept {
        return sampleRate_;
    }

    void setTempoBpm(double bpm) noexcept {
        tempoBpm_ = std::clamp(bpm, 20.0, 999.0);
        updateTimingConstants();
    }

    double getTempoBpm() const noexcept {
        return tempoBpm_;
    }

    void setTimeSignature(int numerator, int denominator) noexcept {
        timeSigNumerator_ = std::clamp(numerator, 1, 32);
        timeSigDenominator_ = (denominator == 2 || denominator == 4 || denominator == 8 || denominator == 16) 
                              ? denominator : 4;
    }

    int getTimeSigNumerator() const noexcept { return timeSigNumerator_; }
    int getTimeSigDenominator() const noexcept { return timeSigDenominator_; }

    void setLoopRange(int64_t startTick, int64_t endTick) noexcept {
        if (startTick >= 0 && endTick > startTick) {
            loopStartTick_ = startTick;
            loopEndTick_ = endTick;
            loopStartSample_ = tickToSample(startTick);
            loopEndSample_ = tickToSample(endTick);
        }
    }

    void setLooping(bool enabled) noexcept {
        isLooping_ = enabled;
    }

    bool isLooping() const noexcept {
        return isLooping_;
    }

    void locateToSample(int64_t sampleIndex) noexcept {
        samplePosition_ = std::max<int64_t>(0, sampleIndex);
    }

    void locateToTick(int64_t tick) noexcept {
        locateToSample(tickToSample(std::max<int64_t>(0, tick)));
    }

    int64_t getCurrentSamplePosition() const noexcept {
        return samplePosition_;
    }

    /**
     * @brief Real-time audio callback advance.
     * Advances sample position by numSamples, applying loop wrap within the block if enabled.
     */
    void advanceSamples(int numSamples) noexcept {
        if (numSamples <= 0) return;

        int64_t target = samplePosition_ + numSamples;

        if (isLooping_ && loopEndSample_ > loopStartSample_) {
            if (samplePosition_ < loopEndSample_ && target >= loopEndSample_) {
                // Wrap to start of loop plus remaining samples in this block
                int64_t overflow = target - loopEndSample_;
                int64_t loopLength = loopEndSample_ - loopStartSample_;
                samplePosition_ = loopStartSample_ + (overflow % loopLength);
                return;
            }
        }

        samplePosition_ = target;
    }

    /**
     * @brief Convert sample position to musical position (Bar, Beat, Division, Tick).
     */
    future_audio::MusicalPosition getMusicalPosition() const noexcept {
        return sampleToMusicalPosition(samplePosition_);
    }

    future_audio::MusicalPosition sampleToMusicalPosition(int64_t sample) const noexcept {
        future_audio::MusicalPosition pos;
        pos.totalSamples = sample;
        pos.seconds = (sampleRate_ > 0.0) ? (static_cast<double>(sample) / sampleRate_) : 0.0;

        int64_t totalTicks = sampleToTick(sample);
        pos.totalTicks = totalTicks;

        int64_t ticksPerBar = kTicksPerBeat * timeSigNumerator_;
        pos.bar = static_cast<int>(totalTicks / ticksPerBar) + 1;

        int64_t remainderBar = totalTicks % ticksPerBar;
        pos.beat = static_cast<int>(remainderBar / kTicksPerBeat) + 1;

        int64_t remainderBeat = remainderBar % kTicksPerBeat;
        int64_t ticksPerDiv = kTicksPerBeat / 4; // 16th note divisions (240 ticks)
        pos.division = static_cast<int>(remainderBeat / ticksPerDiv) + 1;
        pos.tick = static_cast<int>(remainderBeat % ticksPerDiv);

        return pos;
    }

    int64_t tickToSample(int64_t tick) const noexcept {
        if (tick <= 0 || samplesPerTick_ <= 0.0) return 0;
        return static_cast<int64_t>(std::round(static_cast<double>(tick) * samplesPerTick_));
    }

    int64_t sampleToTick(int64_t sample) const noexcept {
        if (sample <= 0 || samplesPerTick_ <= 0.0) return 0;
        return static_cast<int64_t>(std::round(static_cast<double>(sample) / samplesPerTick_));
    }

    double getSamplesPerBeat() const noexcept {
        return samplesPerBeat_;
    }

    double getSamplesPerTick() const noexcept {
        return samplesPerTick_;
    }

private:
    void updateTimingConstants() noexcept {
        // samples per minute / beats per minute = samples per beat
        samplesPerBeat_ = (sampleRate_ * 60.0) / tempoBpm_;
        samplesPerTick_ = samplesPerBeat_ / static_cast<double>(kTicksPerBeat);

        loopStartSample_ = tickToSample(loopStartTick_);
        loopEndSample_ = tickToSample(loopEndTick_);
    }

    double sampleRate_{48000.0};
    double tempoBpm_{120.0};
    int timeSigNumerator_{4};
    int timeSigDenominator_{4};

    double samplesPerBeat_{24000.0}; // at 48k, 120 bpm -> 0.5s = 24000 samples
    double samplesPerTick_{25.0};    // 24000 / 960 = 25 samples per tick

    int64_t samplePosition_{0};

    bool isLooping_{false};
    int64_t loopStartTick_{0};
    int64_t loopEndTick_{3840};      // 4 bars of 4/4 = 16 beats = 15360 ticks
    int64_t loopStartSample_{0};
    int64_t loopEndSample_{96000};
};

} // namespace arcline::audio
