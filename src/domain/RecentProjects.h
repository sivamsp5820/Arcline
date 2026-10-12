#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace sonora::domain {

struct RecentProjectEntry {
    std::string name;
    std::string filePath;
    int64_t lastOpenedTimestamp{0};
    bool existsOnDisk{true};

    bool operator==(const RecentProjectEntry& other) const noexcept {
        return filePath == other.filePath;
    }
};

class RecentProjectsModel {
public:
    explicit RecentProjectsModel(size_t maxEntries = 10);

    void addOrPromote(const std::string& name, const std::string& filePath);
    bool remove(const std::string& filePath);
    void clear() noexcept;

    const std::vector<RecentProjectEntry>& getEntries() const noexcept { return entries_; }
    size_t size() const noexcept { return entries_.size(); }
    bool empty() const noexcept { return entries_.empty(); }

    void setMaxEntries(size_t max) noexcept;
    size_t getMaxEntries() const noexcept { return maxEntries_; }

    void validatePaths(const std::function<bool(const std::string&)>& pathChecker);
    void pruneNonExistent(const std::function<bool(const std::string&)>& pathChecker);

    void setEntries(std::vector<RecentProjectEntry> entries);

private:
    std::vector<RecentProjectEntry> entries_;
    size_t maxEntries_{10};
};

} // namespace sonora::domain
