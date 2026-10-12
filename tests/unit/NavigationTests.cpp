#include "app/NavigationController.h"
#include <cassert>
#include <iostream>

namespace sonora::tests {

class MockNavListener : public app::NavigationController::Listener {
public:
    void onWorkspaceChanged(domain::WorkspaceType newType, domain::WorkspaceType oldType) override {
        lastNew = newType;
        lastOld = oldType;
        changeCount++;
    }

    domain::WorkspaceType lastNew{domain::WorkspaceType::StartStudio};
    domain::WorkspaceType lastOld{domain::WorkspaceType::StartStudio};
    int changeCount{0};
};

void runNavigationTests() {
    std::cout << "[TEST] Running NavigationTests...\n";

    app::NavigationController nav;
    MockNavListener listener;
    nav.addListener(&listener);

    assert(nav.getCurrentWorkspace() == domain::WorkspaceType::StartStudio);
    assert(!nav.canGoBack());

    // Navigate to Arrangement
    nav.navigateTo(domain::WorkspaceType::Arrangement);
    assert(nav.getCurrentWorkspace() == domain::WorkspaceType::Arrangement);
    assert(listener.changeCount == 1);
    assert(listener.lastNew == domain::WorkspaceType::Arrangement);
    assert(listener.lastOld == domain::WorkspaceType::StartStudio);
    assert(nav.canGoBack());

    // Back navigation
    nav.goBack();
    assert(nav.getCurrentWorkspace() == domain::WorkspaceType::StartStudio);
    assert(!nav.canGoBack());
    assert(listener.changeCount == 2);

    nav.removeListener(&listener);
    std::cout << "  ✓ All NavigationTests passed.\n";
}

} // namespace sonora::tests
