#pragma once

#include "future_audio/IAudioDeviceService.h"
#include "audio/devices/AudioSettingsModel.h"
#include "audio/engine/AudioEngineState.h"
#include <juce_audio_devices/juce_audio_devices.h>
#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace arcline::audio {

/**
 * @brief Native audio hardware discovery, configuration, and device lifecycle manager.
 */
class AudioDeviceService : public future_audio::IAudioDeviceService,
                           private juce::ChangeListener {
public:
    AudioDeviceService();
    ~AudioDeviceService() override;

    /**
     * @brief Initializes audio device subsystem with preferred channel counts.
     */
    bool initialize(int numInputs = 0, int numOutputs = 2, double preferredSampleRate = 48000.0, int preferredBufferSize = 256);

    /**
     * @brief Close and release current audio hardware.
     */
    void closeDevice();

    // Query available device types (WASAPI, ASIO, DirectSound, CoreAudio, ALSA, etc.)
    std::vector<std::string> getAvailableDeviceTypes();
    std::string getCurrentDeviceType() const;
    bool setDeviceType(const std::string& typeName);

    // Query hardware devices
    std::vector<DeviceDescriptor> getOutputDeviceDescriptors();
    std::vector<DeviceDescriptor> getInputDeviceDescriptors();

    // Reconfiguration with capability validation
    bool applyConfiguration(const std::string& outputDeviceName,
                            double sampleRate,
                            int bufferSize,
                            std::string& outError);

    // Status queries
    AudioSettingsSnapshot getSnapshot() const;
    AudioEngineState getDeviceState() const noexcept { return deviceState_; }
    bool isDeviceOpen() const noexcept;

    // future_audio::IAudioDeviceService implementation
    std::vector<future_audio::AudioDeviceInfo> getAvailableInputDevices() override;
    std::vector<future_audio::AudioDeviceInfo> getAvailableOutputDevices() override;

    bool setAudioDevice(const std::string& inputDeviceId, 
                        const std::string& outputDeviceId,
                        double sampleRate, 
                        int bufferSize) override;

    std::string getCurrentInputDeviceId() const override;
    std::string getCurrentOutputDeviceId() const override;
    double getCurrentSampleRate() const noexcept override;
    int getCurrentBufferSize() const noexcept override;

    void registerDeviceChangeListener(std::function<void()> callback) override;

    juce::AudioDeviceManager& getJuceDeviceManager() noexcept { return deviceManager_; }

private:
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void updateStateFromCurrentDevice();

    juce::AudioDeviceManager deviceManager_;
    AudioEngineState deviceState_{AudioEngineState::Uninitialized};
    std::string lastErrorMessage_;

    std::vector<std::function<void()>> deviceChangeListeners_;
};

} // namespace arcline::audio
