#include "core/CommandRegistry.h"
#include <algorithm>
#include <cctype>

namespace sonora::core {

namespace {
std::string toLower(std::string_view s) {
    std::string result;
    result.reserve(s.size());
    for (char c : s) {
        result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return result;
}
} // namespace

CommandRegistry::CommandRegistry() = default;

bool CommandRegistry::registerCommand(Command command) {
    if (command.id.empty() || !command.execute) {
        return false;
    }
    auto it = std::find_if(commands_.begin(), commands_.end(),
                           [&](const Command& c) { return c.id == command.id; });
    if (it != commands_.end()) {
        *it = std::move(command);
    } else {
        commands_.push_back(std::move(command));
    }
    return true;
}

bool CommandRegistry::unregisterCommand(const std::string& id) {
    auto it = std::find_if(commands_.begin(), commands_.end(),
                           [&](const Command& c) { return c.id == id; });
    if (it != commands_.end()) {
        commands_.erase(it);
        customShortcuts_.erase(id);
        return true;
    }
    return false;
}

bool CommandRegistry::execute(const std::string& id) {
    auto* cmd = findCommand(id);
    if (cmd && cmd->canExecute()) {
        cmd->execute();
        return true;
    }
    return false;
}

const Command* CommandRegistry::findCommand(const std::string& id) const noexcept {
    auto it = std::find_if(commands_.begin(), commands_.end(),
                           [&](const Command& c) { return c.id == id; });
    return it != commands_.end() ? &(*it) : nullptr;
}

std::vector<const Command*> CommandRegistry::search(const std::string& query) const {
    std::vector<const Command*> results;
    if (query.empty()) {
        for (const auto& cmd : commands_) {
            if (cmd.canExecute()) {
                results.push_back(&cmd);
            }
        }
        return results;
    }

    const std::string qLower = toLower(query);
    for (const auto& cmd : commands_) {
        if (!cmd.canExecute()) continue;

        std::string titleLower = toLower(cmd.title);
        std::string catLower = toLower(cmd.category);
        std::string idLower = toLower(cmd.id);

        if (titleLower.find(qLower) != std::string::npos ||
            catLower.find(qLower) != std::string::npos ||
            idLower.find(qLower) != std::string::npos) {
            results.push_back(&cmd);
        }
    }
    return results;
}

void CommandRegistry::setCustomShortcut(const std::string& id, std::string shortcut) {
    customShortcuts_[id] = std::move(shortcut);
}

std::string CommandRegistry::getEffectiveShortcut(const std::string& id) const {
    auto it = customShortcuts_.find(id);
    if (it != customShortcuts_.end() && !it->second.empty()) {
        return it->second;
    }
    auto* cmd = findCommand(id);
    return cmd ? cmd->shortcutText : std::string{};
}

std::string CommandRegistry::getDefaultShortcut(const std::string& id) const {
    auto* cmd = findCommand(id);
    return cmd ? cmd->shortcutText : std::string{};
}

void CommandRegistry::resetCustomShortcuts() noexcept {
    customShortcuts_.clear();
}

std::string CommandRegistry::migrateCommandId(std::string_view legacyId) {
    if (legacyId == "file.new") return "project.new";
    if (legacyId == "file.open") return "project.open";
    if (legacyId == "file.save") return "project.save";
    if (legacyId == "file.close") return "project.close";
    if (legacyId == "view.toggle_pianoroll") return "view.toggle_editor";
    if (legacyId == "view.toggle_audioeditor") return "view.toggle_audio_editor";
    if (legacyId == "transport.play") return "transport.play_pause";
    return std::string(legacyId);
}

void CommandRegistry::loadCustomShortcuts(const std::unordered_map<std::string, std::string>& rawMappings) {
    customShortcuts_.clear();
    for (const auto& [key, shortcut] : rawMappings) {
        std::string migratedKey = migrateCommandId(key);
        customShortcuts_[migratedKey] = shortcut;
    }
}

void CommandRegistry::clear() noexcept {
    commands_.clear();
    customShortcuts_.clear();
}

} // namespace sonora::core
