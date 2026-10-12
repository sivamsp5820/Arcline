#include "domain/UndoHistory.h"
#include "domain/Project.h"
#include "domain/Track.h"
#include "domain/Region.h"
#include <algorithm>

namespace sonora::domain {

// ==============================================================================
// AddTrackAction
// ==============================================================================

AddTrackAction::AddTrackAction(Project& project, TrackType type, std::string name)
    : project_(project), type_(type), name_(std::move(name)) {}

bool AddTrackAction::perform() {
    auto track = project_.addTrack(type_, name_);
    if (!track) return false;
    createdTrackId_ = track->getId();
    return true;
}

bool AddTrackAction::undo() {
    if (createdTrackId_.empty()) return false;
    return project_.removeTrack(createdTrackId_);
}

// ==============================================================================
// RemoveTrackAction
// ==============================================================================

RemoveTrackAction::RemoveTrackAction(Project& project, std::string trackId)
    : project_(project), trackId_(std::move(trackId)) {}

bool RemoveTrackAction::perform() {
    auto& tracks = project_.getTracks();
    for (size_t i = 0; i < tracks.size(); ++i) {
        if (tracks[i] && tracks[i]->getId() == trackId_) {
            removedIndex_ = i;
            removedTrack_ = tracks[i];
            break;
        }
    }
    if (!removedTrack_) return false;
    return project_.removeTrack(trackId_);
}

bool RemoveTrackAction::undo() {
    if (!removedTrack_) return false;
    auto& tracks = project_.getTracks();
    if (removedIndex_ <= tracks.size()) {
        tracks.insert(tracks.begin() + removedIndex_, removedTrack_);
    } else {
        tracks.push_back(removedTrack_);
    }
    return true;
}

// ==============================================================================
// SetTrackVolumeAction
// ==============================================================================

SetTrackVolumeAction::SetTrackVolumeAction(std::shared_ptr<Track> track, float newVolume)
    : track_(std::move(track)), newVolume_(newVolume) {
    if (track_) {
        oldVolume_ = track_->getVolume();
    }
}

bool SetTrackVolumeAction::perform() {
    if (!track_) return false;
    track_->setVolume(newVolume_);
    return true;
}

bool SetTrackVolumeAction::undo() {
    if (!track_) return false;
    track_->setVolume(oldVolume_);
    return true;
}

// ==============================================================================
// SetTrackPanAction
// ==============================================================================

SetTrackPanAction::SetTrackPanAction(std::shared_ptr<Track> track, float newPan)
    : track_(std::move(track)), newPan_(newPan) {
    if (track_) {
        oldPan_ = track_->getPan();
    }
}

bool SetTrackPanAction::perform() {
    if (!track_) return false;
    track_->setPan(newPan_);
    return true;
}

bool SetTrackPanAction::undo() {
    if (!track_) return false;
    track_->setPan(oldPan_);
    return true;
}

// ==============================================================================
// SetTrackMuteAction
// ==============================================================================

SetTrackMuteAction::SetTrackMuteAction(std::shared_ptr<Track> track, bool muted)
    : track_(std::move(track)), newMuted_(muted) {
    if (track_) {
        oldMuted_ = track_->isMuted();
    }
}

bool SetTrackMuteAction::perform() {
    if (!track_) return false;
    track_->setMuted(newMuted_);
    return true;
}

bool SetTrackMuteAction::undo() {
    if (!track_) return false;
    track_->setMuted(oldMuted_);
    return true;
}

// ==============================================================================
// SetTrackSoloAction
// ==============================================================================

SetTrackSoloAction::SetTrackSoloAction(std::shared_ptr<Track> track, bool soloed)
    : track_(std::move(track)), newSoloed_(soloed) {
    if (track_) {
        oldSoloed_ = track_->isSoloed();
    }
}

bool SetTrackSoloAction::perform() {
    if (!track_) return false;
    track_->setSoloed(newSoloed_);
    return true;
}

bool SetTrackSoloAction::undo() {
    if (!track_) return false;
    track_->setSoloed(oldSoloed_);
    return true;
}

// ==============================================================================
// MoveRegionAction
// ==============================================================================

MoveRegionAction::MoveRegionAction(Project& project, std::string regionId,
                                   std::string targetTrackId, int64_t targetStartTick)
    : project_(project), regionId_(std::move(regionId)),
      targetTrackId_(std::move(targetTrackId)), targetStartTick_(targetStartTick) {
    auto r = project_.findRegion(regionId_);
    if (r) {
        oldTrackId_ = r->getTrackId();
        oldStartTick_ = r->getStartTick();
    }
}

bool MoveRegionAction::perform() {
    return project_.moveRegion(regionId_, targetTrackId_, targetStartTick_);
}

bool MoveRegionAction::undo() {
    return project_.moveRegion(regionId_, oldTrackId_, oldStartTick_);
}

// ==============================================================================
// UndoHistory Implementation
// ==============================================================================

UndoHistory::UndoHistory(size_t maxDepth)
    : maxDepth_(maxDepth > 0 ? maxDepth : kDefaultMaxDepth) {}

bool UndoHistory::executeAction(std::unique_ptr<IUndoableAction> action) {
    if (!action) return false;
    if (!action->perform()) return false;

    // A newly executed action clears redo stack
    redoStack_.clear();

    undoStack_.push_back(std::move(action));
    if (undoStack_.size() > maxDepth_) {
        undoStack_.erase(undoStack_.begin());
    }
    return true;
}

bool UndoHistory::undo() {
    if (undoStack_.empty()) return false;
    auto action = std::move(undoStack_.back());
    undoStack_.pop_back();

    if (!action->undo()) {
        return false;
    }

    redoStack_.push_back(std::move(action));
    return true;
}

bool UndoHistory::redo() {
    if (redoStack_.empty()) return false;
    auto action = std::move(redoStack_.back());
    redoStack_.pop_back();

    if (!action->perform()) {
        return false;
    }

    undoStack_.push_back(std::move(action));
    return true;
}

std::string UndoHistory::getUndoActionName() const {
    if (undoStack_.empty()) return {};
    return undoStack_.back()->getName();
}

std::string UndoHistory::getRedoActionName() const {
    if (redoStack_.empty()) return {};
    return redoStack_.back()->getName();
}

void UndoHistory::clear() noexcept {
    undoStack_.clear();
    redoStack_.clear();
}

void UndoHistory::setMaxHistoryDepth(size_t maxDepth) noexcept {
    maxDepth_ = (maxDepth > 0 ? maxDepth : 1);
    while (undoStack_.size() > maxDepth_) {
        undoStack_.erase(undoStack_.begin());
    }
}

} // namespace sonora::domain
