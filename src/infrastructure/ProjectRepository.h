#pragma once

#include "domain/Project.h"
#include <string>
#include <optional>
#include <string_view>

namespace sonora::infrastructure {

class ProjectRepository {
public:
    static constexpr const char* kProjectFileExtension = ".arcline";
    static constexpr const char* kLegacyProjectFileExtension = ".sonora";
    static constexpr const char* kProjectFormatId = "ARCLINE_PROJECT";
    static constexpr const char* kLegacyProjectFormatId = "SONORA_PROJECT";

    static bool saveToFile(const domain::Project& project, const std::string& filePath, std::string& outError);
    static std::optional<domain::Project> loadFromFile(const std::string& filePath, std::string& outError);

    static std::string serializeToJson(const domain::Project& project);
    static std::optional<domain::Project> deserializeFromJson(const std::string& jsonString, std::string& outError);

    static bool isValidProjectFile(const std::string& filePath);
};

} // namespace sonora::infrastructure
