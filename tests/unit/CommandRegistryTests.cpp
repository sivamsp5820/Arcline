#include "core/CommandRegistry.h"
#include "tests/TestAssert.h"
#include <iostream>

namespace sonora::tests {

void runCommandRegistryTests() {
    std::cout << "[TEST] Running CommandRegistryTests..." << std::endl;

    core::CommandRegistry registry;
    bool executed1 = false;
    bool executed2 = false;

    registry.registerCommand({
        "cmd.test1", "New Session", "Project", "Ctrl+N",
        [&] { executed1 = true; },
        nullptr
    });

    registry.registerCommand({
        "cmd.test2", "Export Audio", "File", "Ctrl+E",
        [&] { executed2 = true; },
        [] { return false; } // Disabled
    });

    // 1. Lookup
    ARCLINE_TEST_ASSERT(registry.findCommand("cmd.test1") != nullptr);
    ARCLINE_TEST_ASSERT(registry.findCommand("cmd.test2") != nullptr);
    ARCLINE_TEST_ASSERT(registry.findCommand("cmd.unknown") == nullptr);

    // 2. Default Shortcuts
    ARCLINE_TEST_ASSERT(registry.getEffectiveShortcut("cmd.test1") == "Ctrl+N");
    ARCLINE_TEST_ASSERT(registry.getEffectiveShortcut("cmd.test2") == "Ctrl+E");

    // 3. Execution
    ARCLINE_TEST_ASSERT(registry.execute("cmd.test1"));
    ARCLINE_TEST_ASSERT(executed1);

    // Disabled execution should fail safely
    ARCLINE_TEST_ASSERT(!registry.execute("cmd.test2"));
    ARCLINE_TEST_ASSERT(!executed2);

    // 4. Custom Shortcut Migration and Overrides
    registry.setCustomShortcut("cmd.test1", "Ctrl+Shift+N");
    ARCLINE_TEST_ASSERT(registry.getEffectiveShortcut("cmd.test1") == "Ctrl+Shift+N");
    ARCLINE_TEST_ASSERT(registry.getCustomShortcuts().size() == 1);
    ARCLINE_TEST_ASSERT(registry.getCustomShortcuts().at("cmd.test1") == "Ctrl+Shift+N");

    // Reset shortcuts
    registry.resetCustomShortcuts();
    ARCLINE_TEST_ASSERT(registry.getEffectiveShortcut("cmd.test1") == "Ctrl+N");
    ARCLINE_TEST_ASSERT(registry.getCustomShortcuts().empty());

    // 5. Legacy Command ID Migration
    ARCLINE_TEST_ASSERT(core::CommandRegistry::migrateCommandId("file.new") == "project.new");
    ARCLINE_TEST_ASSERT(core::CommandRegistry::migrateCommandId("file.open") == "project.open");
    ARCLINE_TEST_ASSERT(core::CommandRegistry::migrateCommandId("file.save") == "project.save");
    ARCLINE_TEST_ASSERT(core::CommandRegistry::migrateCommandId("view.toggle_pianoroll") == "view.toggle_editor");
    ARCLINE_TEST_ASSERT(core::CommandRegistry::migrateCommandId("edit.delete") == "edit.delete"); // Already current
    ARCLINE_TEST_ASSERT(core::CommandRegistry::migrateCommandId("custom.unrecognized") == "custom.unrecognized"); // Passthrough

    // 6. Search query
    auto results = registry.search("session");
    ARCLINE_TEST_ASSERT(results.size() == 1);
    ARCLINE_TEST_ASSERT(results.front()->id == "cmd.test1");

    auto emptyResults = registry.search("nonexistent");
    ARCLINE_TEST_ASSERT(emptyResults.empty());

    std::cout << "  ✓ All CommandRegistryTests passed.\n";
}

} // namespace sonora::tests
