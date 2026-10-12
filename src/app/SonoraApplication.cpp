#include "app/SonoraApplication.h"
#include "core/Logger.h"
#include "infrastructure/FileSystem.h"
#include <iostream>

namespace sonora::app {

static void setupDemoSession(AppContext& context) {
    // Add demo recent projects
    context.getRecentProjects().addOrPromote("Horizon Synthwave", "C:/Projects/Arcline/Horizon_Synthwave.arcline");
    context.getRecentProjects().addOrPromote("Midnight Chillhop", "C:/Projects/Arcline/Midnight_Chillhop.arcline");
    context.getRecentProjects().addOrPromote("Orchestral Scoring Template", "C:/Projects/Arcline/Orchestral_Scoring.arcline");

    // Create rich arrangement project
    std::string err;
    std::string parentDir = "build/DemoSession";
    infrastructure::FileSystem::createDirectories(parentDir);
    std::string projDir = infrastructure::FileSystem::combinePath(parentDir, "Horizon Synthwave");
    std::string projFile = infrastructure::FileSystem::combinePath(projDir, "Horizon Synthwave.arcline");
    infrastructure::FileSystem::deleteFile(projFile);

    if (!context.createNewProject("Horizon Synthwave", parentDir, err)) {
        context.openProject(projFile, err);
    }
    auto* proj = context.getActiveProject();
    if (!proj) return;

    proj->setTempoBpm(124.0);
    proj->setTimeSignature(4, 4);
    proj->setKeySignature("F# Min");

    // Track 1: Lead Vocals (Audio)
    auto t1 = proj->addTrack(domain::TrackType::Audio, "Lead Vocals");
    t1->setColor(0xFF29B6F6);
    t1->setVolume(0.85f);
    t1->setPan(0.0f);
    auto r1 = std::make_shared<domain::Region>("r1", t1->getId(), "Lead Verse Take 3", domain::RegionType::Audio, 0, 960 * 16);
    r1->generateDemoWaveform(120, 2.5f);
    t1->addRegion(r1);

    // Track 2: Drum Loop Stems (Audio)
    auto t2 = proj->addTrack(domain::TrackType::Audio, "Drum Loop Stems");
    t2->setColor(0xFFFF7043);
    t2->setVolume(0.92f);
    t2->setPan(-0.1f);
    auto r2 = std::make_shared<domain::Region>("r2", t2->getId(), "Full Kit Groove", domain::RegionType::Audio, 0, 960 * 32);
    r2->generateDemoWaveform(160, 4.0f);
    t2->addRegion(r2);

    // Track 3: 808 Sub Bass (Software Instrument)
    auto t3 = proj->addTrack(domain::TrackType::SoftwareInstrument, "808 Sub Bass");
    t3->setColor(0xFFAB47BC);
    t3->setVolume(0.88f);
    auto r3 = std::make_shared<domain::Region>("r3", t3->getId(), "Sub Bassline MIDI", domain::RegionType::Midi, 960 * 4, 960 * 16);
    r3->addNote({36, 0, 960 * 2, 100});
    r3->addNote({38, 960 * 2, 960 * 2, 95});
    r3->addNote({41, 960 * 4, 960 * 4, 110});
    t3->addRegion(r3);

    // Track 4: Analog Poly Synth (Software Instrument)
    auto t4 = proj->addTrack(domain::TrackType::SoftwareInstrument, "Analog Poly Synth");
    t4->setColor(0xFF66BB6A);
    t4->setVolume(0.80f);
    t4->setPan(0.25f);
    auto r4 = std::make_shared<domain::Region>("r4", t4->getId(), "Warm Chords", domain::RegionType::Midi, 0, 960 * 24);
    r4->addNote({60, 0, 960 * 4, 90});
    r4->addNote({63, 0, 960 * 4, 85});
    r4->addNote({67, 0, 960 * 4, 88});
    t4->addRegion(r4);

    // Track 5: Rhythm Electric Guitar (Audio)
    auto t5 = proj->addTrack(domain::TrackType::Audio, "Rhythm Electric Guitar");
    t5->setColor(0xFFFFCA28);
    t5->setVolume(0.78f);
    t5->setPan(-0.35f);
    auto r5 = std::make_shared<domain::Region>("r5", t5->getId(), "Chug Riffs", domain::RegionType::Audio, 960 * 8, 960 * 16);
    r5->generateDemoWaveform(100, 2.0f);
    t5->addRegion(r5);

    // Track 6: Synth Arpeggiator (Software Instrument)
    auto t6 = proj->addTrack(domain::TrackType::SoftwareInstrument, "Synth Arpeggiator");
    t6->setColor(0xFF26A69A);
    t6->setVolume(0.75f);
    t6->setPan(0.40f);
    auto r6 = std::make_shared<domain::Region>("r6", t6->getId(), "16th Arp Pattern", domain::RegionType::Midi, 0, 960 * 32);
    for (int i = 0; i < 16; ++i) {
        r6->addNote({static_cast<uint8_t>(60 + (i % 8) * 2), static_cast<int64_t>(i * 240), 200, 95});
    }
    t6->addRegion(r6);

    // Track 7: SFX Transition Riser (Audio)
    auto t7 = proj->addTrack(domain::TrackType::Audio, "SFX Transition Riser");
    t7->setColor(0xFFEF5350);
    t7->setVolume(0.70f);
    auto r7 = std::make_shared<domain::Region>("r7", t7->getId(), "White Noise Riser", domain::RegionType::Audio, 960 * 28, 960 * 4);
    r7->generateDemoWaveform(60, 1.5f);
    t7->addRegion(r7);

    // Track 8: Master Reverb Aux (Aux Bus)
    auto t8 = proj->addTrack(domain::TrackType::AuxBus, "Master Reverb Aux");
    t8->setColor(0xFF78909C);
    t8->setVolume(0.65f);

    context.notifyProjectChanged();
}

static bool captureAndSave(juce::Component* comp, const juce::File& file, int width, int height) {
    if (!comp) return false;
    comp->setBounds(0, 0, width, height);
    comp->resized();
    auto snapshot = comp->createComponentSnapshot(comp->getLocalBounds());
    file.getParentDirectory().createDirectory();
    file.deleteFile();
    juce::FileOutputStream stream(file);
    if (stream.openedOk()) {
        juce::PNGImageFormat png;
        bool ok = png.writeImageToStream(snapshot, stream);
        core::Logger::getInstance().info("Saved screenshot (" + std::to_string(width) + "x" + std::to_string(height) + ") to: " + file.getFullPathName().toStdString() + " [ok=" + (ok ? "true" : "false") + "]");
        return ok;
    }
    core::Logger::getInstance().warn("Failed to open stream for screenshot file: " + file.getFullPathName().toStdString());
    return false;
}

SonoraApplication::SonoraApplication() = default;

void SonoraApplication::initialise(const juce::String& commandLine) {
    // Initialize logging
    std::string appData = infrastructure::FileSystem::getAppDataDirectory();
    infrastructure::FileSystem::createDirectories(appData);
    core::Logger::getInstance().initialize(
        infrastructure::FileSystem::combinePath(appData, "arcline_debug.log"));
    core::Logger::getInstance().info("Starting ARCLINE Phase 1 Desktop Application - Shape Your Sound.");
    core::Logger::getInstance().info("CommandLine passed: '" + commandLine.toStdString() + "'");
    core::Logger::getInstance().info("getCommandLineParameters: '" + getCommandLineParameters().toStdString() + "'");

    context_ = std::make_unique<AppContext>();
    mainWindow_ = std::make_unique<MainWindow>("ARCLINE - Shape Your Sound.", *context_);

    bool exportRequested = commandLine.contains("--export-screens") || 
                           getCommandLineParameters().contains("--export-screens") ||
                           std::getenv("ARCLINE_EXPORT_SCREENS") != nullptr;

    if (exportRequested) {
        juce::String outDirPath = "screenshots";
        if (const char* envOut = std::getenv("ARCLINE_SCREENS_DIR")) {
            outDirPath = envOut;
        } else if (commandLine.contains("--export-screens=")) {
            outDirPath = commandLine.fromFirstOccurrenceOf("--export-screens=", false, false).upToFirstOccurrenceOf(" ", false, false);
        } else if (getCommandLineParameters().contains("--export-screens=")) {
            outDirPath = getCommandLineParameters().fromFirstOccurrenceOf("--export-screens=", false, false).upToFirstOccurrenceOf(" ", false, false);
        }

        outDirPath = outDirPath.trim();
        juce::File outDir = juce::File::isAbsolutePath(outDirPath)
            ? juce::File(outDirPath)
            : juce::File::getCurrentWorkingDirectory().getChildFile(outDirPath);
        outDir.createDirectory();

        core::Logger::getInstance().info("Target screens output dir: " + outDir.getFullPathName().toStdString());

        setupDemoSession(*context_);
        auto* layout = mainWindow_->getLayoutComponent();
        int width = 1440;
        int height = 900;

        // 1. Start Studio Workspace
        context_->getNavigation().navigateTo(domain::WorkspaceType::StartStudio);
        layout->hideAllDialogs();
        captureAndSave(layout, outDir.getChildFile("01_start_studio_workspace.png"), width, height);

        // 2. Multitrack Arrangement View (Inspector + Library open, Lower Editor closed)
        context_->getNavigation().navigateTo(domain::WorkspaceType::Arrangement);
        if (!context_->isInspectorVisible()) context_->toggleInspector();
        if (!context_->isLibraryVisible()) context_->toggleLibrary();
        context_->setLowerEditorTab(LowerEditorTab::None);
        layout->hideAllDialogs();
        captureAndSave(layout, outDir.getChildFile("02_arrangement_main_workspace.png"), width, height);

        // 3. Mixer Console Dock (Lower Editor Mixer active)
        if (context_->isLibraryVisible()) context_->toggleLibrary();
        context_->setLowerEditorTab(LowerEditorTab::Mixer);
        layout->hideAllDialogs();
        captureAndSave(layout, outDir.getChildFile("03_mixer_console_dock.png"), width, height);

        // 4. Piano Roll Editor (Lower Editor Piano Roll active)
        if (auto* proj = context_->getActiveProject()) {
            if (proj->getTracks().size() > 2) {
                auto tMidi = proj->getTracks()[2];
                if (!tMidi->getRegions().empty()) {
                    proj->selectTrack(tMidi->getId());
                    proj->selectRegion(tMidi->getRegions()[0]->getId());
                    context_->notifySelectionChanged();
                }
            }
        }
        context_->setLowerEditorTab(LowerEditorTab::PianoRoll);
        layout->hideAllDialogs();
        captureAndSave(layout, outDir.getChildFile("04_piano_roll_editor.png"), width, height);

        // 5. Audio Sample Editor (Lower Editor Audio Editor active)
        if (auto* proj = context_->getActiveProject()) {
            if (!proj->getTracks().empty()) {
                auto tAudio = proj->getTracks()[0];
                if (!tAudio->getRegions().empty()) {
                    proj->selectTrack(tAudio->getId());
                    proj->selectRegion(tAudio->getRegions()[0]->getId());
                    context_->notifySelectionChanged();
                }
            }
        }
        context_->setLowerEditorTab(LowerEditorTab::AudioEditor);
        layout->hideAllDialogs();
        captureAndSave(layout, outDir.getChildFile("05_audio_sample_editor.png"), width, height);

        // 6. Command Palette Modal (Ctrl+K overlay)
        context_->setLowerEditorTab(LowerEditorTab::Mixer);
        layout->hideAllDialogs();
        layout->setBounds(0, 0, width, height);
        layout->resized();
        layout->showCommandPalette();
        captureAndSave(layout, outDir.getChildFile("06_command_palette_modal.png"), width, height);

        // 7. New Project Dialog
        layout->hideAllDialogs();
        layout->setBounds(0, 0, width, height);
        layout->resized();
        layout->showNewProjectDialog();
        captureAndSave(layout, outDir.getChildFile("07_new_project_dialog.png"), width, height);

        // 8. Preferences Dialog
        layout->hideAllDialogs();
        layout->setBounds(0, 0, width, height);
        layout->resized();
        layout->showPreferencesDialog();
        captureAndSave(layout, outDir.getChildFile("08_preferences_dialog.png"), width, height);

        core::Logger::getInstance().info("Exported 8 application screens to: " + outDir.getFullPathName().toStdString());
        std::cout << "[SUCCESS] Exported 8 application screens to: " << outDir.getFullPathName().toStdString() << std::endl;
        quit();
        return;
    }
}

void SonoraApplication::shutdown() {
    core::Logger::getInstance().info("Shutting down ARCLINE cleanly");
    mainWindow_.reset();
    context_.reset();
}

void SonoraApplication::systemRequestedQuit() {
    quit();
}

void SonoraApplication::anotherInstanceStarted(const juce::String& /*commandLine*/) {
    if (mainWindow_) {
        mainWindow_->toFront(true);
    }
}

} // namespace sonora::app
