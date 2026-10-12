#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace sonora::domain {

class Project;
class Track;
class Region;
enum class TrackType;

/**
 * Interface representing an undoable and redoable domain action.
 */
class IUndoableAction {
public:
    virtual ~IUndoableAction() = default;
    virtual bool perform() = 0;
    virtual bool undo() = 0;
    virtual std::string getName() const = 0;
};

// ==============================================================================
// Concrete Domain Actions Hooked to Phase 1 Operations
// ==============================================================================

class AddTrackAction : public IUndoableAction {
public:
    AddTrackAction(Project& project, TrackType type, std::string name);
    bool perform() override;
    bool undo() override;
    std::string getName() const override { return "Add Track"; }

private:
    Project& project_;
    TrackType type_;
    std::string name_;
    std::string createdTrackId_;
};

class RemoveTrackAction : public IUndoableAction {
public:
    RemoveTrackAction(Project& project, std::string trackId);
    bool perform() override;
    bool undo() override;
    std::string getName() const override { return "Delete Track"; }

private:
    Project& project_;
    std::string trackId_;
    std::shared_ptr<Track> removedTrack_;
    size_t removedIndex_{0};
};

class SetTrackVolumeAction : public IUndoableAction {
public:
    SetTrackVolumeAction(std::shared_ptr<Track> track, float newVolume);
    bool perform() override;
    bool undo() override;
    std::string getName() const override { return "Change Track Volume"; }

private:
    std::shared_ptr<Track> track_;
    float oldVolume_{1.0f};
    float newVolume_{1.0f};
};

class SetTrackPanAction : public IUndoableAction {
public:
    SetTrackPanAction(std::shared_ptr<Track> track, float newPan);
    bool perform() override;
    bool undo() override;
    std::string getName() const override { return "Change Track Pan"; }

private:
    std::shared_ptr<Track> track_;
    float oldPan_{0.0f};
    float newPan_{0.0f};
};

class SetTrackMuteAction : public IUndoableAction {
public:
    SetTrackMuteAction(std::shared_ptr<Track> track, bool muted);
    bool perform() override;
    bool undo() override;
    std::string getName() const override { return "Toggle Track Mute"; }

private:
    std::shared_ptr<Track> track_;
    bool oldMuted_{false};
    bool newMuted_{false};
};

class SetTrackSoloAction : public IUndoableAction {
public:
    SetTrackSoloAction(std::shared_ptr<Track> track, bool soloed);
    bool perform() override;
    bool undo() override;
    std::string getName() const override { return "Toggle Track Solo"; }

private:
    std::shared_ptr<Track> track_;
    bool oldSoloed_{false};
    bool newSoloed_{false};
};

class MoveRegionAction : public IUndoableAction {
public:
    MoveRegionAction(Project& project, std::string regionId,
                     std::string targetTrackId, int64_t targetStartTick);
    bool perform() override;
    bool undo() override;
    std::string getName() const override { return "Move Region"; }

private:
    Project& project_;
    std::string regionId_;
    std::string oldTrackId_;
    std::string targetTrackId_;
    int64_t oldStartTick_{0};
    int64_t targetStartTick_{0};
};

// ==============================================================================
// UndoHistory Management Stack
// ==============================================================================

class UndoHistory {
public:
    static constexpr size_t kDefaultMaxDepth = 100;

    explicit UndoHistory(size_t maxDepth = kDefaultMaxDepth);
    ~UndoHistory() = default;

    bool executeAction(std::unique_ptr<IUndoableAction> action);
    bool undo();
    bool redo();

    bool canUndo() const noexcept { return !undoStack_.empty(); }
    bool canRedo() const noexcept { return !redoStack_.empty(); }

    std::string getUndoActionName() const;
    std::string getRedoActionName() const;

    size_t getUndoCount() const noexcept { return undoStack_.size(); }
    size_t getRedoCount() const noexcept { return redoStack_.size(); }

    void clear() noexcept;
    void setMaxHistoryDepth(size_t maxDepth) noexcept;

private:
    size_t maxDepth_{kDefaultMaxDepth};
    std::vector<std::unique_ptr<IUndoableAction>> undoStack_;
    std::vector<std::unique_ptr<IUndoableAction>> redoStack_;
};

} // namespace sonora::domain
