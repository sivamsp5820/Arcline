#include "domain/Project.h"
#include "infrastructure/FileSystem.h"
#include "infrastructure/ProjectRepository.h"
#include "infrastructure/PreferencesRepository.h"
#include "audio/engine/AudioEngine.h"
#include "audio/transport/TransportController.h"
#include "tests/TestAssert.h"
#include <iostream>
#include <filesystem>

namespace sonora::tests {

void runProjectLifecycleTests() {
    std::cout << "[TEST] Running ProjectLifecycleTests..." << std::endl;

    std::string tempDir = infrastructure::FileSystem::combinePath(
        infrastructure::FileSystem::getAppDataDirectory(), "TestWorkspace");
    std::cout << "  -> Temp dir: " << tempDir << std::endl;
    bool dirCreated = infrastructure::FileSystem::createDirectories(tempDir);
    SONORA_TEST_ASSERT(dirCreated);

    std::string projectPath = infrastructure::FileSystem::combinePath(tempDir, "IntegrationTest.sonora");
    std::cout << "  -> Project path: " << projectPath << std::endl;

    // 1. Create with Multitrack Arrangement and Save
    std::cout << "  -> Step 1: Create project and initialize arrangement" << std::endl;
    domain::Project original("Integration Test Song", projectPath);
    original.setTempoBpm(128.0);
    original.setTimeSignature(4, 4);
    original.setKeySignature("D Min");
    original.initializeDefaultArrangement();

    SONORA_TEST_ASSERT(original.getTracks().size() == 4);
    std::cout << "  -> Step 1b: Saving to file..." << std::endl;

    std::string err;
    bool saveOk = infrastructure::ProjectRepository::saveToFile(original, projectPath, err);
    if (!saveOk) {
        std::cerr << "  -> SAVE FAILED! Error: " << err << std::endl;
    }
    SONORA_TEST_ASSERT(saveOk);
    SONORA_TEST_ASSERT(infrastructure::FileSystem::fileExists(projectPath));
    std::cout << "  -> Step 1 completed successfully." << std::endl;

    // 2. Load and verify deep Schema v2 restoration
    std::cout << "  -> Step 2: Loading from file..." << std::endl;
    auto loaded = infrastructure::ProjectRepository::loadFromFile(projectPath, err);
    std::cout << "  -> Step 2b: Verifying loaded project..." << std::endl;
    if (!loaded.has_value()) {
        std::cerr << "  -> LOAD FAILED! Error: " << err << std::endl;
    }
    SONORA_TEST_ASSERT(loaded.has_value());
    SONORA_TEST_ASSERT(loaded->getName() == "Integration Test Song");
    SONORA_TEST_ASSERT(loaded->getTempoBpm() == 128.0);
    SONORA_TEST_ASSERT(loaded->getTimeSigNumerator() == 4);
    SONORA_TEST_ASSERT(loaded->getTimeSigDenominator() == 4);
    SONORA_TEST_ASSERT(loaded->getKeySignature() == "D Min");
    SONORA_TEST_ASSERT(loaded->getTracks().size() == 4);

    // Verify audio track and waveform restoration
    auto voxTrack = loaded->getTracks()[0];
    SONORA_TEST_ASSERT(voxTrack->getName() == "Audio 1 (Lead Vocal)");
    SONORA_TEST_ASSERT(voxTrack->getType() == domain::TrackType::Audio);
    SONORA_TEST_ASSERT(voxTrack->getRegions().size() >= 2);
    SONORA_TEST_ASSERT(!voxTrack->getRegions()[0]->getWaveformPeaks().empty());

    // Verify MIDI track and notes restoration
    auto pianoTrack = loaded->getTracks()[1];
    SONORA_TEST_ASSERT(pianoTrack->getType() == domain::TrackType::SoftwareInstrument);
    SONORA_TEST_ASSERT(!pianoTrack->getRegions().empty());
    SONORA_TEST_ASSERT(!pianoTrack->getRegions()[0]->getNotes().empty());
    std::cout << "  -> Step 2 completed successfully." << std::endl;

    // 3. Reject corrupted JSON
    std::cout << "  -> Step 3: Rejecting corrupted JSON..." << std::endl;
    std::string corruptPath = infrastructure::FileSystem::combinePath(tempDir, "Corrupted.sonora");
    bool wroteCorrupt = infrastructure::FileSystem::writeTextAtomic(corruptPath, "{ \"format\": \"INVALID\", malformed JSON }}}");
    SONORA_TEST_ASSERT(wroteCorrupt);

    auto corruptLoaded = infrastructure::ProjectRepository::loadFromFile(corruptPath, err);
    SONORA_TEST_ASSERT(!corruptLoaded.has_value());
    SONORA_TEST_ASSERT(!err.empty());
    std::cout << "  -> Step 3 completed successfully." << std::endl;

    // 4. Preferences Repository lifecycle
    std::cout << "  -> Step 4: Preferences repository lifecycle..." << std::endl;
    infrastructure::PreferencesRepository prefsRepo(tempDir);
    domain::Preferences prefs;
    prefs.setConfirmOnExit(false);
    prefs.setUiScaleFactor(1.25);
    bool savePrefsOk = prefsRepo.savePreferences(prefs);
    SONORA_TEST_ASSERT(savePrefsOk);

    auto reloadedPrefs = prefsRepo.loadPreferences();
    SONORA_TEST_ASSERT(reloadedPrefs.getConfirmOnExit() == false);
    SONORA_TEST_ASSERT(reloadedPrefs.getUiScaleFactor() == 1.25);
    std::cout << "  -> Step 4 completed successfully." << std::endl;

    // 5. Phase 1 & Phase 2 Integrated Audio Playback & Transport Clock Synchronization
    std::cout << "  -> Step 5: Integrated Audio Transport & Playback Session..." << std::endl;
    auto transport = std::make_shared<arcline::audio::TransportController>();
    auto engine = std::make_shared<arcline::audio::AudioEngine>(nullptr, transport);

    // Prepare engine offline at 48 kHz / 256 samples
    bool engineInit = engine->initialize(48000.0, 256);
    SONORA_TEST_ASSERT(engineInit);
    SONORA_TEST_ASSERT(engine->start());

    // Project loaded has tempo 128 BPM, 4/4
    transport->setTempoBpm(loaded->getTempoBpm());
    transport->setTimeSignature(loaded->getTimeSigNumerator(), loaded->getTimeSigDenominator());
    transport->locateToTick(0);

    // Start playback
    transport->play();
    SONORA_TEST_ASSERT(transport->isPlaying());

    // Process 4 blocks of 256 samples (= 1024 samples)
    std::vector<float> bufL(256, 0.0f);
    std::vector<float> bufR(256, 0.0f);
    float* channels[] = { bufL.data(), bufR.data() };
    for (int b = 0; b < 4; ++b) {
        engine->processOfflineBlock(channels, 2, 256);
    }

    // Verify clock advancement
    SONORA_TEST_ASSERT(transport->getCurrentSamplePosition() == 1024);
    SONORA_TEST_ASSERT(transport->getCurrentTickPosition() > 0);

    // Stop playback
    transport->stop();
    SONORA_TEST_ASSERT(!transport->isPlaying());
    SONORA_TEST_ASSERT(transport->getCurrentSamplePosition() == 1024);

    // Return to start
    transport->returnToStart();
    SONORA_TEST_ASSERT(transport->getCurrentSamplePosition() == 0);
    SONORA_TEST_ASSERT(transport->getCurrentTickPosition() == 0);

    engine->shutdown();
    std::cout << "  -> Step 5 completed successfully." << std::endl;

    // Clean up temporary test files
    std::error_code ec;
    std::filesystem::remove_all(std::filesystem::u8path(tempDir), ec);

    std::cout << "  ✓ All ProjectLifecycleTests passed." << std::endl;
}

} // namespace sonora::tests
