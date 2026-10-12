#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "domain/WorkspaceType.h"

namespace sonora::ui::workspaces {

class WorkspaceView : public juce::Component {
public:
    ~WorkspaceView() override = default;

    virtual domain::WorkspaceType getWorkspaceType() const noexcept = 0;
    virtual void onActivated() {}
    virtual void onDeactivated() {}
};

} // namespace sonora::ui::workspaces
