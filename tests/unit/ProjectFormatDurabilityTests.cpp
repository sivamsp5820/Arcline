#include "domain/Project.h"
#include "infrastructure/ProjectRepository.h"
#include "infrastructure/FileSystem.h"
#include "tests/TestAssert.h"
#include <iostream>

namespace sonora::tests {

void runProjectFormatDurabilityTests() {
    std::cout << "[TEST] Running ProjectFormatDurabilityTests..." << std::endl;

    std::string err;

    // 1. Empty Path and Nonexistent File Load Validation
    {
        auto emptyResult = infrastructure::ProjectRepository::loadFromFile("", err);
        ARCLINE_TEST_ASSERT(!emptyResult.has_value());
        ARCLINE_TEST_ASSERT(!err.empty());
        ARCLINE_TEST_ASSERT(err.find("empty") != std::string::npos);

        auto nonExistentResult = infrastructure::ProjectRepository::loadFromFile("Z:/nonexistent_dir/fake_project.arcline", err);
        ARCLINE_TEST_ASSERT(!nonExistentResult.has_value());
        ARCLINE_TEST_ASSERT(!err.empty());
        ARCLINE_TEST_ASSERT(err.find("does not exist") != std::string::npos);
    }

    // 2. Malformed JSON Syntax Rejection
    {
        std::string malformedJson = "{ \"format\": \"ARCLINE_PROJECT\", \"broken\": [1, 2, ";
        auto res = infrastructure::ProjectRepository::deserializeFromJson(malformedJson, err);
        ARCLINE_TEST_ASSERT(!res.has_value());
        ARCLINE_TEST_ASSERT(!err.empty());
        ARCLINE_TEST_ASSERT(err.find("Malformed JSON") != std::string::npos);
    }

    // 3. Missing Format Identifier Rejection
    {
        std::string missingFormat = "{\"schemaVersion\": 2, \"name\": \"Test Song\"}";
        auto res = infrastructure::ProjectRepository::deserializeFromJson(missingFormat, err);
        ARCLINE_TEST_ASSERT(!res.has_value());
        ARCLINE_TEST_ASSERT(!err.empty());
        ARCLINE_TEST_ASSERT(err.find("missing format") != std::string::npos);
    }

    // 4. Unsupported Format Identifier Rejection
    {
        std::string unsupportedFormat = "{\"format\": \"OTHER_DAW_FORMAT\", \"schemaVersion\": 2, \"name\": \"Song\"}";
        auto res = infrastructure::ProjectRepository::deserializeFromJson(unsupportedFormat, err);
        ARCLINE_TEST_ASSERT(!res.has_value());
        ARCLINE_TEST_ASSERT(!err.empty());
        ARCLINE_TEST_ASSERT(err.find("unsupported format") != std::string::npos);
    }

    // 5. Missing or Non-Positive Schema Version Rejection
    {
        std::string missingSchema = "{\"format\": \"ARCLINE_PROJECT\", \"name\": \"Song\"}";
        auto res = infrastructure::ProjectRepository::deserializeFromJson(missingSchema, err);
        ARCLINE_TEST_ASSERT(!res.has_value());
        ARCLINE_TEST_ASSERT(!err.empty());
        ARCLINE_TEST_ASSERT(err.find("missing schemaVersion") != std::string::npos);

        std::string zeroSchema = "{\"format\": \"ARCLINE_PROJECT\", \"schemaVersion\": 0, \"name\": \"Song\"}";
        res = infrastructure::ProjectRepository::deserializeFromJson(zeroSchema, err);
        ARCLINE_TEST_ASSERT(!res.has_value());
        ARCLINE_TEST_ASSERT(!err.empty());
        ARCLINE_TEST_ASSERT(err.find("Invalid project schema") != std::string::npos);
    }

    // 6. Unsupported Future Schema Version Rejection (Forward Migration Protection)
    {
        std::string futureSchema = "{\"format\": \"ARCLINE_PROJECT\", \"schemaVersion\": 999, \"name\": \"Future DAW Song\"}";
        auto res = infrastructure::ProjectRepository::deserializeFromJson(futureSchema, err);
        ARCLINE_TEST_ASSERT(!res.has_value());
        ARCLINE_TEST_ASSERT(!err.empty());
        ARCLINE_TEST_ASSERT(err.find("Unsupported project schema version (999)") != std::string::npos);
        ARCLINE_TEST_ASSERT(err.find("Please upgrade ARCLINE") != std::string::npos);
    }

    // 7. Missing or Whitespace-only Project Name Rejection
    {
        std::string emptyName = "{\"format\": \"ARCLINE_PROJECT\", \"schemaVersion\": 2, \"name\": \"   \"}";
        auto res = infrastructure::ProjectRepository::deserializeFromJson(emptyName, err);
        ARCLINE_TEST_ASSERT(!res.has_value());
        ARCLINE_TEST_ASSERT(!err.empty());
        ARCLINE_TEST_ASSERT(err.find("missing or empty project name") != std::string::npos);
    }

    // 8. Invalid Tempo Boundaries Rejection
    {
        std::string lowTempo = "{\"format\": \"ARCLINE_PROJECT\", \"schemaVersion\": 2, \"name\": \"Song\", \"tempoBpm\": 10.0}";
        auto res = infrastructure::ProjectRepository::deserializeFromJson(lowTempo, err);
        ARCLINE_TEST_ASSERT(!res.has_value());
        ARCLINE_TEST_ASSERT(!err.empty());
        ARCLINE_TEST_ASSERT(err.find("tempo out of valid range") != std::string::npos);

        std::string highTempo = "{\"format\": \"ARCLINE_PROJECT\", \"schemaVersion\": 2, \"name\": \"Song\", \"tempoBpm\": 1200.0}";
        res = infrastructure::ProjectRepository::deserializeFromJson(highTempo, err);
        ARCLINE_TEST_ASSERT(!res.has_value());
        ARCLINE_TEST_ASSERT(!err.empty());
        ARCLINE_TEST_ASSERT(err.find("tempo out of valid range") != std::string::npos);
    }

    // 9. Invalid Time Signature Rejection
    {
        std::string invalidSig = "{\"format\": \"ARCLINE_PROJECT\", \"schemaVersion\": 2, \"name\": \"Song\", \"timeSigNumerator\": 0, \"timeSigDenominator\": 4}";
        auto res = infrastructure::ProjectRepository::deserializeFromJson(invalidSig, err);
        ARCLINE_TEST_ASSERT(!res.has_value());
        ARCLINE_TEST_ASSERT(!err.empty());
        ARCLINE_TEST_ASSERT(err.find("invalid time signature") != std::string::npos);
    }

    // 10. Invalid Sample Rate Rejection
    {
        std::string badSampleRate = "{\"format\": \"ARCLINE_PROJECT\", \"schemaVersion\": 2, \"name\": \"Song\", \"sampleRate\": 4000.0}";
        auto res = infrastructure::ProjectRepository::deserializeFromJson(badSampleRate, err);
        ARCLINE_TEST_ASSERT(!res.has_value());
        ARCLINE_TEST_ASSERT(!err.empty());
        ARCLINE_TEST_ASSERT(err.find("Invalid sample rate") != std::string::npos);
    }

    // 11. Malformed Tracks List Rejection
    {
        std::string nonArrayTracks = "{\"format\": \"ARCLINE_PROJECT\", \"schemaVersion\": 2, \"name\": \"Song\", \"tracks\": \"not-an-array\"}";
        auto res = infrastructure::ProjectRepository::deserializeFromJson(nonArrayTracks, err);
        ARCLINE_TEST_ASSERT(!res.has_value());
        ARCLINE_TEST_ASSERT(!err.empty());
        ARCLINE_TEST_ASSERT(err.find("must be a JSON array") != std::string::npos);
    }

    // 12. Malformed Region Bounds Rejection
    {
        std::string badRegion = "{\"format\": \"ARCLINE_PROJECT\", \"schemaVersion\": 2, \"name\": \"Song\", \"tracks\": ["
                                "{\"id\": \"t1\", \"name\": \"Track 1\", \"type\": 0, \"volume\": 1.0, \"pan\": 0.0, "
                                "\"regions\": [{\"id\": \"r1\", \"name\": \"R1\", \"type\": 0, \"startTick\": -100, \"lengthTicks\": 100}]}"
                                "]}";
        auto res = infrastructure::ProjectRepository::deserializeFromJson(badRegion, err);
        ARCLINE_TEST_ASSERT(!res.has_value());
        ARCLINE_TEST_ASSERT(!err.empty());
        ARCLINE_TEST_ASSERT(err.find("Invalid region bounds") != std::string::npos);
    }

    // 13. Transparent Backward Compatibility: Legacy SONORA_PROJECT Loading
    {
        std::string legacyJson = "{\"format\": \"SONORA_PROJECT\", \"schemaVersion\": 1, \"name\": \"Legacy Hit\", \"tempoBpm\": 124.0, "
                                 "\"timeSigNumerator\": 4, \"timeSigDenominator\": 4, \"keySignature\": \"C Maj\"}";
        auto res = infrastructure::ProjectRepository::deserializeFromJson(legacyJson, err);
        ARCLINE_TEST_ASSERT(res.has_value());
        ARCLINE_TEST_ASSERT(res->getName() == "Legacy Hit");
        ARCLINE_TEST_ASSERT(res->getTempoBpm() == 124.0);
        ARCLINE_TEST_ASSERT(res->getKeySignature() == "C Maj");
    }

    // 14. Data Preservation Roundtrip (Waveform Peaks and MIDI Notes)
    {
        domain::Project proj("Deep Durability Test", "test_path.arcline");
        proj.setTempoBpm(132.0);
        proj.setTimeSignature(6, 8);
        proj.setKeySignature("Ab Maj");

        // Audio Track with 200 waveform peak points
        auto audioTrack = proj.addTrack(domain::TrackType::Audio, "Master Vocal");
        audioTrack->setVolume(0.85f);
        audioTrack->setPan(-0.25f);
        auto audioRegion = std::make_shared<domain::Region>("a_reg_01", audioTrack->getId(), "Lead Vox Take",
                                                            domain::RegionType::Audio, 0, 960 * 8);
        audioRegion->generateDemoWaveform(200, 4.0f);
        ARCLINE_TEST_ASSERT(audioRegion->getWaveformPeaks().size() == 200);
        audioTrack->addRegion(audioRegion);

        // MIDI Track with 4 chords (16 notes)
        auto midiTrack = proj.addTrack(domain::TrackType::SoftwareInstrument, "Analog Poly Synth");
        midiTrack->setVolume(0.70f);
        midiTrack->setPan(0.30f);
        auto midiRegion = std::make_shared<domain::Region>("m_reg_01", midiTrack->getId(), "Synth Progression",
                                                           domain::RegionType::Midi, 960 * 4, 960 * 8);
        for (int i = 0; i < 4; ++i) {
            int64_t tick = 960 * 4 + i * 960;
            midiRegion->addNote({60 + i * 2, tick, 960, 100});
            midiRegion->addNote({64 + i * 2, tick, 960, 90});
            midiRegion->addNote({67 + i * 2, tick, 960, 95});
            midiRegion->addNote({71 + i * 2, tick, 960, 85});
        }
        ARCLINE_TEST_ASSERT(midiRegion->getNotes().size() == 16);
        midiTrack->addRegion(midiRegion);

        // Serialize and Deserialize
        std::string serialized = infrastructure::ProjectRepository::serializeToJson(proj);
        ARCLINE_TEST_ASSERT(!serialized.empty());

        auto loadedProj = infrastructure::ProjectRepository::deserializeFromJson(serialized, err);
        ARCLINE_TEST_ASSERT(loadedProj.has_value());
        ARCLINE_TEST_ASSERT(loadedProj->getName() == "Deep Durability Test");
        ARCLINE_TEST_ASSERT(loadedProj->getTempoBpm() == 132.0);
        ARCLINE_TEST_ASSERT(loadedProj->getTimeSigNumerator() == 6);
        ARCLINE_TEST_ASSERT(loadedProj->getTimeSigDenominator() == 8);
        ARCLINE_TEST_ASSERT(loadedProj->getKeySignature() == "Ab Maj");
        ARCLINE_TEST_ASSERT(loadedProj->getTracks().size() == 2);

        // Verify Audio Track data integrity
        auto loadedAudioTrack = loadedProj->getTracks()[0];
        ARCLINE_TEST_ASSERT(loadedAudioTrack->getName() == "Master Vocal");
        ARCLINE_TEST_ASSERT(std::abs(loadedAudioTrack->getVolume() - 0.85f) < 0.001f);
        ARCLINE_TEST_ASSERT(std::abs(loadedAudioTrack->getPan() - (-0.25f)) < 0.001f);
        ARCLINE_TEST_ASSERT(loadedAudioTrack->getRegions().size() == 1);
        auto loadedAudioReg = loadedAudioTrack->getRegions()[0];
        ARCLINE_TEST_ASSERT(loadedAudioReg->getWaveformPeaks().size() == 200);

        // Verify MIDI Track data integrity
        auto loadedMidiTrack = loadedProj->getTracks()[1];
        ARCLINE_TEST_ASSERT(loadedMidiTrack->getName() == "Analog Poly Synth");
        ARCLINE_TEST_ASSERT(loadedMidiTrack->getRegions().size() == 1);
        auto loadedMidiReg = loadedMidiTrack->getRegions()[0];
        ARCLINE_TEST_ASSERT(loadedMidiReg->getNotes().size() == 16);
        ARCLINE_TEST_ASSERT(loadedMidiReg->getNotes()[0].pitch == 60);
        ARCLINE_TEST_ASSERT(loadedMidiReg->getNotes()[0].velocity == 100);
        ARCLINE_TEST_ASSERT(loadedMidiReg->getNotes()[15].pitch == 77);
    }

    // 15. Safe Error Reporting on Unwritable Destination Path
    {
        domain::Project proj("Save Failure Test", "");
        bool saveOk = infrastructure::ProjectRepository::saveToFile(proj, "", err);
        ARCLINE_TEST_ASSERT(!saveOk);
        ARCLINE_TEST_ASSERT(!err.empty());
        ARCLINE_TEST_ASSERT(err.find("empty") != std::string::npos);

        // Attempt write to impossible path
        saveOk = infrastructure::ProjectRepository::saveToFile(proj, "Z:/__nonexistent_root_dir_9999/test.arcline", err);
        ARCLINE_TEST_ASSERT(!saveOk);
        ARCLINE_TEST_ASSERT(!err.empty());
        ARCLINE_TEST_ASSERT(err.find("Failed to write") != std::string::npos);
    }

    // 16. POSIX Path Portability Conversion
    {
        ARCLINE_TEST_ASSERT(infrastructure::FileSystem::toPosixPath("C:\\Projects\\Arcline\\vocal.wav") == "C:/Projects/Arcline/vocal.wav");
        ARCLINE_TEST_ASSERT(infrastructure::FileSystem::toPosixPath("/usr/local/share/sound.wav") == "/usr/local/share/sound.wav");
        ARCLINE_TEST_ASSERT(infrastructure::FileSystem::toPosixPath("") == "");
    }

    // 17. Path Relativity Calculation
    {
        std::string baseDir = "E:/Projects/MySong";
        std::string targetAudio = "E:/Projects/MySong/Audio/vocal.wav";
        std::string relPath = infrastructure::FileSystem::makeRelative(baseDir, targetAudio);
        ARCLINE_TEST_ASSERT(relPath == "Audio/vocal.wav");

        // Already relative path
        ARCLINE_TEST_ASSERT(infrastructure::FileSystem::makeRelative(baseDir, "Audio/snare.wav") == "Audio/snare.wav");
    }

    // 18. Path Relative Resolution
    {
        std::string baseDir = "E:/Projects/MySong";
        std::string resolved = infrastructure::FileSystem::resolveRelative(baseDir, "Audio/vocal.wav");
        ARCLINE_TEST_ASSERT(resolved == "E:/Projects/MySong/Audio/vocal.wav");

        // Empty relative path
        ARCLINE_TEST_ASSERT(infrastructure::FileSystem::resolveRelative(baseDir, "") == "");
    }

    // 19. Cross-Platform Project Serialization Portability
    {
        std::string projectDir = "build/portable_project_test";
        infrastructure::FileSystem::createDirectories(projectDir);
        std::string projectFilePath = infrastructure::FileSystem::combinePath(projectDir, "PortableSong.arcline");

        domain::Project proj("Portable Song", projectFilePath);
        auto track = proj.addTrack(domain::TrackType::Audio, "Lead Synth");
        auto reg = std::make_shared<domain::Region>("reg_take1", track->getId(), "Vocal Stem", domain::RegionType::Audio, 0, 1920);
        
        // Use an audio path inside the project directory
        std::string audioPath = infrastructure::FileSystem::combinePath(projectDir, "Audio/take1.wav");
        reg->setAudioFilePath(audioPath);
        track->addRegion(reg);

        // Save to file
        bool saveOk = infrastructure::ProjectRepository::saveToFile(proj, projectFilePath, err);
        ARCLINE_TEST_ASSERT(saveOk);

        // Read raw file content to verify relative POSIX path in JSON
        bool readOk = false;
        std::string rawJson = infrastructure::FileSystem::readTextFile(projectFilePath, readOk);
        ARCLINE_TEST_ASSERT(readOk);
        ARCLINE_TEST_ASSERT(rawJson.find("\"audioFilePath\": \"Audio/take1.wav\"") != std::string::npos);

        // Load project from file and verify resolution back to full path
        auto loadedOpt = infrastructure::ProjectRepository::loadFromFile(projectFilePath, err);
        ARCLINE_TEST_ASSERT(loadedOpt.has_value());
        auto loadedTrack = loadedOpt->getTracks()[0];
        auto loadedReg = loadedTrack->getRegions()[0];
        ARCLINE_TEST_ASSERT(loadedReg->getAudioFilePath() == infrastructure::FileSystem::toPosixPath(audioPath));

        // Cleanup
        infrastructure::FileSystem::deleteFile(projectFilePath);
    }

    // 20. Atomic Write Durability and Crash Recovery
    {
        std::string testDir = "build/atomic_recovery_test";
        infrastructure::FileSystem::createDirectories(testDir);
        std::string testFile = infrastructure::FileSystem::combinePath(testDir, "metadata.arcline");

        std::string originalContent = "{\"valid\": true, \"state\": \"pristine\"}";
        ARCLINE_TEST_ASSERT(infrastructure::FileSystem::writeTextAtomic(testFile, originalContent));

        // Verify original file exists
        bool success = false;
        std::string readContent = infrastructure::FileSystem::readTextFile(testFile, success);
        ARCLINE_TEST_ASSERT(success);
        ARCLINE_TEST_ASSERT(readContent == originalContent);

        // Attempting write with empty target must fail without touching original
        ARCLINE_TEST_ASSERT(!infrastructure::FileSystem::writeTextAtomic("", "corrupt_data"));

        // Original file must remain 100% pristine
        readContent = infrastructure::FileSystem::readTextFile(testFile, success);
        ARCLINE_TEST_ASSERT(success);
        ARCLINE_TEST_ASSERT(readContent == originalContent);

        // Cleanup
        infrastructure::FileSystem::deleteFile(testFile);
    }

    std::cout << "[TEST] ProjectFormatDurabilityTests PASSED successfully." << std::endl;
}

} // namespace sonora::tests

namespace arcline::tests {
    using sonora::tests::runProjectFormatDurabilityTests;
}

