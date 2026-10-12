#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "app/AppContext.h"

namespace sonora::ui::dialogs {

class PreferencesDialog : public juce::Component {
public:
    explicit PreferencesDialog(app::AppContext& context);
    ~PreferencesDialog() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;

    std::function<void()> onDismiss;

private:
    void saveAndApply();
    void resetDefaults();

    app::AppContext& context_;

    juce::Label titleLabel_;

    juce::Label themeLabel_;
    juce::ComboBox themeCombo_;

    juce::Label scaleLabel_;
    juce::ComboBox scaleCombo_;

    juce::ToggleButton confirmOnExitToggle_{"Ask for confirmation before quitting application"};
    juce::ToggleButton autoSaveToggle_{"Enable automatic background project snapshot save"};

    juce::TextButton clearRecentsBtn_{"Clear Recent Projects History"};
    juce::TextButton resetDefaultsBtn_{"Reset to Defaults"};

    juce::TextButton saveBtn_{"Save Preferences"};
    juce::TextButton cancelBtn_{"Close"};

    juce::Label infoLabel_;
};

} // namespace sonora::ui::dialogs
