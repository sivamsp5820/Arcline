#pragma once

#include "audio/processors/AudioProcessor.h"
#include <atomic>
#include <cmath>
#include <algorithm>
#include <numbers>

namespace arcline::audio {

/**
 * @brief Continuous-phase sine wave generator with de-zippered gain ramping for hardware verification.
 */
class TestToneGenerator : public AudioProcessor {
public:
    static constexpr float kDefaultFrequency = 440.0f; // Concert A
    static constexpr float kDefaultAmplitude = 0.20f;  // -14 dBFS (safe hearing level)
    static constexpr float kMaxSafeAmplitude = 0.80f;  // Prevent accidental 0 dBFS blast
    static constexpr float kGainRampPerSample = 0.005f; // ~200 samples fade (~4.2 ms at 48 kHz)

    TestToneGenerator() = default;

    void prepareToPlay(double sampleRate, int /*maxBlockSize*/) override {
        sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
        updatePhaseIncrement();
    }

    void releaseResources() override {
        // No heap resources to free
    }

    void processBlock(float* const* outputChannels, int numChannels, int numSamples) override {
        if (numChannels <= 0 || numSamples <= 0 || !outputChannels) return;

        bool enabled = isEnabled_.load(std::memory_order_relaxed);
        float targetAmp = enabled ? std::clamp(amplitude_.load(std::memory_order_relaxed), 0.0f, kMaxSafeAmplitude) : 0.0f;

        // If completely silent and gain has reached zero, return immediately
        if (!enabled && currentGain_ <= 0.00001f) {
            currentGain_ = 0.0f;
            return;
        }

        // Cache phase state locally
        double phase = phase_;
        double phaseInc = phaseIncrement_;
        float currentGain = currentGain_;
        const double twoPi = 2.0 * std::numbers::pi;

        for (int i = 0; i < numSamples; ++i) {
            // Smoothly ramp gain toward target to prevent clicks
            if (currentGain < targetAmp) {
                currentGain = std::min(targetAmp, currentGain + kGainRampPerSample);
            } else if (currentGain > targetAmp) {
                currentGain = std::max(targetAmp, currentGain - kGainRampPerSample);
            }

            // Generate sine sample with continuous phase
            float sample = currentGain * static_cast<float>(std::sin(phase));

            // Accumulate into all output channels
            for (int ch = 0; ch < numChannels; ++ch) {
                if (outputChannels[ch]) {
                    outputChannels[ch][i] += sample;
                }
            }

            // Advance and wrap phase in [0, 2*pi)
            phase += phaseInc;
            if (phase >= twoPi) {
                phase -= twoPi;
            }
        }

        // Save state back
        phase_ = phase;
        currentGain_ = currentGain;
    }

    // Thread-safe control methods
    void setEnabled(bool enabled) noexcept {
        isEnabled_.store(enabled, std::memory_order_relaxed);
    }

    bool isEnabled() const noexcept {
        return isEnabled_.load(std::memory_order_relaxed);
    }

    void setFrequency(float freqHz) noexcept {
        frequency_.store(std::clamp(freqHz, 20.0f, 20000.0f), std::memory_order_relaxed);
        updatePhaseIncrement();
    }

    float getFrequency() const noexcept {
        return frequency_.load(std::memory_order_relaxed);
    }

    void setAmplitude(float amp) noexcept {
        amplitude_.store(std::clamp(amp, 0.0f, kMaxSafeAmplitude), std::memory_order_relaxed);
    }

    float getAmplitude() const noexcept {
        return amplitude_.load(std::memory_order_relaxed);
    }

    double getPhase() const noexcept {
        return phase_;
    }

    float getCurrentGain() const noexcept {
        return currentGain_;
    }

    void resetPhase() noexcept {
        phase_ = 0.0;
        currentGain_ = 0.0f;
    }

private:
    void updatePhaseIncrement() noexcept {
        float f = frequency_.load(std::memory_order_relaxed);
        phaseIncrement_ = (2.0 * std::numbers::pi * static_cast<double>(f)) / sampleRate_;
    }

    double sampleRate_{48000.0};
    double phase_{0.0};
    double phaseIncrement_{0.0};
    float currentGain_{0.0f};

    std::atomic<bool> isEnabled_{false};
    std::atomic<float> frequency_{kDefaultFrequency};
    std::atomic<float> amplitude_{kDefaultAmplitude};
};

} // namespace arcline::audio
