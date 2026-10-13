#include "tests/TestAssert.h"
#include "audio/processors/TestToneGenerator.h"
#include <vector>
#include <cmath>
#include <numeric>

namespace arcline::tests {

void runTestToneSignalTests() {
    using namespace arcline::audio;

    TestToneGenerator gen;
    gen.prepareToPlay(48000.0, 256);
    gen.setFrequency(440.0f);
    gen.setAmplitude(0.20f);

    // 1. Initial State (Disabled = Silence)
    {
        ARCLINE_TEST_ASSERT(!gen.isEnabled());
        std::vector<float> bufL(256, 0.0f);
        std::vector<float> bufR(256, 0.0f);
        float* channels[] = { bufL.data(), bufR.data() };

        gen.processBlock(channels, 2, 256);

        // Entire buffer must remain absolute 0.0f
        for (int i = 0; i < 256; ++i) {
            ARCLINE_TEST_ASSERT(bufL[i] == 0.0f);
            ARCLINE_TEST_ASSERT(bufR[i] == 0.0f);
        }
    }

    // 2. Enable with Smooth Ramping (Click-Free)
    {
        gen.setEnabled(true);
        std::vector<float> bufL(256, 0.0f);
        std::vector<float> bufR(256, 0.0f);
        float* channels[] = { bufL.data(), bufR.data() };

        gen.processBlock(channels, 2, 256);

        // First sample should start gentle (not full blast instant jump)
        ARCLINE_TEST_ASSERT(std::abs(bufL[0]) < 0.05f);

        // No NaN or Inf
        for (int i = 0; i < 256; ++i) {
            ARCLINE_TEST_ASSERT(!std::isnan(bufL[i]));
            ARCLINE_TEST_ASSERT(!std::isinf(bufL[i]));
            ARCLINE_TEST_ASSERT(std::abs(bufL[i]) <= 0.205f); // Peak within target amplitude
        }
    }

    // 3. Continuous Phase Across Block Boundaries
    {
        double phaseBefore = gen.getPhase();

        std::vector<float> bufL(128, 0.0f);
        std::vector<float> bufR(128, 0.0f);
        float* channels[] = { bufL.data(), bufR.data() };

        gen.processBlock(channels, 2, 128);

        double phaseAfter = gen.getPhase();
        // Phase must have advanced continuously without reset to 0
        ARCLINE_TEST_ASSERT(phaseAfter != phaseBefore);
        ARCLINE_TEST_ASSERT(phaseAfter >= 0.0 && phaseAfter < 2.0 * std::numbers::pi);
    }

    // 4. Smooth Ramp Down to Silence upon Disable
    {
        gen.setEnabled(false);

        std::vector<float> bufL(512, 0.0f);
        std::vector<float> bufR(512, 0.0f);
        float* channels[] = { bufL.data(), bufR.data() };

        gen.processBlock(channels, 2, 512);

        // At end of 512 samples, gain must have ramped down to 0.0
        ARCLINE_TEST_ASSERT(gen.getCurrentGain() == 0.0f);
        ARCLINE_TEST_ASSERT(bufL[511] == 0.0f);
    }
}

} // namespace arcline::tests
