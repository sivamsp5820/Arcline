#include "app/NavigationController.h"
#include <algorithm>

namespace sonora::app {

NavigationController::NavigationController() = default;

void NavigationController::navigateTo(domain::WorkspaceType type) {
    if (type == currentWorkspace_) return;

    domain::WorkspaceType oldType = currentWorkspace_;
    history_.push_back(oldType);
    currentWorkspace_ = type;

    for (auto* l : listeners_) {
        if (l) l->onWorkspaceChanged(currentWorkspace_, oldType);
    }
}

void NavigationController::goBack() {
    if (history_.empty()) return;

    domain::WorkspaceType prev = history_.back();
    history_.pop_back();

    domain::WorkspaceType oldType = currentWorkspace_;
    currentWorkspace_ = prev;

    for (auto* l : listeners_) {
        if (l) l->onWorkspaceChanged(currentWorkspace_, oldType);
    }
}

void NavigationController::addListener(Listener* listener) {
    if (listener && std::find(listeners_.begin(), listeners_.end(), listener) == listeners_.end()) {
        listeners_.push_back(listener);
    }
}

void NavigationController::removeListener(Listener* listener) {
    listeners_.erase(std::remove(listeners_.begin(), listeners_.end(), listener), listeners_.end());
}

} // namespace sonora::app
