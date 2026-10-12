#include "domain/RecentProjects.h"
#include "infrastructure/PreferencesRepository.h"
#include "infrastructure/FileSystem.h"
#include "tests/TestAssert.h"
#include <iostream>

namespace sonora::tests {

void runRecentProjectsTests() {
    std::cout << "[TEST] Running RecentProjectsTests..." << std::endl;

    // 1. Initial State
    domain::RecentProjectsModel recents(3);
    ARCLINE_TEST_ASSERT(recents.empty());
    ARCLINE_TEST_ASSERT(recents.size() == 0);

    // 2. Add entries
    recents.addOrPromote("Song A", "/path/a.arcline");
    recents.addOrPromote("Song B", "/path/b.arcline");
    recents.addOrPromote("Song C", "/path/c.arcline");
    ARCLINE_TEST_ASSERT(recents.size() == 3);
    ARCLINE_TEST_ASSERT(recents.getEntries().front().name == "Song C");

    // 3. Overflow truncation to capacity
    recents.addOrPromote("Song D", "/path/d.arcline");
    ARCLINE_TEST_ASSERT(recents.size() == 3);
    ARCLINE_TEST_ASSERT(recents.getEntries().front().name == "Song D");

    // 4. Promote existing entry to most recent
    recents.addOrPromote("Song B", "/path/b.arcline");
    ARCLINE_TEST_ASSERT(recents.size() == 3);
    ARCLINE_TEST_ASSERT(recents.getEntries().front().name == "Song B");

    // 5. Explicit removal
    ARCLINE_TEST_ASSERT(recents.remove("/path/b.arcline"));
    ARCLINE_TEST_ASSERT(recents.size() == 2);
    ARCLINE_TEST_ASSERT(!recents.remove("/path/nonexistent.arcline"));

    // 6. Nonexistent file pruning / recovery
    recents.pruneNonExistent([](const std::string& p) {
        return p == "/path/d.arcline"; // only d exists on disk
    });
    ARCLINE_TEST_ASSERT(recents.size() == 1);
    ARCLINE_TEST_ASSERT(recents.getEntries().front().filePath == "/path/d.arcline");

    // 7. Disk persistence & recent-project recovery round-trip
    std::string tempDir = "build/test_recents_recovery";
    infrastructure::FileSystem::createDirectories(tempDir);

    infrastructure::PreferencesRepository repo(tempDir);
    domain::RecentProjectsModel diskRecents(5);
    diskRecents.addOrPromote("Album Track 1", "/sessions/track1.arcline");
    diskRecents.addOrPromote("Album Track 2", "/sessions/track2.arcline");
    ARCLINE_TEST_ASSERT(repo.saveRecentProjects(diskRecents));

    // Reload and verify recovery
    auto loadedRecents = repo.loadRecentProjects();
    ARCLINE_TEST_ASSERT(loadedRecents.size() == 2);
    ARCLINE_TEST_ASSERT(loadedRecents.getEntries()[0].name == "Album Track 2");
    ARCLINE_TEST_ASSERT(loadedRecents.getEntries()[1].name == "Album Track 1");

    // 8. Corrupt recents file recovery
    std::string recentsFile = infrastructure::FileSystem::combinePath(tempDir, "recent_projects.json");
    ARCLINE_TEST_ASSERT(infrastructure::FileSystem::writeTextAtomic(recentsFile, "INVALID_CORRUPT_JSON_DATA"));
    auto recoveredRecents = repo.loadRecentProjects();
    ARCLINE_TEST_ASSERT(recoveredRecents.empty()); // Recovers safely with empty list instead of crashing

    // Cleanup
    infrastructure::FileSystem::deleteFile(recentsFile);

    std::cout << "  ✓ All RecentProjectsTests passed.\n";
}

} // namespace sonora::tests
