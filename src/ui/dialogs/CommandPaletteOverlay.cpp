#include "ui/dialogs/CommandPaletteOverlay.h"
#include "ui/theme/SonoraTheme.h"

namespace sonora::ui::dialogs {

CommandPaletteOverlay::CommandPaletteOverlay(app::AppContext& context)
    : context_(context) {
    auto& theme = theme::SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    searchEditor_.setTextToShowWhenEmpty("Type a command or action name... (e.g., 'New Project', 'Universe')", c.textMuted);
    searchEditor_.setFont(theme.getBodyFont(14.0f));
    searchEditor_.onTextChange = [this] { filterCommands(); };
    searchEditor_.onReturnKey = [this] { executeSelected(); };
    searchEditor_.onEscapeKey = [this] { if (onDismiss) onDismiss(); };
    addAndMakeVisible(searchEditor_);

    resultsList_.setModel(&listModel_);
    resultsList_.setRowHeight(36);
    resultsList_.setColour(juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    resultsList_.setColour(juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(resultsList_);

    filterCommands();
}

void CommandPaletteOverlay::visibilityChanged() {
    if (isVisible()) {
        searchEditor_.clear();
        filterCommands();
        searchEditor_.grabKeyboardFocus();
    }
}

void CommandPaletteOverlay::filterCommands() {
    std::string query = searchEditor_.getText().trim().toStdString();
    currentResults_ = context_.getCommandRegistry().search(query);
    selectedIndex_ = currentResults_.empty() ? -1 : 0;
    resultsList_.updateContent();
    resultsList_.selectRow(selectedIndex_);
    resultsList_.repaint();
}

void CommandPaletteOverlay::executeSelected() {
    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(currentResults_.size())) {
        const auto* cmd = currentResults_[static_cast<size_t>(selectedIndex_)];
        if (cmd && cmd->canExecute()) {
            if (onDismiss) onDismiss();
            cmd->execute();
        }
    }
}

void CommandPaletteOverlay::moveSelection(int delta) {
    if (currentResults_.empty()) return;
    int count = static_cast<int>(currentResults_.size());
    selectedIndex_ = (selectedIndex_ + delta + count) % count;
    resultsList_.selectRow(selectedIndex_);
    resultsList_.scrollToEnsureRowIsOnscreen(selectedIndex_);
    resultsList_.repaint();
}

bool CommandPaletteOverlay::keyPressed(const juce::KeyPress& key) {
    if (key == juce::KeyPress::escapeKey) {
        if (onDismiss) onDismiss();
        return true;
    }
    if (key == juce::KeyPress::upKey) {
        moveSelection(-1);
        return true;
    }
    if (key == juce::KeyPress::downKey) {
        moveSelection(1);
        return true;
    }
    if (key == juce::KeyPress::returnKey) {
        executeSelected();
        return true;
    }
    return false;
}

void CommandPaletteOverlay::paint(juce::Graphics& g) {
    auto& theme = theme::SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    // Dark backdrop overlay
    g.fillAll(juce::Colours::black.withAlpha(0.6f));

    // Centered modal card
    int cardW = 560;
    int cardH = 380;
    int cardX = (getWidth() - cardW) / 2;
    int cardY = 80;

    juce::Rectangle<float> card(static_cast<float>(cardX), static_cast<float>(cardY),
                                static_cast<float>(cardW), static_cast<float>(cardH));

    g.setColour(c.surfaceElevated);
    g.fillRoundedRectangle(card, theme::SonoraTheme::kRadiusLarge);

    g.setColour(c.border);
    g.drawRoundedRectangle(card.reduced(0.5f), theme::SonoraTheme::kRadiusLarge, 1.0f);

    // Footer hint bar
    g.setColour(c.borderSubtle);
    g.fillRect(static_cast<float>(cardX), static_cast<float>(cardY + cardH - 30),
               static_cast<float>(cardW), 1.0f);

    g.setFont(theme.getCaptionFont(10.5f));
    g.setColour(c.textMuted);
    g.drawText("↑/↓ Navigate • Enter to Execute • Escape to Close",
               cardX + 16, cardY + cardH - 26, cardW - 32, 20, juce::Justification::centredLeft);
}

void CommandPaletteOverlay::resized() {
    int cardW = 560;
    int cardH = 380;
    int cardX = (getWidth() - cardW) / 2;
    int cardY = 80;

    searchEditor_.setBounds(cardX + 16, cardY + 16, cardW - 32, 36);
    resultsList_.setBounds(cardX + 16, cardY + 60, cardW - 32, cardH - 96);
}

// ListModel implementation
void CommandPaletteOverlay::ListModel::paintListBoxItem(int rowNumber, juce::Graphics& g,
                                                        int width, int height, bool rowIsSelected) {
    if (rowNumber < 0 || rowNumber >= static_cast<int>(owner_.currentResults_.size())) {
        return;
    }

    auto& theme = theme::SonoraTheme::getInstance();
    const auto& c = theme.getColors();
    const auto* cmd = owner_.currentResults_[static_cast<size_t>(rowNumber)];
    if (!cmd) return;

    auto r = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(2.0f, 2.0f);

    if (rowIsSelected) {
        g.setColour(c.surfaceActive);
        g.fillRoundedRectangle(r, theme::SonoraTheme::kRadiusSmall);
        g.setColour(c.selectionBlue);
        g.fillRect(r.removeFromLeft(3.0f));
    }

    // Category tag
    g.setFont(theme.getCaptionFont(10.0f));
    g.setColour(c.textMuted);
    g.drawText(cmd->category.empty() ? "Action" : cmd->category,
               12, 0, 80, height, juce::Justification::centredLeft);

    // Title
    g.setFont(theme.getBodyFont(13.0f));
    g.setColour(rowIsSelected ? c.textPrimary : c.textSecondary);
    g.drawText(cmd->title, 100, 0, width - 200, height, juce::Justification::centredLeft);

    // Shortcut
    if (!cmd->shortcutText.empty()) {
        g.setFont(theme.getCaptionFont(11.0f));
        g.setColour(c.textMuted);
        g.drawText(cmd->shortcutText, width - 90, 0, 80, height, juce::Justification::centredRight);
    }
}

void CommandPaletteOverlay::ListModel::listBoxItemClicked(int row, const juce::MouseEvent&) {
    owner_.selectedIndex_ = row;
    owner_.resultsList_.repaint();
}

void CommandPaletteOverlay::ListModel::listBoxItemDoubleClicked(int row, const juce::MouseEvent&) {
    owner_.selectedIndex_ = row;
    owner_.executeSelected();
}

} // namespace sonora::ui::dialogs
