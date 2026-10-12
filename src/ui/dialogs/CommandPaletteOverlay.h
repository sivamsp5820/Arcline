#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "app/AppContext.h"
#include "core/CommandRegistry.h"
#include <vector>

namespace sonora::ui::dialogs {

class CommandPaletteOverlay : public juce::Component {
public:
    explicit CommandPaletteOverlay(app::AppContext& context);
    ~CommandPaletteOverlay() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void visibilityChanged() override;
    bool keyPressed(const juce::KeyPress& key) override;

    std::function<void()> onDismiss;

private:
    void filterCommands();
    void executeSelected();
    void moveSelection(int delta);

    app::AppContext& context_;
    std::vector<const core::Command*> currentResults_;
    int selectedIndex_{0};

    juce::TextEditor searchEditor_;
    juce::ListBox resultsList_;

    class ListModel : public juce::ListBoxModel {
    public:
        explicit ListModel(CommandPaletteOverlay& owner) : owner_(owner) {}
        int getNumRows() override { return static_cast<int>(owner_.currentResults_.size()); }
        void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
        void listBoxItemClicked(int row, const juce::MouseEvent&) override;
        void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override;

    private:
        CommandPaletteOverlay& owner_;
    };

    ListModel listModel_{*this};
};

} // namespace sonora::ui::dialogs
