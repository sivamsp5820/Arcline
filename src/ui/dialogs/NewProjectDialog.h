#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "app/AppContext.h"

namespace sonora::ui::dialogs {

class NewProjectDialog : public juce::Component {
public:
    explicit NewProjectDialog(app::AppContext& context);
    ~NewProjectDialog() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;

    std::function<void()> onDismiss;

private:
    void updatePathPreview();
    void attemptCreate();

    app::AppContext& context_;

    juce::Label titleLabel_;
    juce::Label nameLabel_;
    juce::TextEditor nameEditor_;

    juce::Label locationLabel_;
    juce::TextEditor locationEditor_;
    juce::TextButton browseBtn_{"Browse..."};

    juce::Label previewLabel_;
    juce::Label errorLabel_;

    juce::TextButton createBtn_{"Create Project"};
    juce::TextButton cancelBtn_{"Cancel"};

    std::unique_ptr<juce::FileChooser> fileChooser_;
};

} // namespace sonora::ui::dialogs
