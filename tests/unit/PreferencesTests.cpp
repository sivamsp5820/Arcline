#include "domain/Preferences.h"
#include "infrastructure/PreferencesRepository.h"
#include "infrastructure/FileSystem.h"
#include "tests/TestAssert.h"
#include <iostream>

namespace sonora::tests {

void runPreferencesTests() {
    std::cout << "[TEST] Running PreferencesTests..." << std::endl;

    // 1. Defaults and Format Metadata
    domain::Preferences prefs;
    ARCLINE_TEST_ASSERT(std::string_view(domain::Preferences::kConfigFormatId) == "ARCLINE_PREFERENCES");
    ARCLINE_TEST_ASSERT(domain::Preferences::kCurrentSchemaVersion == 1);
    ARCLINE_TEST_ASSERT(prefs.getSchemaVersion() == 1);
    ARCLINE_TEST_ASSERT(prefs.getThemeMode() == "dark");
    ARCLINE_TEST_ASSERT(prefs.getConfirmOnExit() == true);
    ARCLINE_TEST_ASSERT(prefs.getMaxRecentProjects() == 10);
    ARCLINE_TEST_ASSERT(prefs.getUiScaleFactor() == 1.0);
    ARCLINE_TEST_ASSERT(prefs.getInspectorWidth() == domain::Preferences::kDefaultInspectorWidth);
    ARCLINE_TEST_ASSERT(prefs.getLibraryWidth() == domain::Preferences::kDefaultLibraryWidth);
    ARCLINE_TEST_ASSERT(prefs.getLowerEditorHeight() == domain::Preferences::kDefaultLowerEditorHeight);
    ARCLINE_TEST_ASSERT(prefs.isInspectorVisible() == true);
    ARCLINE_TEST_ASSERT(prefs.isLibraryVisible() == false);
    ARCLINE_TEST_ASSERT(prefs.getLowerEditorTab() == "mixer");
    ARCLINE_TEST_ASSERT(prefs.validate());

    // 2. UI Scale bounds clamping
    ARCLINE_TEST_ASSERT(prefs.setUiScaleFactor(1.25));
    ARCLINE_TEST_ASSERT(prefs.getUiScaleFactor() == 1.25);
    ARCLINE_TEST_ASSERT(!prefs.setUiScaleFactor(0.4)); // too small
    ARCLINE_TEST_ASSERT(!prefs.setUiScaleFactor(2.5)); // too large

    // 3. Configurable panel layout clamping
    // Inspector: 180..400
    prefs.setInspectorWidth(100);
    ARCLINE_TEST_ASSERT(prefs.getInspectorWidth() == domain::Preferences::kMinInspectorWidth);
    prefs.setInspectorWidth(900);
    ARCLINE_TEST_ASSERT(prefs.getInspectorWidth() == domain::Preferences::kMaxInspectorWidth);
    prefs.setInspectorWidth(320);
    ARCLINE_TEST_ASSERT(prefs.getInspectorWidth() == 320);

    // Library: 180..450
    prefs.setLibraryWidth(50);
    ARCLINE_TEST_ASSERT(prefs.getLibraryWidth() == domain::Preferences::kMinLibraryWidth);
    prefs.setLibraryWidth(1000);
    ARCLINE_TEST_ASSERT(prefs.getLibraryWidth() == domain::Preferences::kMaxLibraryWidth);
    prefs.setLibraryWidth(350);
    ARCLINE_TEST_ASSERT(prefs.getLibraryWidth() == 350);

    // Lower Editor: 160..600
    prefs.setLowerEditorHeight(80);
    ARCLINE_TEST_ASSERT(prefs.getLowerEditorHeight() == domain::Preferences::kMinLowerEditorHeight);
    prefs.setLowerEditorHeight(1500);
    ARCLINE_TEST_ASSERT(prefs.getLowerEditorHeight() == domain::Preferences::kMaxLowerEditorHeight);
    prefs.setLowerEditorHeight(420);
    ARCLINE_TEST_ASSERT(prefs.getLowerEditorHeight() == 420);

    // Visibility states
    prefs.setInspectorVisible(false);
    ARCLINE_TEST_ASSERT(!prefs.isInspectorVisible());
    prefs.setLibraryVisible(true);
    ARCLINE_TEST_ASSERT(prefs.isLibraryVisible());
    prefs.setLowerEditorTab("pianoroll");
    ARCLINE_TEST_ASSERT(prefs.getLowerEditorTab() == "pianoroll");

    // 4. Theme validation
    prefs.setThemeMode("light");
    ARCLINE_TEST_ASSERT(prefs.validate());
    prefs.setThemeMode("invalid_theme");
    ARCLINE_TEST_ASSERT(!prefs.validate());

    // 5. Reset to defaults
    prefs.resetToDefaults();
    ARCLINE_TEST_ASSERT(prefs.getThemeMode() == "dark");
    ARCLINE_TEST_ASSERT(prefs.getUiScaleFactor() == 1.0);
    ARCLINE_TEST_ASSERT(prefs.getInspectorWidth() == domain::Preferences::kDefaultInspectorWidth);
    ARCLINE_TEST_ASSERT(prefs.getLibraryWidth() == domain::Preferences::kDefaultLibraryWidth);
    ARCLINE_TEST_ASSERT(prefs.getLowerEditorHeight() == domain::Preferences::kDefaultLowerEditorHeight);
    ARCLINE_TEST_ASSERT(prefs.validate());

    // 6. Persistence and Versioned Configuration round-trip
    std::string tempDir = "build/test_config_roundtrip";
    infrastructure::FileSystem::createDirectories(tempDir);

    infrastructure::PreferencesRepository repo(tempDir);
    prefs.setThemeMode("light");
    prefs.setInspectorWidth(310);
    prefs.setLibraryWidth(330);
    prefs.setLowerEditorHeight(450);
    prefs.setUiScaleFactor(1.5);
    prefs.setSampleRate(96000.0);
    prefs.setBufferSize(128);
    ARCLINE_TEST_ASSERT(repo.savePreferences(prefs));

    // Reload from disk
    auto loadedPrefs = repo.loadPreferences();
    ARCLINE_TEST_ASSERT(loadedPrefs.getSchemaVersion() == 1);
    ARCLINE_TEST_ASSERT(loadedPrefs.getThemeMode() == "light");
    ARCLINE_TEST_ASSERT(loadedPrefs.getInspectorWidth() == 310);
    ARCLINE_TEST_ASSERT(loadedPrefs.getLibraryWidth() == 330);
    ARCLINE_TEST_ASSERT(loadedPrefs.getLowerEditorHeight() == 450);
    ARCLINE_TEST_ASSERT(loadedPrefs.getUiScaleFactor() == 1.5);
    ARCLINE_TEST_ASSERT(loadedPrefs.getSampleRate() == 96000.0);
    ARCLINE_TEST_ASSERT(loadedPrefs.getBufferSize() == 128);

    // 7. Defensive fallback on corrupted config file
    std::string prefsFile = infrastructure::FileSystem::combinePath(tempDir, "preferences.json");
    ARCLINE_TEST_ASSERT(infrastructure::FileSystem::writeTextAtomic(prefsFile, "{ not valid json syntax !!!"));
    auto fallbackPrefs = repo.loadPreferences();
    ARCLINE_TEST_ASSERT(fallbackPrefs.getSchemaVersion() == 1);
    ARCLINE_TEST_ASSERT(fallbackPrefs.getThemeMode() == "dark"); // restored default
    ARCLINE_TEST_ASSERT(fallbackPrefs.getInspectorWidth() == domain::Preferences::kDefaultInspectorWidth);

    // Cleanup
    infrastructure::FileSystem::deleteFile(prefsFile);

    std::cout << "  ✓ All PreferencesTests passed.\n";
}

} // namespace sonora::tests
