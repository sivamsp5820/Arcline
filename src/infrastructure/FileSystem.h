#pragma once

#include <string>
#include <vector>

namespace sonora::infrastructure {

class FileSystem {
public:
    static bool directoryExists(const std::string& path);
    static bool fileExists(const std::string& path);
    static bool createDirectories(const std::string& path);
    static bool deleteFile(const std::string& path);
    static bool writeTextAtomic(const std::string& targetPath, const std::string& content);
    static std::string readTextFile(const std::string& path, bool& outSuccess);

    static std::string getAppDataDirectory();
    static std::string getDefaultProjectsDirectory();
    static std::string normalizePath(const std::string& path);
    static std::string getParentDirectory(const std::string& path);
    static std::string getFileName(const std::string& path);
    static std::string combinePath(const std::string& dir, const std::string& file);

    // Cross-platform path portability helpers
    static std::string toPosixPath(const std::string& path);
    static std::string makeRelative(const std::string& basePath, const std::string& targetPath);
    static std::string resolveRelative(const std::string& basePath, const std::string& relativePath);
};

} // namespace sonora::infrastructure
