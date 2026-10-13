#include "audio/devices/AudioDeviceService.h"
#include <algorithm>

namespace arcline::audio {

AudioDeviceService::AudioDeviceService() {
    deviceManager_.addChangeListener(this);
}

AudioDeviceService::~AudioDeviceService() {
    deviceManager_.removeChangeListener(this);
    closeDevice();
}

bool AudioDeviceService::initialize(int numInputs, int numOutputs, double preferredSampleRate, int preferredBufferSize) {
    deviceState_ = AudioEngineState::Initializing;

    // Build preferred setup
    juce::AudioDeviceManager::AudioDeviceSetup setup;
    setup.sampleRate = preferredSampleRate;
    setup.bufferSize = preferredBufferSize;

    juce::String error = deviceManager_.initialise(numInputs, numOutputs, nullptr, true, {}, &setup);
    if (error.isNotEmpty()) {
        lastErrorMessage_ = error.toStdString();
        deviceState_ = AudioEngineState::DeviceUnavailable;
        return false;
    }

    updateStateFromCurrentDevice();
    return isDeviceOpen();
}

void AudioDeviceService::closeDevice() {
    deviceState_ = AudioEngineState::ShuttingDown;
    deviceManager_.closeAudioDevice();
    deviceState_ = AudioEngineState::Uninitialized;
}

std::vector<std::string> AudioDeviceService::getAvailableDeviceTypes() {
    std::vector<std::string> types;
    for (const auto* type : deviceManager_.getAvailableDeviceTypes()) {
        if (type) {
            types.push_back(type->getTypeName().toStdString());
        }
    }
    return types;
}

std::string AudioDeviceService::getCurrentDeviceType() const {
    return deviceManager_.getCurrentAudioDeviceType().toStdString();
}

bool AudioDeviceService::setDeviceType(const std::string& typeName) {
    deviceState_ = AudioEngineState::Reconfiguring;
    deviceManager_.setCurrentAudioDeviceType(juce::String(typeName), true);
    updateStateFromCurrentDevice();
    return isDeviceOpen();
}

std::vector<DeviceDescriptor> AudioDeviceService::getOutputDeviceDescriptors() {
    std::vector<DeviceDescriptor> descriptors;
    auto* currentType = deviceManager_.getCurrentDeviceTypeObject();
    if (!currentType) return descriptors;

    currentType->scanForDevices();
    auto deviceNames = currentType->getDeviceNames(false); // false = output devices
    int defaultIndex = currentType->getDefaultDeviceIndex(false);

    for (int i = 0; i < deviceNames.size(); ++i) {
        DeviceDescriptor desc;
        desc.name = deviceNames[i].toStdString();
        desc.typeName = currentType->getTypeName().toStdString();
        desc.isDefault = (i == defaultIndex);
        desc.maxOutputChannels = 2; // Default baseline; updated if current

        // If it's the currently active device, populate actual capabilities
        auto* activeDev = deviceManager_.getCurrentAudioDevice();
        if (activeDev && activeDev->getName() == deviceNames[i]) {
            desc.maxOutputChannels = activeDev->getOutputChannelNames().size();
            for (double rate : activeDev->getAvailableSampleRates()) {
                desc.availableSampleRates.push_back(rate);
            }
            for (int buf : activeDev->getAvailableBufferSizes()) {
                desc.availableBufferSizes.push_back(buf);
            }
        } else {
            // Standard fallback capabilities for non-open devices
            desc.availableSampleRates = { 44100.0, 48000.0, 88200.0, 96000.0 };
            desc.availableBufferSizes = { 64, 128, 256, 512, 1024 };
        }

        descriptors.push_back(std::move(desc));
    }

    return descriptors;
}

std::vector<DeviceDescriptor> AudioDeviceService::getInputDeviceDescriptors() {
    std::vector<DeviceDescriptor> descriptors;
    auto* currentType = deviceManager_.getCurrentDeviceTypeObject();
    if (!currentType) return descriptors;

    currentType->scanForDevices();
    auto deviceNames = currentType->getDeviceNames(true); // true = input devices
    int defaultIndex = currentType->getDefaultDeviceIndex(true);

    for (int i = 0; i < deviceNames.size(); ++i) {
        DeviceDescriptor desc;
        desc.name = deviceNames[i].toStdString();
        desc.typeName = currentType->getTypeName().toStdString();
        desc.isDefault = (i == defaultIndex);
        desc.maxInputChannels = 2;

        descriptors.push_back(std::move(desc));
    }

    return descriptors;
}

bool AudioDeviceService::applyConfiguration(const std::string& outputDeviceName,
                                            double sampleRate,
                                            int bufferSize,
                                            std::string& outError) {
    deviceState_ = AudioEngineState::Reconfiguring;

    auto setup = deviceManager_.getAudioDeviceSetup();
    if (!outputDeviceName.empty()) {
        setup.outputDeviceName = juce::String(outputDeviceName);
    }
    if (sampleRate > 0.0) {
        setup.sampleRate = sampleRate;
    }
    if (bufferSize > 0) {
        setup.bufferSize = bufferSize;
    }

    juce::String err = deviceManager_.setAudioDeviceSetup(setup, true);
    if (err.isNotEmpty()) {
        outError = err.toStdString();
        lastErrorMessage_ = outError;
        deviceState_ = AudioEngineState::RecoverableError;
        return false;
    }

    updateStateFromCurrentDevice();
    return true;
}

AudioSettingsSnapshot AudioDeviceService::getSnapshot() const {
    AudioSettingsSnapshot snap;
    snap.deviceTypeName = getCurrentDeviceType();

    auto* dev = deviceManager_.getCurrentAudioDevice();
    if (dev) {
        snap.isDeviceOpen = dev->isOpen();
        snap.outputDeviceName = dev->getName().toStdString();
        snap.sampleRate = dev->getCurrentSampleRate();
        snap.bufferSize = dev->getCurrentBufferSizeSamples();
        snap.numOutputChannels = dev->getOutputChannelNames().size();
        snap.numInputChannels = dev->getInputChannelNames().size();
    } else {
        snap.isDeviceOpen = false;
        snap.lastErrorMessage = lastErrorMessage_;
    }

    return snap;
}

bool AudioDeviceService::isDeviceOpen() const noexcept {
    auto* dev = deviceManager_.getCurrentAudioDevice();
    return dev != nullptr && dev->isOpen();
}

void AudioDeviceService::updateStateFromCurrentDevice() {
    auto* dev = deviceManager_.getCurrentAudioDevice();
    if (dev && dev->isOpen()) {
        deviceState_ = AudioEngineState::DeviceReady;
        lastErrorMessage_.clear();
    } else {
        deviceState_ = AudioEngineState::DeviceUnavailable;
    }
}

void AudioDeviceService::changeListenerCallback(juce::ChangeBroadcaster* /*source*/) {
    updateStateFromCurrentDevice();
    for (const auto& cb : deviceChangeListeners_) {
        if (cb) cb();
    }
}

// future_audio::IAudioDeviceService adapter methods
std::vector<future_audio::AudioDeviceInfo> AudioDeviceService::getAvailableInputDevices() {
    std::vector<future_audio::AudioDeviceInfo> result;
    for (const auto& desc : getInputDeviceDescriptors()) {
        future_audio::AudioDeviceInfo info;
        info.deviceId = desc.name;
        info.name = desc.name;
        info.isDefaultInput = desc.isDefault;
        info.inputChannelCount = desc.maxInputChannels;
        info.supportedSampleRates = desc.availableSampleRates;
        info.supportedBufferSizes = desc.availableBufferSizes;
        result.push_back(std::move(info));
    }
    return result;
}

std::vector<future_audio::AudioDeviceInfo> AudioDeviceService::getAvailableOutputDevices() {
    std::vector<future_audio::AudioDeviceInfo> result;
    for (const auto& desc : getOutputDeviceDescriptors()) {
        future_audio::AudioDeviceInfo info;
        info.deviceId = desc.name;
        info.name = desc.name;
        info.isDefaultOutput = desc.isDefault;
        info.outputChannelCount = desc.maxOutputChannels;
        info.supportedSampleRates = desc.availableSampleRates;
        info.supportedBufferSizes = desc.availableBufferSizes;
        result.push_back(std::move(info));
    }
    return result;
}

bool AudioDeviceService::setAudioDevice(const std::string& /*inputDeviceId*/, 
                                        const std::string& outputDeviceId,
                                        double sampleRate, 
                                        int bufferSize) {
    std::string err;
    return applyConfiguration(outputDeviceId, sampleRate, bufferSize, err);
}

std::string AudioDeviceService::getCurrentInputDeviceId() const {
    auto* dev = deviceManager_.getCurrentAudioDevice();
    return dev ? dev->getName().toStdString() : "";
}

std::string AudioDeviceService::getCurrentOutputDeviceId() const {
    auto* dev = deviceManager_.getCurrentAudioDevice();
    return dev ? dev->getName().toStdString() : "";
}

double AudioDeviceService::getCurrentSampleRate() const noexcept {
    auto* dev = deviceManager_.getCurrentAudioDevice();
    return dev ? dev->getCurrentSampleRate() : 48000.0;
}

int AudioDeviceService::getCurrentBufferSize() const noexcept {
    auto* dev = deviceManager_.getCurrentAudioDevice();
    return dev ? dev->getCurrentBufferSizeSamples() : 256;
}

void AudioDeviceService::registerDeviceChangeListener(std::function<void()> callback) {
    if (callback) {
        deviceChangeListeners_.push_back(std::move(callback));
    }
}

} // namespace arcline::audio
