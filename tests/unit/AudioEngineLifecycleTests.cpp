#include "tests/TestAssert.h"
#include "audio/engine/AudioEngineState.h"
#include "audio/engine/AudioEngine.h"
#include "audio/devices/AudioDeviceService.h"
#include "audio/transport/TransportController.h"

namespace arcline::tests {

void runAudioEngineLifecycleTests() {
    using namespace arcline::audio;

    // 1. AudioEngineState transitions
    {
        ARCLINE_TEST_ASSERT(isValidTransition(AudioEngineState::Uninitialized, AudioEngineState::Initializing));
        ARCLINE_TEST_ASSERT(isValidTransition(AudioEngineState::Initializing, AudioEngineState::DeviceReady));
        ARCLINE_TEST_ASSERT(isValidTransition(AudioEngineState::DeviceReady, AudioEngineState::ProcessingRunning));
        ARCLINE_TEST_ASSERT(isValidTransition(AudioEngineState::ProcessingRunning, AudioEngineState::DeviceReady));
        ARCLINE_TEST_ASSERT(isValidTransition(AudioEngineState::DeviceReady, AudioEngineState::Reconfiguring));
        ARCLINE_TEST_ASSERT(isValidTransition(AudioEngineState::Reconfiguring, AudioEngineState::DeviceReady));
        ARCLINE_TEST_ASSERT(isValidTransition(AudioEngineState::DeviceReady, AudioEngineState::ShuttingDown));
        ARCLINE_TEST_ASSERT(isValidTransition(AudioEngineState::ShuttingDown, AudioEngineState::Uninitialized));

        // Invalid transitions
        ARCLINE_TEST_ASSERT(!isValidTransition(AudioEngineState::Uninitialized, AudioEngineState::ProcessingRunning));
        ARCLINE_TEST_ASSERT(!isValidTransition(AudioEngineState::ShuttingDown, AudioEngineState::ProcessingRunning));
    }

    // 2. AudioEngine Lifecycle Initialization & Teardown
    {
        auto devService = std::make_shared<AudioDeviceService>();
        auto transport = std::make_shared<TransportController>();
        auto engine = std::make_shared<AudioEngine>(devService, transport);

        ARCLINE_TEST_ASSERT(engine->getState() == AudioEngineState::Uninitialized);
        ARCLINE_TEST_ASSERT(!engine->isRunning());

        bool initOk = engine->initialize(48000.0, 256);
        ARCLINE_TEST_ASSERT(initOk);
        ARCLINE_TEST_ASSERT(engine->getSampleRate() == 48000.0);
        ARCLINE_TEST_ASSERT(engine->getBlockSize() == 256);

        bool started = engine->start();
        ARCLINE_TEST_ASSERT(started);
        ARCLINE_TEST_ASSERT(engine->isRunning());
        ARCLINE_TEST_ASSERT(engine->getState() == AudioEngineState::ProcessingRunning);

        engine->stop();
        ARCLINE_TEST_ASSERT(!engine->isRunning());
        ARCLINE_TEST_ASSERT(engine->getState() == AudioEngineState::DeviceReady);

        engine->shutdown();
        ARCLINE_TEST_ASSERT(engine->getState() == AudioEngineState::Uninitialized);
    }

    // 3. Audio Diagnostics Telemetry
    {
        AudioDiagnostics diag;
        ARCLINE_TEST_ASSERT(diag.getXrunCount() == 0);
        ARCLINE_TEST_ASSERT(diag.getCpuUsagePercent() == 0.0f);

        diag.recordCallback(256, 120.0f, 5333.3f, false);
        auto snap = diag.getSnapshot();
        ARCLINE_TEST_ASSERT(snap.totalBuffersProcessed == 1);
        ARCLINE_TEST_ASSERT(snap.totalSamplesProcessed == 256);
        ARCLINE_TEST_ASSERT(snap.lastCallbackTimeUs == 120.0f);
        ARCLINE_TEST_ASSERT(snap.xrunCount == 0);

        // Record a buffer overrun (exceeding budget)
        diag.recordCallback(256, 6000.0f, 5333.3f, false);
        ARCLINE_TEST_ASSERT(diag.getXrunCount() == 1);

        diag.reset();
        ARCLINE_TEST_ASSERT(diag.getSnapshot().totalBuffersProcessed == 0);
        ARCLINE_TEST_ASSERT(diag.getXrunCount() == 0);
    }
}

} // namespace arcline::tests
