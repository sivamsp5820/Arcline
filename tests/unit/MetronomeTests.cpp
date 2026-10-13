#include "tests/TestAssert.h"
#include "audio/processors/MetronomeProcessor.h"
#include "audio/transport/AudioClock.h"
#include <vector>
#include <cmath>

namespace arcline::tests {

void runMetronomeTests() {
    using namespace arcline::audio;

    MetronomeProcessor metro;
    metro.prepareToPlay(48000.0, 256);
    metro.setGain(0.5f);

    AudioClock clock;
    clock.setSampleRate(48000.0);
    clock.setTempoBpm(120.0);
    clock.setTimeSignature(4, 4);

    // 1. Disabled Metronome Produces Absolute Silence
    {
        metro.setEnabled(false);
        std::vector<float> bufL(256, 0.0f);
        std::vector<float> bufR(256, 0.0f);
        float* channels[] = { bufL.data(), bufR.data() };

        metro.processBlockWithClock(channels, 2, 256, &clock, true);

        for (int i = 0; i < 256; ++i) {
            ARCLINE_TEST_ASSERT(bufL[i] == 0.0f);
            ARCLINE_TEST_ASSERT(bufR[i] == 0.0f);
        }
    }

    // 2. Sample-Accurate Downbeat Click Trigger (Beat 1 of measure)
    {
        metro.setEnabled(true);
        clock.locateToSample(0); // Exactly at Beat 1 boundary

        std::vector<float> bufL(256, 0.0f);
        std::vector<float> bufR(256, 0.0f);
        float* channels[] = { bufL.data(), bufR.data() };

        metro.processBlockWithClock(channels, 2, 256, &clock, true);

        // Click starts in this block, peak is generated
        float maxSample = 0.0f;
        for (int i = 0; i < 256; ++i) {
            ARCLINE_TEST_ASSERT(!std::isnan(bufL[i]));
            ARCLINE_TEST_ASSERT(!std::isinf(bufL[i]));
            maxSample = std::max(maxSample, std::abs(bufL[i]));
        }
        ARCLINE_TEST_ASSERT(maxSample > 0.05f);
    }

    // 3. Silence Between Beats
    {
        // Sample 12,000 is half a beat into measure (no beat boundary in [12000, 12256])
        clock.locateToSample(12000);

        std::vector<float> bufL(256, 0.0f);
        std::vector<float> bufR(256, 0.0f);
        float* channels[] = { bufL.data(), bufR.data() };

        metro.processBlockWithClock(channels, 2, 256, &clock, true);

        for (int i = 0; i < 256; ++i) {
            ARCLINE_TEST_ASSERT(bufL[i] == 0.0f);
            ARCLINE_TEST_ASSERT(bufR[i] == 0.0f);
        }
    }
}

} // namespace arcline::tests
