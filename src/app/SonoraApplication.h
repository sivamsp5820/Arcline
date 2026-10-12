#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "app/AppContext.h"
#include "app/MainWindow.h"
#include <memory>

namespace sonora::app {

class SonoraApplication : public juce::JUCEApplication {
public:
    SonoraApplication();
    ~SonoraApplication() override = default;

    const juce::String getApplicationName() override       { return "ARCLINE"; }
    const juce::String getApplicationVersion() override    { return "0.1.0-alpha"; }
    bool moreThanOneInstanceAllowed() override             { return false; }

    void initialise(const juce::String& commandLine) override;
    void shutdown() override;
    void systemRequestedQuit() override;
    void anotherInstanceStarted(const juce::String& commandLine) override;

private:
    std::unique_ptr<AppContext> context_;
    std::unique_ptr<MainWindow> mainWindow_;
};

} // namespace sonora::app
