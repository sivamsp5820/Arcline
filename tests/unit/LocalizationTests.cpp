#include "core/Localization.h"
#include "tests/TestAssert.h"
#include <iostream>

namespace sonora::tests {

void runLocalizationTests() {
    std::cout << "[TEST] Running LocalizationTests..." << std::endl;

    auto& loc = core::Localization::getInstance();
    loc.clearTranslations();
    loc.setLanguage("en-US");

    // 1. Basic Built-in Catalog Lookup
    ARCLINE_TEST_ASSERT(loc.get("app.title") == "ARCLINE");
    ARCLINE_TEST_ASSERT(loc.get("app.tagline") == "Shape Your Sound.");
    ARCLINE_TEST_ASSERT(loc.get("transport.play") == "Play");
    ARCLINE_TEST_ASSERT(loc.get("transport.stop") == "Stop");
    ARCLINE_TEST_ASSERT(loc.get("menu.file.new") == "New Project");
    ARCLINE_TEST_ASSERT(loc.get("menu.file.save") == "Save Project");

    // Static shortcut tr()
    ARCLINE_TEST_ASSERT(core::Localization::tr("app.title") == "ARCLINE");
    ARCLINE_TEST_ASSERT(core::Localization::tr("panels.inspector") == "Inspector");

    // 2. Fallback to Provided Default Text
    ARCLINE_TEST_ASSERT(loc.get("nonexistent.key.xyz", "Fallback Text") == "Fallback Text");
    ARCLINE_TEST_ASSERT(core::Localization::tr("another.missing.key", "My Default") == "My Default");
    // Missing key with empty default should return key itself
    ARCLINE_TEST_ASSERT(loc.get("unregistered.key") == "unregistered.key");

    // 3. Token Substitution / Format Strings
    ARCLINE_TEST_ASSERT(loc.get("status.project_saved") == "Project saved: {0}");
    std::string formatted = loc.format("status.project_saved", "Project saved: {0}", {"MyMasterSong.arcline"});
    ARCLINE_TEST_ASSERT(formatted == "Project saved: MyMasterSong.arcline");

    // Static shortcut trFormat
    std::string shortcutFmt = core::Localization::trFormat("status.track_count", "{0} tracks loaded", {"16"});
    ARCLINE_TEST_ASSERT(shortcutFmt == "16 tracks");

    // Multiple tokens formatting
    std::string multiFmt = loc.format("custom.two_tokens", "{0} of {1} completed", {"3", "10"});
    ARCLINE_TEST_ASSERT(multiFmt == "3 of 10 completed");

    // 4. Dynamic Translation Catalog Injection and Language Switching
    std::unordered_map<std::string, std::string> germanCatalog = {
        {"app.tagline", "Forme deinen Klang."},
        {"transport.play", "Abspielen"},
        {"transport.stop", "Stopp"},
        {"panels.inspector", "Inspektor"},
        {"menu.file.save", "Projekt speichern"}
    };
    loc.loadTranslations("de-DE", germanCatalog);
    loc.setLanguage("de-DE");
    ARCLINE_TEST_ASSERT(loc.getLanguage() == "de-DE");

    // Verify translated strings
    ARCLINE_TEST_ASSERT(loc.get("app.tagline") == "Forme deinen Klang.");
    ARCLINE_TEST_ASSERT(loc.get("transport.play") == "Abspielen");
    ARCLINE_TEST_ASSERT(loc.get("transport.stop") == "Stopp");
    ARCLINE_TEST_ASSERT(loc.get("menu.file.save") == "Projekt speichern");

    // Fallback to English when key is missing in German catalog
    ARCLINE_TEST_ASSERT(loc.get("app.title") == "ARCLINE");
    ARCLINE_TEST_ASSERT(loc.get("transport.record") == "Record");

    // 5. Reset to English
    loc.setLanguage("en-US");
    ARCLINE_TEST_ASSERT(loc.getLanguage() == "en-US");
    ARCLINE_TEST_ASSERT(loc.get("transport.play") == "Play");

    std::cout << "  ✓ All LocalizationTests passed." << std::endl;
}

} // namespace sonora::tests
