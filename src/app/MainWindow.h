#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "app/AppContext.h"
#include "ui/theme/SonoraLookAndFeel.h"
#include "ui/components/MainLayoutComponent.h"

namespace sonora::app {

class MainWindow : public juce::DocumentWindow {
public:
    explicit MainWindow(juce::String name, AppContext& context);
    ~MainWindow() override;

    void closeButtonPressed() override;
    ui::components::MainLayoutComponent* getLayoutComponent() const noexcept { return layoutComponent_.get(); }

private:
    AppContext& context_;
    ui::theme::SonoraLookAndFeel lookAndFeel_;
    std::unique_ptr<ui::components::MainLayoutComponent> layoutComponent_;
};

} // namespace sonora::app
