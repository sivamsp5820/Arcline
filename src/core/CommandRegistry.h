#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>

namespace sonora::core {

struct Command {
    std::string id;
    std::string title;
    std::string category;
    std::string shortcutText;
    std::function<void()> execute;
    std::function<bool()> isEnabled;

    bool canExecute() const {
        return execute && (!isEnabled || isEnabled());
    }
};

class CommandRegistry {
public:
    CommandRegistry();

    bool registerCommand(Command command);
    bool unregisterCommand(const std::string& id);

    bool execute(const std::string& id);
    const Command* findCommand(const std::string& id) const noexcept;

    std::vector<const Command*> search(const std::string& query) const;
    const std::vector<Command>& getAllCommands() const noexcept { return commands_; }

    // Shortcut management and migration
    void setCustomShortcut(const std::string& id, std::string shortcut);
    std::string getEffectiveShortcut(const std::string& id) const;
    std::string getDefaultShortcut(const std::string& id) const;
    void resetCustomShortcuts() noexcept;

    // Migrate legacy command identifiers to current Phase 1 command identifiers
    static std::string migrateCommandId(std::string_view legacyId);
    void loadCustomShortcuts(const std::unordered_map<std::string, std::string>& rawMappings);
    const std::unordered_map<std::string, std::string>& getCustomShortcuts() const noexcept {
        return customShortcuts_;
    }

    void clear() noexcept;

private:
    std::vector<Command> commands_;
    std::unordered_map<std::string, std::string> customShortcuts_;
};

} // namespace sonora::core
