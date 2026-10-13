#include "audio/engine/AudioEngine.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace arcline::audio {

AudioEngine::AudioEngine(std::shared_ptr<AudioDeviceService> deviceService,
                         std::shared_ptr<TransportController> transportController)
    : deviceService_(std::move(deviceService)),
      transportController_(std::move(transportController)) {
}

AudioEngine::~AudioEngine() {
    shutdown();
}

bool AudioEngine::initialize(double sampleRate, int blockSize) {
    state_ = AudioEngineState::Initializing;

    currentSampleRate_ = (sampleRate > 0.0) ? sampleRate : 48000.0;
    currentBlockSize_ = (blockSize > 0) ? blockSize : 256;
    bufferBudgetUs_ = static_cast<float>((static_cast<double>(currentBlockSize_) / currentSampleRate_) * 1000000.0);

    testToneGenerator_.prepareToPlay(currentSampleRate_, currentBlockSize_);
    metronomeProcessor_.prepareToPlay(currentSampleRate_, currentBlockSize_);

    if (transportController_) {
        transportController_->prepareToPlay(currentSampleRate_, currentBlockSize_);
    }

    diagnostics_.reset();

    // Register callback with device manager if available
    if (deviceService_) {
        if (!deviceService_->isDeviceOpen()) {
            deviceService_->initialize(0, 2, currentSampleRate_, currentBlockSize_);
        }
        deviceService_->getJuceDeviceManager().addAudioCallback(this);
        state_ = deviceService_->isDeviceOpen() ? AudioEngineState::DeviceReady : AudioEngineState::DeviceUnavailable;
    } else {
        state_ = AudioEngineState::DeviceReady;
    }

    return true;
}

void AudioEngine::shutdown() {
    state_ = AudioEngineState::ShuttingDown;
    isRunning_.store(false, std::memory_order_release);

    if (deviceService_) {
        deviceService_->getJuceDeviceManager().removeAudioCallback(this);
    }

    testToneGenerator_.releaseResources();
    metronomeProcessor_.releaseResources();

    state_ = AudioEngineState::Uninitialized;
}

bool AudioEngine::start() {
    if (state_ == AudioEngineState::DeviceUnavailable) {
        return false;
    }
    isRunning_.store(true, std::memory_order_release);
    state_ = AudioEngineState::ProcessingRunning;
    return true;
}

void AudioEngine::stop() {
    isRunning_.store(false, std::memory_order_release);
    if (state_ == AudioEngineState::ProcessingRunning) {
        state_ = AudioEngineState::DeviceReady;
    }
}

bool AudioEngine::isRunning() const noexcept {
    return isRunning_.load(std::memory_order_relaxed);
}

double AudioEngine::getSampleRate() const noexcept {
    return currentSampleRate_;
}

int AudioEngine::getBlockSize() const noexcept {
    return currentBlockSize_;
}

double AudioEngine::getCpuUsagePercent() const noexcept {
    return static_cast<double>(diagnostics_.getCpuUsagePercent());
}

void AudioEngine::audioDeviceIOCallbackWithContext(const float* const* /*inputChannelData*/,
                                                   int /*numInputChannels*/,
                                                   float* const* outputChannelData,
                                                   int numOutputChannels,
                                                   int numSamples,
                                                   const juce::AudioIODeviceCallbackContext& /*context*/) {
    auto startTime = std::chrono::steady_clock::now();

    // Zero output buffers
    for (int ch = 0; ch < numOutputChannels; ++ch) {
        if (outputChannelData[ch]) {
            std::memset(outputChannelData[ch], 0, sizeof(float) * static_cast<size_t>(numSamples));
        }
    }

    // Advance transport clock if playing
    if (transportController_) {
        transportController_->advanceClockInCallback(numSamples);
    }

    // Render test tone
    testToneGenerator_.processBlock(outputChannelData, numOutputChannels, numSamples);

    // Render metronome clicks
    if (transportController_) {
        metronomeProcessor_.processBlockWithClock(outputChannelData, 
                                                 numOutputChannels, 
                                                 numSamples, 
                                                 &transportController_->getClock(), 
                                                 transportController_->isPlaying());
    }

    // Compute peak levels for UI telemetry
    float peakL = 0.0f;
    float peakR = 0.0f;
    bool hasNanOrInf = false;

    if (numOutputChannels > 0 && outputChannelData[0]) {
        for (int i = 0; i < numSamples; ++i) {
            float s = std::abs(outputChannelData[0][i]);
            if (std::isnan(s) || std::isinf(s)) {
                hasNanOrInf = true;
                outputChannelData[0][i] = 0.0f;
            } else if (s > peakL) {
                peakL = s;
            }
        }
    }

    if (numOutputChannels > 1 && outputChannelData[1]) {
        for (int i = 0; i < numSamples; ++i) {
            float s = std::abs(outputChannelData[1][i]);
            if (std::isnan(s) || std::isinf(s)) {
                hasNanOrInf = true;
                outputChannelData[1][i] = 0.0f;
            } else if (s > peakR) {
                peakR = s;
            }
        }
    } else {
        peakR = peakL;
    }

    masterPeakLeft_.store(peakL, std::memory_order_relaxed);
    masterPeakRight_.store(peakR, std::memory_order_relaxed);

    auto endTime = std::chrono::steady_clock::now();
    float durationUs = static_cast<float>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count()) / 1000.0f;

    diagnostics_.recordCallback(numSamples, durationUs, bufferBudgetUs_, hasNanOrInf);
}

void AudioEngine::processOfflineBlock(float* const* outputChannels, int numChannels, int numSamples) {
    audioDeviceIOCallbackWithContext(nullptr, 0, outputChannels, numChannels, numSamples, {});
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device) {
    if (device) {
        currentSampleRate_ = device->getCurrentSampleRate();
        currentBlockSize_ = device->getCurrentBufferSizeSamples();
        bufferBudgetUs_ = static_cast<float>((static_cast<double>(currentBlockSize_) / currentSampleRate_) * 1000000.0);

        testToneGenerator_.prepareToPlay(currentSampleRate_, currentBlockSize_);
        metronomeProcessor_.prepareToPlay(currentSampleRate_, currentBlockSize_);

        if (transportController_) {
            transportController_->prepareToPlay(currentSampleRate_, currentBlockSize_);
        }
    }
    state_ = AudioEngineState::ProcessingRunning;
    isRunning_.store(true, std::memory_order_release);
}

void AudioEngine::audioDeviceStopped() {
    isRunning_.store(false, std::memory_order_release);
    state_ = AudioEngineState::DeviceReady;
}

void AudioEngine::audioDeviceError(const juce::String& errorMessage) {
    diagnostics_.recordXrun();
    state_ = AudioEngineState::RecoverableError;
}

} // namespace arcline::audio
