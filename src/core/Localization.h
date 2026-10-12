#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace sonora::core {

class Localization {
public:
    static Localization& getInstance() noexcept;

    void setLanguage(const std::string& langCode);
    std::string getLanguage() const;

    std::string get(std::string_view key, std::string_view defaultText = "") const;
    std::string format(std::string_view key, std::string_view defaultText, const std::vector<std::string>& args) const;

    void loadTranslations(const std::string& langCode, const std::unordered_map<std::string, std::string>& translations);
    void clearTranslations();

    // Convenience static shortcuts
    static std::string tr(std::string_view key, std::string_view defaultText = "") {
        return getInstance().get(key, defaultText);
    }

    static std::string trFormat(std::string_view key, std::string_view defaultText, const std::vector<std::string>& args) {
        return getInstance().format(key, defaultText, args);
    }

private:
    Localization();
    void registerDefaultStrings();

    std::string currentLanguage_{"en-US"};
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> catalog_;
};

} // namespace sonora::core
