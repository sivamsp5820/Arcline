#pragma once

#include "audio/processors/AudioProcessor.h"
#include "audio/transport/AudioClock.h"
#include <atomic>
#include <cmath>
#include <algorithm>
#include <numbers>

namespace arcline::audio {

/**
 * @brief Sample-accurate metronome click synthesizer aligned with AudioClock beats.
 */
class MetronomeProcessor : public AudioProcessor {
public:
    static constexpr float kDefaultGain = 0.35f;

    MetronomeProcessor() = default;

    void prepareToPlay(double sampleRate, int /*maxBlockSize*/) override {
        sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
        accentDecayRate_ = std::exp(-1.0 / (sampleRate_ * 0.020)); // ~20 ms burst
        normalDecayRate_ = std::exp(-1.0 / (sampleRate_ * 0.012)); // ~12 ms burst
        accentPhaseInc_  = (2.0 * std::numbers::pi * 1600.0) / sampleRate_; // High pitch: 1600 Hz
        normalPhaseInc_  = (2.0 * std::numbers::pi * 800.0) / sampleRate_;  // Normal pitch: 800 Hz
        resetClick();
    }

    void releaseResources() override {
        resetClick();
    }

    void processBlock(float* const* outputChannels, int numChannels, int numSamples) override {
        // Fallback overload; use processBlockWithClock for timing alignment
        processBlockWithClock(outputChannels, numChannels, numSamples, nullptr, false);
    }

    void reset() noexcept {
        resetClick();
        lastProcessedSample_ = -1;
    }

    /**
     * @brief Real-time callback synthesizing metronome clicks aligned to musical beats.
     */
    void processBlockWithClock(float* const* outputChannels, int numChannels, int numSamples,
                               const AudioClock* clock, bool isPlaying) noexcept {
        if (numChannels <= 0 || numSamples <= 0 || !outputChannels) return;

        bool enabled = isEnabled_.load(std::memory_order_relaxed);
        float masterGain = gain_.load(std::memory_order_relaxed);

        if (!enabled || !isPlaying || !clock) {
            // Decay any remaining active click to silence smoothly
            if (activeClickEnvelope_ > 0.0001f) {
                renderActiveClick(outputChannels, numChannels, numSamples, masterGain);
            }
            lastProcessedSample_ = -1;
            return;
        }

        double samplesPerBeat = clock->getSamplesPerBeat();
        if (samplesPerBeat <= 0.0) return;

        int64_t startSample = clock->getCurrentSamplePosition();
        if (lastProcessedSample_ >= 0 && startSample != lastProcessedSample_) {
            // Discontinuous transport reposition / locate detected: cut old click
            resetClick();
        }
        lastProcessedSample_ = startSample + numSamples;

        int timeSigNumerator = clock->getTimeSigNumerator();

        for (int i = 0; i < numSamples; ++i) {
            int64_t currentSample = startSample + i;

            // Check if a beat occurs exactly at this sample offset
            int64_t currentBeatIndex = static_cast<int64_t>(std::floor(static_cast<double>(currentSample) / samplesPerBeat));
            int64_t beatSamplePosition = static_cast<int64_t>(std::round(static_cast<double>(currentBeatIndex) * samplesPerBeat));

            if (currentSample == beatSamplePosition) {
                // Downbeat accent (beat 1 of measure) vs regular beat
                bool isDownbeat = (currentBeatIndex % timeSigNumerator == 0);
                triggerClick(isDownbeat);
            }

            // Synthesize active click sample
            if (activeClickEnvelope_ > 0.0001f) {
                float clickSample = static_cast<float>(std::sin(activeClickPhase_)) * 
                                    activeClickEnvelope_ * masterGain;

                for (int ch = 0; ch < numChannels; ++ch) {
                    if (outputChannels[ch]) {
                        outputChannels[ch][i] += clickSample;
                    }
                }

                // Advance phase and exponential decay
                activeClickPhase_ += activePhaseInc_;
                activeClickEnvelope_ *= activeDecayRate_;
            }
        }
    }

    void setEnabled(bool enabled) noexcept {
        isEnabled_.store(enabled, std::memory_order_relaxed);
    }

    bool isEnabled() const noexcept {
        return isEnabled_.load(std::memory_order_relaxed);
    }

    void setGain(float gain) noexcept {
        gain_.store(std::clamp(gain, 0.0f, 1.0f), std::memory_order_relaxed);
    }

    float getGain() const noexcept {
        return gain_.load(std::memory_order_relaxed);
    }

private:
    void triggerClick(bool isDownbeat) noexcept {
        activePhaseInc_ = isDownbeat ? accentPhaseInc_ : normalPhaseInc_;
        activeDecayRate_ = isDownbeat ? accentDecayRate_ : normalDecayRate_;
        activeClickEnvelope_ = isDownbeat ? 1.0f : 0.70f;
        activeClickPhase_ = 0.0;
    }

    void renderActiveClick(float* const* outputChannels, int numChannels, int numSamples, float masterGain) noexcept {
        for (int i = 0; i < numSamples; ++i) {
            if (activeClickEnvelope_ <= 0.0001f) break;

            float clickSample = static_cast<float>(std::sin(activeClickPhase_)) * 
                                activeClickEnvelope_ * masterGain;

            for (int ch = 0; ch < numChannels; ++ch) {
                if (outputChannels[ch]) {
                    outputChannels[ch][i] += clickSample;
                }
            }

            activeClickPhase_ += activePhaseInc_;
            activeClickEnvelope_ *= activeDecayRate_;
        }
    }

    void resetClick() noexcept {
        activeClickEnvelope_ = 0.0f;
        activeClickPhase_ = 0.0;
    }

    double sampleRate_{48000.0};
    double accentDecayRate_{0.998};
    double normalDecayRate_{0.996};
    double accentPhaseInc_{0.209};
    double normalPhaseInc_{0.104};

    float activeClickEnvelope_{0.0f};
    double activeClickPhase_{0.0};
    double activePhaseInc_{0.0};
    double activeDecayRate_{0.0};

    std::atomic<bool> isEnabled_{false};
    std::atomic<float> gain_{kDefaultGain};
    int64_t lastProcessedSample_{-1};
};

} // namespace arcline::audio
