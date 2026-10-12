#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "app/AppContext.h"

namespace sonora::ui::components {

class LibraryPanel : public juce::Component {
public:
    explicit LibraryPanel(app::AppContext& context);
    ~LibraryPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void populateItems();

    app::AppContext& context_;
    juce::Label headerLabel_{"LIBRARY", "LIBRARY"};
    juce::TextButton closeButton_{"✕"};
    juce::TextEditor searchFilter_;

    juce::ListBox listBox_;

    class ItemListModel : public juce::ListBoxModel {
    public:
        int getNumRows() override;
        void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
        void listBoxItemClicked(int row, const juce::MouseEvent&) override;
        void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override;

        struct Item {
            std::string category;
            std::string name;
            std::string type;
        };
        std::vector<Item> items;
        std::function<void(const Item&)> onItemDoubleClicked;
    };

    ItemListModel listModel_;
};

} // namespace sonora::ui::components
