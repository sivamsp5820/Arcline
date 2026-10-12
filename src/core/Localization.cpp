#include "core/Localization.h"
#include <sstream>

namespace sonora::core {

Localization& Localization::getInstance() noexcept {
    static Localization instance;
    return instance;
}

Localization::Localization() {
    registerDefaultStrings();
}

void Localization::registerDefaultStrings() {
    std::unordered_map<std::string, std::string> en = {
        {"app.name", "ARCLINE"},
        {"app.title", "ARCLINE"},
        {"app.tagline", "Shape Your Sound."},
        {"menu.file.new", "New Project"},
        {"menu.file.open", "Open Project..."},
        {"menu.file.save", "Save Project"},
        {"menu.file.close", "Close Project"},
        {"transport.play", "Play"},
        {"transport.stop", "Stop"},
        {"transport.record", "Record"},
        {"transport.cycle", "Cycle"},
        {"transport.return_to_zero", "Return to Zero"},
        {"panel.inspector", "Inspector"},
        {"panels.inspector", "Inspector"},
        {"panel.library", "Library"},
        {"panels.library", "Library"},
        {"panel.mixer", "Mixer Console"},
        {"panel.piano_roll", "Piano Roll Editor"},
        {"panel.audio_editor", "Audio Sample Editor"},
        {"mixer.master_volume", "Master Output Volume"},
        {"dialog.quit.title", "Quit ARCLINE"},
        {"dialog.quit.message", "Are you sure you want to exit ARCLINE? Any unsaved changes in the current project will be closed."},
        {"dialog.quit.confirm", "Quit"},
        {"dialog.quit.cancel", "Cancel"},
        {"project.untitled", "Untitled Project"},
        {"project.no_project", "No Project Open"},
        {"status.project_saved", "Project saved: {0}"},
        {"status.track_count", "{0} tracks"}
    };
    catalog_["en-US"] = std::move(en);
}

void Localization::setLanguage(const std::string& langCode) {
    if (!langCode.empty()) {
        currentLanguage_ = langCode;
    }
}

std::string Localization::getLanguage() const {
    return currentLanguage_;
}

std::string Localization::get(std::string_view key, std::string_view defaultText) const {
    std::string keyStr(key);

    // 1. Try active language
    auto langIt = catalog_.find(currentLanguage_);
    if (langIt != catalog_.end()) {
        auto strIt = langIt->second.find(keyStr);
        if (strIt != langIt->second.end()) {
            return strIt->second;
        }
    }

    // 2. Try default fallback language (en-US)
    if (currentLanguage_ != "en-US") {
        auto defIt = catalog_.find("en-US");
        if (defIt != catalog_.end()) {
            auto strIt = defIt->second.find(keyStr);
            if (strIt != defIt->second.end()) {
                return strIt->second;
            }
        }
    }

    // 3. Fallback to defaultText or key
    if (!defaultText.empty()) {
        return std::string(defaultText);
    }
    return keyStr;
}

std::string Localization::format(std::string_view key, std::string_view defaultText, const std::vector<std::string>& args) const {
    std::string text = get(key, defaultText);
    for (size_t i = 0; i < args.size(); ++i) {
        std::string placeholder = "{" + std::to_string(i) + "}";
        size_t pos = 0;
        while ((pos = text.find(placeholder, pos)) != std::string::npos) {
            text.replace(pos, placeholder.length(), args[i]);
            pos += args[i].length();
        }
    }
    return text;
}

void Localization::loadTranslations(const std::string& langCode, const std::unordered_map<std::string, std::string>& translations) {
    auto& targetMap = catalog_[langCode];
    for (const auto& [k, v] : translations) {
        targetMap[k] = v;
    }
}

void Localization::clearTranslations() {
    catalog_.clear();
    registerDefaultStrings();
    currentLanguage_ = "en-US";
}

} // namespace sonora::core
