#include "domain/RecentProjects.h"
#include <algorithm>
#include <chrono>

namespace sonora::domain {

RecentProjectsModel::RecentProjectsModel(size_t maxEntries)
    : maxEntries_(maxEntries > 0 ? maxEntries : 10) {
}

void RecentProjectsModel::addOrPromote(const std::string& name, const std::string& filePath) {
    if (filePath.empty()) return;

    auto it = std::find_if(entries_.begin(), entries_.end(),
                           [&](const RecentProjectEntry& e) { return e.filePath == filePath; });

    int64_t now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    if (it != entries_.end()) {
        it->name = name.empty() ? it->name : name;
        it->lastOpenedTimestamp = now;
        it->existsOnDisk = true;
        std::rotate(entries_.begin(), it, it + 1);
    } else {
        RecentProjectEntry entry;
        entry.name = name.empty() ? "Untitled Project" : name;
        entry.filePath = filePath;
        entry.lastOpenedTimestamp = now;
        entry.existsOnDisk = true;
        entries_.insert(entries_.begin(), std::move(entry));
    }

    if (entries_.size() > maxEntries_) {
        entries_.resize(maxEntries_);
    }
}

bool RecentProjectsModel::remove(const std::string& filePath) {
    auto it = std::find_if(entries_.begin(), entries_.end(),
                           [&](const RecentProjectEntry& e) { return e.filePath == filePath; });
    if (it != entries_.end()) {
        entries_.erase(it);
        return true;
    }
    return false;
}

void RecentProjectsModel::clear() noexcept {
    entries_.clear();
}

void RecentProjectsModel::setMaxEntries(size_t max) noexcept {
    maxEntries_ = max > 0 ? max : 10;
    if (entries_.size() > maxEntries_) {
        entries_.resize(maxEntries_);
    }
}

void RecentProjectsModel::validatePaths(const std::function<bool(const std::string&)>& pathChecker) {
    for (auto& entry : entries_) {
        entry.existsOnDisk = pathChecker(entry.filePath);
    }
}

void RecentProjectsModel::pruneNonExistent(const std::function<bool(const std::string&)>& pathChecker) {
    entries_.erase(
        std::remove_if(entries_.begin(), entries_.end(),
                       [&](const RecentProjectEntry& e) { return !pathChecker(e.filePath); }),
        entries_.end());
}

void RecentProjectsModel::setEntries(std::vector<RecentProjectEntry> entries) {
    entries_ = std::move(entries);
    if (entries_.size() > maxEntries_) {
        entries_.resize(maxEntries_);
    }
}

} // namespace sonora::domain
