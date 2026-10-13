#include "tests/TestAssert.h"
#include "audio/transport/AudioClock.h"

namespace arcline::tests {

void runAudioClockTests() {
    using namespace arcline::audio;

    AudioClock clock;
    clock.setSampleRate(48000.0);
    clock.setTempoBpm(120.0);
    clock.setTimeSignature(4, 4);

    // 1. Initial Position Verification (Zero)
    {
        ARCLINE_TEST_ASSERT(clock.getCurrentSamplePosition() == 0);
        auto pos = clock.getMusicalPosition();
        ARCLINE_TEST_ASSERT(pos.bar == 1);
        ARCLINE_TEST_ASSERT(pos.beat == 1);
        ARCLINE_TEST_ASSERT(pos.division == 1);
        ARCLINE_TEST_ASSERT(pos.tick == 0);
        ARCLINE_TEST_ASSERT(pos.totalTicks == 0);
        ARCLINE_TEST_ASSERT(pos.totalSamples == 0);
        ARCLINE_TEST_ASSERT(pos.seconds == 0.0);
    }

    // 2. Sample to Musical Position Conversions
    {
        // At 48kHz, 120 BPM:
        // 1 beat = 48000 * 60 / 120 = 24000 samples
        ARCLINE_TEST_ASSERT(clock.getSamplesPerBeat() == 24000.0);
        ARCLINE_TEST_ASSERT(clock.getSamplesPerTick() == 25.0);

        // Advance by 1 beat (24000 samples)
        clock.advanceSamples(24000);
        auto pos1 = clock.getMusicalPosition();
        ARCLINE_TEST_ASSERT(pos1.bar == 1);
        ARCLINE_TEST_ASSERT(pos1.beat == 2);
        ARCLINE_TEST_ASSERT(pos1.division == 1);
        ARCLINE_TEST_ASSERT(pos1.tick == 0);
        ARCLINE_TEST_ASSERT(pos1.totalTicks == 960);
        ARCLINE_TEST_ASSERT(std::abs(pos1.seconds - 0.5) < 0.0001);

        // Advance by 3 more beats (total 96000 samples = Bar 2, Beat 1)
        clock.advanceSamples(72000);
        auto pos2 = clock.getMusicalPosition();
        ARCLINE_TEST_ASSERT(pos2.bar == 2);
        ARCLINE_TEST_ASSERT(pos2.beat == 1);
        ARCLINE_TEST_ASSERT(pos2.division == 1);
        ARCLINE_TEST_ASSERT(pos2.tick == 0);
        ARCLINE_TEST_ASSERT(pos2.totalTicks == 3840);
        ARCLINE_TEST_ASSERT(std::abs(pos2.seconds - 2.0) < 0.0001);
    }

    // 3. Locate Controls
    {
        clock.locateToSample(12000);
        ARCLINE_TEST_ASSERT(clock.getCurrentSamplePosition() == 12000);
        auto pos = clock.getMusicalPosition();
        ARCLINE_TEST_ASSERT(pos.bar == 1);
        ARCLINE_TEST_ASSERT(pos.beat == 1);
        ARCLINE_TEST_ASSERT(pos.totalTicks == 480);

        clock.locateToTick(960);
        ARCLINE_TEST_ASSERT(clock.getCurrentSamplePosition() == 24000);
    }

    // 4. Cycle Range Looping Math
    {
        clock.locateToSample(0);
        // Loop range: Tick 0 to Tick 3840 (1 bar of 4/4 = 96000 samples)
        clock.setLoopRange(0, 3840);
        clock.setLooping(true);

        clock.advanceSamples(50000);
        ARCLINE_TEST_ASSERT(clock.getCurrentSamplePosition() == 50000);

        // Advance 50,000 more samples (target 100,000 >= 96,000 loopEnd)
        // Wraps to loopStart (0) + 4000 = 4000 samples
        clock.advanceSamples(50000);
        ARCLINE_TEST_ASSERT(clock.getCurrentSamplePosition() == 4000);
    }
}

} // namespace arcline::tests
