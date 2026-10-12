#pragma once

#include "domain/WorkspaceType.h"
#include <vector>
#include <functional>

namespace sonora::app {

class NavigationController {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void onWorkspaceChanged(domain::WorkspaceType newType, domain::WorkspaceType oldType) = 0;
    };

    NavigationController();

    domain::WorkspaceType getCurrentWorkspace() const noexcept { return currentWorkspace_; }
    void navigateTo(domain::WorkspaceType type);

    bool canGoBack() const noexcept { return !history_.empty(); }
    void goBack();

    void addListener(Listener* listener);
    void removeListener(Listener* listener);

private:
    domain::WorkspaceType currentWorkspace_{domain::WorkspaceType::StartStudio};
    std::vector<domain::WorkspaceType> history_;
    std::vector<Listener*> listeners_;
};

} // namespace sonora::app
