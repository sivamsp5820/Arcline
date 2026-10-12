#include "infrastructure/FileSystem.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace sonora::infrastructure {

namespace fs = std::filesystem;

bool FileSystem::directoryExists(const std::string& path) {
    if (path.empty()) return false;
    std::error_code ec;
    return fs::is_directory(fs::u8path(path), ec);
}

bool FileSystem::fileExists(const std::string& path) {
    if (path.empty()) return false;
    std::error_code ec;
    return fs::is_regular_file(fs::u8path(path), ec);
}

bool FileSystem::createDirectories(const std::string& path) {
    if (path.empty()) return false;
    std::error_code ec;
    return fs::create_directories(fs::u8path(path), ec) || directoryExists(path);
}

bool FileSystem::deleteFile(const std::string& path) {
    if (path.empty()) return false;
    std::error_code ec;
    return fs::remove(fs::u8path(path), ec);
}

bool FileSystem::writeTextAtomic(const std::string& targetPath, const std::string& content) {
    if (targetPath.empty()) return false;

    fs::path target = fs::u8path(targetPath);
    fs::path parent = target.parent_path();
    if (!parent.empty()) {
        std::error_code ec;
        fs::create_directories(parent, ec);
    }

    fs::path temp = target;
    temp += ".tmp";

    {
        std::ofstream out(temp, std::ios::out | std::ios::trunc | std::ios::binary);
        if (!out.is_open()) return false;
        out.write(content.data(), static_cast<std::streamsize>(content.size()));
        if (!out.good()) {
            std::error_code ec;
            fs::remove(temp, ec);
            return false;
        }
    }

    std::error_code ec;
    fs::rename(temp, target, ec);
    if (ec) {
        // Fallback for Windows if target exists and rename fails
        fs::remove(target, ec);
        fs::rename(temp, target, ec);
    }
    return !ec;
}

std::string FileSystem::readTextFile(const std::string& path, bool& outSuccess) {
    outSuccess = false;
    if (!fileExists(path)) return {};

    std::ifstream in(fs::u8path(path), std::ios::in | std::ios::binary);
    if (!in.is_open()) return {};

    std::ostringstream ss;
    ss << in.rdbuf();
    outSuccess = in.good() || in.eof();
    return ss.str();
}

std::string FileSystem::getAppDataDirectory() {
#if defined(_WIN32)
    const char* appData = std::getenv("APPDATA");
    if (appData && *appData) {
        fs::path p = fs::u8path(appData) / "Arcline";
        return p.string();
    }
#elif defined(__APPLE__)
    const char* home = std::getenv("HOME");
    if (home && *home) {
        fs::path p = fs::u8path(home) / "Library" / "Application Support" / "Arcline";
        return p.string();
    }
#else
    const char* configHome = std::getenv("XDG_CONFIG_HOME");
    if (configHome && *configHome) {
        fs::path p = fs::u8path(configHome) / "arcline";
        return p.string();
    }
    const char* home = std::getenv("HOME");
    if (home && *home) {
        fs::path p = fs::u8path(home) / ".config" / "arcline";
        return p.string();
    }
#endif
    return "./ArclineData";
}

std::string FileSystem::getDefaultProjectsDirectory() {
#if defined(_WIN32)
    const char* userProfile = std::getenv("USERPROFILE");
    if (userProfile && *userProfile) {
        fs::path p = fs::u8path(userProfile) / "Documents" / "Arcline Projects";
        return p.string();
    }
#else
    const char* home = std::getenv("HOME");
    if (home && *home) {
        fs::path p = fs::u8path(home) / "Documents" / "Arcline Projects";
        return p.string();
    }
#endif
    return "./Arcline Projects";
}

std::string FileSystem::normalizePath(const std::string& path) {
    if (path.empty()) return {};
    std::error_code ec;
    auto p = fs::u8path(path);
    auto canon = fs::weakly_canonical(p, ec);
    return ec ? p.lexically_normal().string() : canon.string();
}

std::string FileSystem::getParentDirectory(const std::string& path) {
    if (path.empty()) return {};
    return fs::u8path(path).parent_path().string();
}

std::string FileSystem::getFileName(const std::string& path) {
    if (path.empty()) return {};
    return fs::u8path(path).filename().string();
}

std::string FileSystem::combinePath(const std::string& dir, const std::string& file) {
    return (fs::u8path(dir) / fs::u8path(file)).string();
}

std::string FileSystem::toPosixPath(const std::string& path) {
    std::string result = path;
    for (char& c : result) {
        if (c == '\\') c = '/';
    }
    return result;
}

std::string FileSystem::makeRelative(const std::string& basePath, const std::string& targetPath) {
    if (basePath.empty() || targetPath.empty()) return toPosixPath(targetPath);
    std::error_code ec;
    auto base = fs::u8path(basePath);
    auto target = fs::u8path(targetPath);
    auto rel = fs::relative(target, base, ec);
    if (!ec && !rel.empty()) {
        return toPosixPath(rel.string());
    }
    return toPosixPath(targetPath);
}

std::string FileSystem::resolveRelative(const std::string& basePath, const std::string& relativePath) {
    if (relativePath.empty()) return {};
    auto rel = fs::u8path(relativePath);
    if (rel.is_absolute()) {
        return toPosixPath(relativePath);
    }
    auto base = fs::u8path(basePath);
    auto resolved = (base / rel).lexically_normal();
    return toPosixPath(resolved.string());
}

} // namespace sonora::infrastructure
