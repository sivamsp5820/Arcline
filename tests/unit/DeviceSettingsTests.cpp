#include "tests/TestAssert.h"
#include "audio/devices/AudioDeviceService.h"
#include "audio/devices/AudioSettingsModel.h"

namespace arcline::tests {

void runDeviceSettingsTests() {
    using namespace arcline::audio;

    // 1. Snapshot Default State
    {
        AudioSettingsSnapshot snap;
        ARCLINE_TEST_ASSERT(snap.sampleRate == 48000.0);
        ARCLINE_TEST_ASSERT(snap.bufferSize == 256);
        ARCLINE_TEST_ASSERT(!snap.isDeviceOpen);
    }

    // 2. AudioDeviceService Initialisation & Enumeration
    {
        AudioDeviceService service;
        auto types = service.getAvailableDeviceTypes();
        // Types should be discoverable on platform
        ARCLINE_TEST_ASSERT(!types.empty());

        auto outputs = service.getOutputDeviceDescriptors();
        // Devices list is safe to query (may be empty on headless runner)
        for (const auto& dev : outputs) {
            ARCLINE_TEST_ASSERT(!dev.name.empty());
            ARCLINE_TEST_ASSERT(!dev.availableSampleRates.empty());
            ARCLINE_TEST_ASSERT(!dev.availableBufferSizes.empty());
        }

        // Initialize service
        bool ok = service.initialize(0, 2, 48000.0, 256);
        if (ok) {
            ARCLINE_TEST_ASSERT(service.isDeviceOpen());
            auto snap = service.getSnapshot();
            ARCLINE_TEST_ASSERT(snap.isDeviceOpen);
            ARCLINE_TEST_ASSERT(snap.sampleRate > 0.0);
            ARCLINE_TEST_ASSERT(snap.bufferSize > 0);
        } else {
            // Headless environment with no soundcard: must report DeviceUnavailable safely
            ARCLINE_TEST_ASSERT(service.getDeviceState() == AudioEngineState::DeviceUnavailable);
        }

        service.closeDevice();
        ARCLINE_TEST_ASSERT(service.getDeviceState() == AudioEngineState::Uninitialized);
    }
}

} // namespace arcline::tests
