#pragma once

#include <string>
#include <vector>
#include <functional>

namespace arcline::future_audio {

/**
 * @brief Device driver architectures supported across target operating systems.
 */
enum class AudioDriverType {
    Windows_WASAPI,
    Windows_ASIO,
    macOS_CoreAudio,
    Linux_ALSA,
    Linux_PipeWire,
    Linux_JACK
};

struct AudioDeviceInfo {
    std::string deviceId;
    std::string name;
    AudioDriverType driverType;
    int inputChannelCount = 0;
    int outputChannelCount = 0;
    std::vector<double> supportedSampleRates;
    std::vector<int> supportedBufferSizes;
    bool isDefaultInput = false;
    bool isDefaultOutput = false;
};

/**
 * @brief Extension point interface for native OS audio device management (Phase 2).
 * 
 * Boundaries and Contracts:
 * - Abstracts OS-specific hardware drivers (Windows WASAPI/ASIO, macOS CoreAudio, Linux ALSA/PipeWire).
 * - Manages device enumeration, hot-plug notifications, and stream opening/closing.
 * - Bridges the OS audio hardware buffer to the real-time processing graph.
 * - Hardware error states (dropouts, disconnection) are captured and reported safely to UI.
 */
class IAudioDeviceService {
public:
    virtual ~IAudioDeviceService() = default;

    virtual std::vector<AudioDeviceInfo> getAvailableInputDevices() = 0;
    virtual std::vector<AudioDeviceInfo> getAvailableOutputDevices() = 0;

    virtual bool setAudioDevice(const std::string& inputDeviceId, 
                                const std::string& outputDeviceId,
                                double sampleRate, 
                                int bufferSize) = 0;

    virtual std::string getCurrentInputDeviceId() const = 0;
    virtual std::string getCurrentOutputDeviceId() const = 0;
    virtual double getCurrentSampleRate() const noexcept = 0;
    virtual int getCurrentBufferSize() const noexcept = 0;

    virtual void registerDeviceChangeListener(std::function<void()> callback) = 0;
};

} // namespace arcline::future_audio

namespace sonora::future_audio {
    using AudioDriverType = arcline::future_audio::AudioDriverType;
    using AudioDeviceInfo = arcline::future_audio::AudioDeviceInfo;
    using IAudioDeviceService = arcline::future_audio::IAudioDeviceService;
}
