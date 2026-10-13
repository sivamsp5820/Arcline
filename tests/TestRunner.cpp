#include <iostream>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

namespace sonora::tests {
    void runProjectTests();
    void runUndoHistoryTests();
    void runPreferencesTests();
    void runRecentProjectsTests();
    void runCommandRegistryTests();
    void runNavigationTests();
    void runLocalizationTests();
    void runLayoutVerificationTests();
    void runProjectLifecycleTests();
    void runProjectFormatDurabilityTests();
}

namespace arcline::tests {
    void runAudioEngineLifecycleTests();
    void runAudioClockTests();
    void runTestToneSignalTests();
    void runMetronomeTests();
    void runDeviceSettingsTests();
}

int main(int /*argc*/, char** /*argv*/) {
    std::cout << "TEST_RUNNER_START" << std::endl;
    juce::ScopedJuceInitialiser_GUI juceInitialiser;
    std::cout << "JUCE_INITIALISED" << std::endl;

    std::cout << "========================================\n";
    std::cout << "   ARCLINE Phase 2 Master Test Suite    \n";
    std::cout << "   \"Shape Your Sound.\"                  \n";
    std::cout << "========================================\n\n";

    try {
        std::cout << "[RUNNING] ProjectTests..." << std::endl;
        sonora::tests::runProjectTests();

        std::cout << "[RUNNING] UndoHistoryTests..." << std::endl;
        sonora::tests::runUndoHistoryTests();

        std::cout << "[RUNNING] PreferencesTests..." << std::endl;
        sonora::tests::runPreferencesTests();

        std::cout << "[RUNNING] RecentProjectsTests..." << std::endl;
        sonora::tests::runRecentProjectsTests();

        std::cout << "[RUNNING] CommandRegistryTests..." << std::endl;
        sonora::tests::runCommandRegistryTests();

        std::cout << "[RUNNING] NavigationTests..." << std::endl;
        sonora::tests::runNavigationTests();

        std::cout << "[RUNNING] LocalizationTests..." << std::endl;
        sonora::tests::runLocalizationTests();

        std::cout << "[RUNNING] LayoutVerificationTests..." << std::endl;
        sonora::tests::runLayoutVerificationTests();

        std::cout << "[RUNNING] ProjectFormatDurabilityTests..." << std::endl;
        sonora::tests::runProjectFormatDurabilityTests();

        std::cout << "[RUNNING] ProjectLifecycleTests..." << std::endl;
        sonora::tests::runProjectLifecycleTests();

        // Phase 2 Audio Engine Test Suites
        std::cout << "[RUNNING] AudioEngineLifecycleTests..." << std::endl;
        arcline::tests::runAudioEngineLifecycleTests();

        std::cout << "[RUNNING] AudioClockTests..." << std::endl;
        arcline::tests::runAudioClockTests();

        std::cout << "[RUNNING] TestToneSignalTests..." << std::endl;
        arcline::tests::runTestToneSignalTests();

        std::cout << "[RUNNING] MetronomeTests..." << std::endl;
        arcline::tests::runMetronomeTests();

        std::cout << "[RUNNING] DeviceSettingsTests..." << std::endl;
        arcline::tests::runDeviceSettingsTests();

        std::cout << "\n========================================\n";
        std::cout << "   SUCCESS: ALL 15 AUTOMATED SUITES PASSED! \n";
        std::cout << "========================================\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n[TEST FAILED] Exception caught: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "\n[TEST FAILED] Unknown exception caught!\n";
        return 1;
    }
}
