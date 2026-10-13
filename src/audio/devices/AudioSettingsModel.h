#pragma once

#include <string>
#include <vector>

namespace arcline::audio {

/**
 * @brief Representation of an audio hardware device and its actual capabilities.
 */
struct DeviceDescriptor {
    std::string name;
    std::string typeName;
    bool isDefault = false;
    int maxInputChannels = 0;
    int maxOutputChannels = 0;
    std::vector<double> availableSampleRates;
    std::vector<int> availableBufferSizes;
};

/**
 * @brief Current active audio hardware configuration snapshot.
 */
struct AudioSettingsSnapshot {
    std::string deviceTypeName;
    std::string outputDeviceName;
    std::string inputDeviceName;
    double sampleRate = 48000.0;
    int bufferSize = 256;
    int numInputChannels = 0;
    int numOutputChannels = 2;
    bool isDeviceOpen = false;
    std::string lastErrorMessage;
};

} // namespace arcline::audio
