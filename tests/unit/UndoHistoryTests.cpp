#include "domain/UndoHistory.h"
#include "domain/Project.h"
#include "domain/Track.h"
#include "domain/Region.h"
#include "tests/TestAssert.h"
#include <iostream>

namespace sonora::tests {

void runUndoHistoryTests() {
    std::cout << "[TEST] Running UndoHistoryTests..." << std::endl;

    // 1. Initial empty state
    {
        domain::UndoHistory history(10);
        ARCLINE_TEST_ASSERT(!history.canUndo());
        ARCLINE_TEST_ASSERT(!history.canRedo());
        ARCLINE_TEST_ASSERT(history.getUndoCount() == 0);
        ARCLINE_TEST_ASSERT(history.getRedoCount() == 0);
        ARCLINE_TEST_ASSERT(!history.undo());
        ARCLINE_TEST_ASSERT(!history.redo());
    }

    // 2. AddTrackAction Undo and Redo
    {
        domain::Project project("Test Project", "");
        domain::UndoHistory history(10);
        ARCLINE_TEST_ASSERT(project.getTracks().empty());

        auto addAction = std::make_unique<domain::AddTrackAction>(project, domain::TrackType::Audio, "Audio 1");
        ARCLINE_TEST_ASSERT(history.executeAction(std::move(addAction)));
        ARCLINE_TEST_ASSERT(project.getTracks().size() == 1);
        ARCLINE_TEST_ASSERT(project.getTracks()[0]->getName() == "Audio 1");
        ARCLINE_TEST_ASSERT(history.canUndo());
        ARCLINE_TEST_ASSERT(!history.canRedo());

        // Undo
        ARCLINE_TEST_ASSERT(history.undo());
        ARCLINE_TEST_ASSERT(project.getTracks().empty());
        ARCLINE_TEST_ASSERT(!history.canUndo());
        ARCLINE_TEST_ASSERT(history.canRedo());

        // Redo
        ARCLINE_TEST_ASSERT(history.redo());
        ARCLINE_TEST_ASSERT(project.getTracks().size() == 1);
        ARCLINE_TEST_ASSERT(project.getTracks()[0]->getName() == "Audio 1");
        ARCLINE_TEST_ASSERT(history.canUndo());
        ARCLINE_TEST_ASSERT(!history.canRedo());
    }

    // 3. RemoveTrackAction Undo and Redo preserving track position
    {
        domain::Project project("Test Project", "");
        domain::UndoHistory history(10);

        auto t1 = project.addTrack(domain::TrackType::Audio, "Track 1");
        auto t2 = project.addTrack(domain::TrackType::SoftwareInstrument, "Track 2");
        auto t3 = project.addTrack(domain::TrackType::Audio, "Track 3");
        ARCLINE_TEST_ASSERT(project.getTracks().size() == 3);

        std::string t2Id = t2->getId();
        auto removeAction = std::make_unique<domain::RemoveTrackAction>(project, t2Id);
        ARCLINE_TEST_ASSERT(history.executeAction(std::move(removeAction)));
        ARCLINE_TEST_ASSERT(project.getTracks().size() == 2);
        ARCLINE_TEST_ASSERT(project.getTracks()[0]->getName() == "Track 1");
        ARCLINE_TEST_ASSERT(project.getTracks()[1]->getName() == "Track 3");

        // Undo should restore t2 at index 1
        ARCLINE_TEST_ASSERT(history.undo());
        ARCLINE_TEST_ASSERT(project.getTracks().size() == 3);
        ARCLINE_TEST_ASSERT(project.getTracks()[0]->getName() == "Track 1");
        ARCLINE_TEST_ASSERT(project.getTracks()[1]->getName() == "Track 2");
        ARCLINE_TEST_ASSERT(project.getTracks()[2]->getName() == "Track 3");

        // Redo should remove t2 again
        ARCLINE_TEST_ASSERT(history.redo());
        ARCLINE_TEST_ASSERT(project.getTracks().size() == 2);
        ARCLINE_TEST_ASSERT(project.getTracks()[0]->getName() == "Track 1");
        ARCLINE_TEST_ASSERT(project.getTracks()[1]->getName() == "Track 3");
    }

    // 4. SetTrackVolumeAction and SetTrackPanAction
    {
        domain::Project project("Test Project", "");
        domain::UndoHistory history(10);
        auto track = project.addTrack(domain::TrackType::Audio, "Track Volume Test");
        track->setVolume(1.0f);
        track->setPan(0.0f);

        // Volume
        auto volAction = std::make_unique<domain::SetTrackVolumeAction>(track, 0.75f);
        ARCLINE_TEST_ASSERT(history.executeAction(std::move(volAction)));
        ARCLINE_TEST_ASSERT(track->getVolume() == 0.75f);

        ARCLINE_TEST_ASSERT(history.undo());
        ARCLINE_TEST_ASSERT(track->getVolume() == 1.0f);

        ARCLINE_TEST_ASSERT(history.redo());
        ARCLINE_TEST_ASSERT(track->getVolume() == 0.75f);

        // Pan
        auto panAction = std::make_unique<domain::SetTrackPanAction>(track, -0.5f);
        ARCLINE_TEST_ASSERT(history.executeAction(std::move(panAction)));
        ARCLINE_TEST_ASSERT(track->getPan() == -0.5f);

        ARCLINE_TEST_ASSERT(history.undo());
        ARCLINE_TEST_ASSERT(track->getPan() == 0.0f);

        ARCLINE_TEST_ASSERT(history.redo());
        ARCLINE_TEST_ASSERT(track->getPan() == -0.5f);
    }

    // 5. Mute and Solo Actions
    {
        domain::Project project("Test Project", "");
        domain::UndoHistory history(10);
        auto track = project.addTrack(domain::TrackType::Audio, "MuteSolo Track");
        ARCLINE_TEST_ASSERT(!track->isMuted());
        ARCLINE_TEST_ASSERT(!track->isSoloed());

        history.executeAction(std::make_unique<domain::SetTrackMuteAction>(track, true));
        ARCLINE_TEST_ASSERT(track->isMuted());
        history.undo();
        ARCLINE_TEST_ASSERT(!track->isMuted());
        history.redo();
        ARCLINE_TEST_ASSERT(track->isMuted());

        history.executeAction(std::make_unique<domain::SetTrackSoloAction>(track, true));
        ARCLINE_TEST_ASSERT(track->isSoloed());
        history.undo();
        ARCLINE_TEST_ASSERT(!track->isSoloed());
    }

    // 6. MoveRegionAction
    {
        domain::Project project("Test Project", "");
        domain::UndoHistory history(10);
        auto track = project.addTrack(domain::TrackType::Audio, "Region Track");
        auto region = std::make_shared<domain::Region>("r1", track->getId(), "Take 1",
                                                       domain::RegionType::Audio, 0, 960 * 4);
        track->addRegion(region);

        ARCLINE_TEST_ASSERT(region->getStartTick() == 0);

        auto moveAction = std::make_unique<domain::MoveRegionAction>(project, region->getId(), track->getId(), static_cast<int64_t>(960 * 2));
        ARCLINE_TEST_ASSERT(history.executeAction(std::move(moveAction)));
        ARCLINE_TEST_ASSERT(region->getStartTick() == 960 * 2);

        ARCLINE_TEST_ASSERT(history.undo());
        ARCLINE_TEST_ASSERT(region->getStartTick() == 0);

        ARCLINE_TEST_ASSERT(history.redo());
        ARCLINE_TEST_ASSERT(region->getStartTick() == 960 * 2);
    }

    // 7. Bounded stack depth pruning
    {
        domain::Project project("Test Project", "");
        domain::UndoHistory history(3); // Small capacity of 3
        auto track = project.addTrack(domain::TrackType::Audio, "Capacity Track");

        history.executeAction(std::make_unique<domain::SetTrackVolumeAction>(track, 0.1f));
        history.executeAction(std::make_unique<domain::SetTrackVolumeAction>(track, 0.2f));
        history.executeAction(std::make_unique<domain::SetTrackVolumeAction>(track, 0.3f));
        history.executeAction(std::make_unique<domain::SetTrackVolumeAction>(track, 0.4f));

        // Depth capped at 3
        ARCLINE_TEST_ASSERT(history.getUndoCount() == 3);

        // Can undo 3 times
        ARCLINE_TEST_ASSERT(history.undo()); // back to 0.3
        ARCLINE_TEST_ASSERT(history.undo()); // back to 0.2
        ARCLINE_TEST_ASSERT(history.undo()); // back to 0.1
        ARCLINE_TEST_ASSERT(!history.canUndo()); // oldest was pruned
    }

    // 8. New action clears redo stack
    {
        domain::Project project("Test Project", "");
        domain::UndoHistory history(10);
        auto track = project.addTrack(domain::TrackType::Audio, "Branch Track");

        history.executeAction(std::make_unique<domain::SetTrackVolumeAction>(track, 0.2f));
        history.executeAction(std::make_unique<domain::SetTrackVolumeAction>(track, 0.4f));
        ARCLINE_TEST_ASSERT(history.undo()); // undo to 0.2, redo available
        ARCLINE_TEST_ASSERT(history.canRedo());

        // Perform a new action
        history.executeAction(std::make_unique<domain::SetTrackVolumeAction>(track, 0.8f));
        ARCLINE_TEST_ASSERT(!history.canRedo()); // Redo stack cleared
    }

    std::cout << "  ✓ All UndoHistoryTests passed.\n";
}

} // namespace sonora::tests
