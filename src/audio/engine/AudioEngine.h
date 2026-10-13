#pragma once

#include "future_audio/IAudioEngine.h"
#include "audio/engine/AudioEngineState.h"
#include "audio/devices/AudioDeviceService.h"
#include "audio/transport/TransportController.h"
#include "audio/processors/TestToneGenerator.h"
#include "audio/processors/MetronomeProcessor.h"
#include "audio/diagnostics/AudioDiagnostics.h"
#include <juce_audio_devices/juce_audio_devices.h>
#include <memory>
#include <chrono>
#include <vector>

namespace arcline::audio {

/**
 * @brief Master real-time audio engine coordinating hardware I/O, transport, and DSP processors.
 */
class AudioEngine : public future_audio::IAudioEngine,
                    public juce::AudioIODeviceCallback {
public:
    AudioEngine(std::shared_ptr<AudioDeviceService> deviceService,
                std::shared_ptr<TransportController> transportController);
    ~AudioEngine() override;

    // future_audio::IAudioEngine implementation
    bool initialize(double sampleRate, int blockSize) override;
    void shutdown() override;
    bool isRunning() const noexcept override;

    double getSampleRate() const noexcept override;
    int getBlockSize() const noexcept override;
    double getCpuUsagePercent() const noexcept override;

    // Real-time processing control
    bool start();
    void stop();

    AudioEngineState getState() const noexcept { return state_; }

    // Subsystem accessors
    std::shared_ptr<AudioDeviceService> getDeviceService() const noexcept { return deviceService_; }
    std::shared_ptr<TransportController> getTransportController() const noexcept { return transportController_; }
    TestToneGenerator& getTestToneGenerator() noexcept { return testToneGenerator_; }
    MetronomeProcessor& getMetronomeProcessor() noexcept { return metronomeProcessor_; }
    const AudioDiagnostics& getDiagnostics() const noexcept { return diagnostics_; }

    // Metering telemetry (lock-free query for UI at 60 FPS)
    float getMasterPeakLeft() const noexcept { return masterPeakLeft_.load(std::memory_order_relaxed); }
    float getMasterPeakRight() const noexcept { return masterPeakRight_.load(std::memory_order_relaxed); }

    // juce::AudioIODeviceCallback implementation
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext& context) override;

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError(const juce::String& errorMessage) override;

    // Headless / Offline render block test method
    void processOfflineBlock(float* const* outputChannels, int numChannels, int numSamples);

private:
    std::shared_ptr<AudioDeviceService> deviceService_;
    std::shared_ptr<TransportController> transportController_;

    TestToneGenerator testToneGenerator_;
    MetronomeProcessor metronomeProcessor_;
    AudioDiagnostics diagnostics_;

    AudioEngineState state_{AudioEngineState::Uninitialized};
    std::atomic<bool> isRunning_{false};

    double currentSampleRate_{48000.0};
    int currentBlockSize_{256};
    float bufferBudgetUs_{5333.3f};

    std::atomic<float> masterPeakLeft_{0.0f};
    std::atomic<float> masterPeakRight_{0.0f};
};

} // namespace arcline::audio
