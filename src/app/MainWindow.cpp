#include "app/MainWindow.h"
#include "ui/theme/SonoraTheme.h"

namespace sonora::app {

MainWindow::MainWindow(juce::String name, AppContext& context)
    : DocumentWindow(name,
                     ui::theme::SonoraTheme::getInstance().getColors().background,
                     DocumentWindow::allButtons),
      context_(context) {
    juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel_);

    // Apply High-DPI and UI Scale Factor from preferences
    float uiScale = static_cast<float>(context_.getPreferences().getUiScaleFactor());
    if (uiScale >= 0.5f && uiScale <= 3.0f) {
        juce::Desktop::getInstance().setGlobalScaleFactor(uiScale);
    }

    setUsingNativeTitleBar(true);

    layoutComponent_ = std::make_unique<ui::components::MainLayoutComponent>(context_);
    setContentNonOwned(layoutComponent_.get(), true);

    setResizable(true, true);
    setResizeLimits(960, 600, 3840, 2160);

    centreWithSize(1200, 780);
    setVisible(true);
}

MainWindow::~MainWindow() {
    clearContentComponent();
}

void MainWindow::closeButtonPressed() {
    if (context_.getPreferences().getConfirmOnExit() && context_.hasActiveProject()) {
        juce::AlertWindow::showOkCancelBox(
            juce::AlertWindow::QuestionIcon,
            "Quit ARCLINE",
            "Are you sure you want to exit ARCLINE? Any unsaved changes in the current project will be closed.",
            "Quit",
            "Cancel",
            this,
            juce::ModalCallbackFunction::create([](int result) {
                if (result == 1) {
                    juce::JUCEApplication::getInstance()->systemRequestedQuit();
                }
            }));
    } else {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
}

} // namespace sonora::app
