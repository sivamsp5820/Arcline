#include "domain/Preferences.h"
#include "tests/TestAssert.h"
#include <iostream>
#include <vector>
#include <algorithm>

namespace sonora::tests {

struct WindowResolution {
    int width;
    int height;
    const char* label;
};

struct ScaleScenario {
    double scale;
    const char* label;
};

// Layout computation model mirroring MainLayoutComponent rules
struct ComputedLayout {
    int headerHeight{50};
    int inspectorWidth{0};
    int libraryWidth{0};
    int lowerEditorHeight{0};
    int centerTimelineWidth{0};
    int centerTimelineHeight{0};
    
    // Modal dialogs
    int newProjectX{0};
    int newProjectY{0};
    int newProjectW{460};
    int newProjectH{340};
    
    int prefsDialogX{0};
    int prefsDialogY{0};
    int prefsDialogW{480};
    int prefsDialogH{380};
};

static ComputedLayout calculateLayout(
    int windowW, int windowH,
    int prefInspectorW, int prefLibraryW, int prefLowerH,
    bool showInspector, bool showLibrary, bool showLower
) {
    ComputedLayout layout;
    int remainingW = windowW;
    int remainingH = windowH - layout.headerHeight;

    if (showInspector) {
        int maxAllowedW = std::max(180, remainingW / 3);
        layout.inspectorWidth = std::min(prefInspectorW, maxAllowedW);
        remainingW -= layout.inspectorWidth;
    }

    if (showLibrary) {
        int maxAllowedW = std::max(180, remainingW / 3);
        layout.libraryWidth = std::min(prefLibraryW, maxAllowedW);
        remainingW -= layout.libraryWidth;
    }

    if (showLower) {
        int maxAllowedH = std::max(150, remainingH * 3 / 5);
        layout.lowerEditorHeight = std::min(prefLowerH, maxAllowedH);
        remainingH -= layout.lowerEditorHeight;
    }

    layout.centerTimelineWidth = remainingW;
    layout.centerTimelineHeight = remainingH;

    // Centered modals
    layout.newProjectX = (windowW - layout.newProjectW) / 2;
    layout.newProjectY = (windowH - layout.newProjectH) / 2;
    layout.prefsDialogX = (windowW - layout.prefsDialogW) / 2;
    layout.prefsDialogY = (windowH - layout.prefsDialogH) / 2;

    return layout;
}

void runLayoutVerificationTests() {
    std::cout << "[TEST] Running LayoutVerificationTests..." << std::endl;

    // Representative window resolutions to audit
    const std::vector<WindowResolution> resolutions = {
        { 960,  600, "960x600 (Compact Laptop / Scaled Mobile)" },
        { 1200, 780, "1200x780 (Default Standard DAW Window)" },
        { 1920, 1080, "1920x1080 (Full HD 1080p)" },
        { 2560, 1440, "2560x1440 (2K QHD Display)" },
        { 3840, 2160, "3840x2160 (4K UHD HiDPI)" }
    };

    // Representative UI scaling factors
    const std::vector<ScaleScenario> scalingLevels = {
        { 1.0,  "100% (Standard 1.0x)" },
        { 1.25, "125% (1.25x Scaling)" },
        { 1.5,  "150% (1.5x Scaling)" },
        { 2.0,  "200% (HiDPI Retina 2.0x)" }
    };

    domain::Preferences prefs;
    int prefInspW = prefs.getInspectorWidth();      // default 240
    int prefLibW  = prefs.getLibraryWidth();        // default 280
    int prefLowH  = prefs.getLowerEditorHeight();    // default 260

    // 1. Verify layout across all representative resolutions
    for (const auto& res : resolutions) {
        // Case A: Full arrangement (no sidebars open)
        {
            auto layout = calculateLayout(res.width, res.height, prefInspW, prefLibW, prefLowH, false, false, false);
            ARCLINE_TEST_ASSERT(layout.headerHeight == 50);
            ARCLINE_TEST_ASSERT(layout.inspectorWidth == 0);
            ARCLINE_TEST_ASSERT(layout.libraryWidth == 0);
            ARCLINE_TEST_ASSERT(layout.lowerEditorHeight == 0);
            ARCLINE_TEST_ASSERT(layout.centerTimelineWidth == res.width);
            ARCLINE_TEST_ASSERT(layout.centerTimelineHeight == res.height - 50);
            ARCLINE_TEST_ASSERT(layout.centerTimelineWidth > 0);
            ARCLINE_TEST_ASSERT(layout.centerTimelineHeight > 0);
        }

        // Case B: Inspector only open
        {
            auto layout = calculateLayout(res.width, res.height, prefInspW, prefLibW, prefLowH, true, false, false);
            ARCLINE_TEST_ASSERT(layout.inspectorWidth > 0);
            ARCLINE_TEST_ASSERT(layout.inspectorWidth <= prefInspW);
            ARCLINE_TEST_ASSERT(layout.centerTimelineWidth > 0);
            ARCLINE_TEST_ASSERT(layout.centerTimelineWidth == res.width - layout.inspectorWidth);
        }

        // Case C: All panels open simultaneously (dense Logic Pro layout)
        {
            auto layout = calculateLayout(res.width, res.height, prefInspW, prefLibW, prefLowH, true, true, true);
            ARCLINE_TEST_ASSERT(layout.headerHeight == 50);
            ARCLINE_TEST_ASSERT(layout.inspectorWidth >= 180);
            ARCLINE_TEST_ASSERT(layout.libraryWidth >= 180);
            ARCLINE_TEST_ASSERT(layout.lowerEditorHeight >= 150);

            // Center timeline must ALWAYS maintain positive usable width and height
            ARCLINE_TEST_ASSERT(layout.centerTimelineWidth > 0);
            ARCLINE_TEST_ASSERT(layout.centerTimelineHeight > 0);

            // Bounds must perfectly partition the client rectangle without overlap
            int totalAllocatedW = layout.inspectorWidth + layout.centerTimelineWidth + layout.libraryWidth;
            ARCLINE_TEST_ASSERT(totalAllocatedW == res.width);

            int totalAllocatedH = layout.headerHeight + layout.centerTimelineHeight + layout.lowerEditorHeight;
            ARCLINE_TEST_ASSERT(totalAllocatedH == res.height);

            // Modal dialogs must be fully within viewport bounds
            ARCLINE_TEST_ASSERT(layout.newProjectX >= 0);
            ARCLINE_TEST_ASSERT(layout.newProjectY >= 0);
            ARCLINE_TEST_ASSERT(layout.newProjectX + layout.newProjectW <= res.width);
            ARCLINE_TEST_ASSERT(layout.newProjectY + layout.newProjectH <= res.height);

            ARCLINE_TEST_ASSERT(layout.prefsDialogX >= 0);
            ARCLINE_TEST_ASSERT(layout.prefsDialogY >= 0);
            ARCLINE_TEST_ASSERT(layout.prefsDialogX + layout.prefsDialogW <= res.width);
            ARCLINE_TEST_ASSERT(layout.prefsDialogY + layout.prefsDialogH <= res.height);
        }
    }

    // 2. High-DPI Scaling Level Verification
    for (const auto& scale : scalingLevels) {
        // Compute effective logical pixels on a 4K display (3840x2160) under scaling
        int logicalW = static_cast<int>(3840.0 / scale.scale);
        int logicalH = static_cast<int>(2160.0 / scale.scale);

        ARCLINE_TEST_ASSERT(logicalW >= 960);
        ARCLINE_TEST_ASSERT(logicalH >= 600);

        auto layout = calculateLayout(logicalW, logicalH, prefInspW, prefLibW, prefLowH, true, true, true);
        ARCLINE_TEST_ASSERT(layout.centerTimelineWidth > 400); // Plenty of room for multitrack
        ARCLINE_TEST_ASSERT(layout.centerTimelineHeight > 300);
    }

    std::cout << "  ✓ All LayoutVerificationTests passed." << std::endl;
}

} // namespace sonora::tests
