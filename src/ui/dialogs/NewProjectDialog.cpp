#include "ui/dialogs/NewProjectDialog.h"
#include "ui/theme/SonoraTheme.h"
#include "infrastructure/FileSystem.h"

namespace sonora::ui::dialogs {

NewProjectDialog::NewProjectDialog(app::AppContext& context)
    : context_(context) {
    auto& theme = theme::SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    titleLabel_.setFont(theme.getHeadingFont(18.0f));
    titleLabel_.setColour(juce::Label::textColourId, c.textPrimary);
    titleLabel_.setText("Create New ARCLINE Project", juce::dontSendNotification);
    addAndMakeVisible(titleLabel_);

    nameLabel_.setFont(theme.getCaptionFont(11.0f));
    nameLabel_.setColour(juce::Label::textColourId, c.textMuted);
    nameLabel_.setText("PROJECT NAME", juce::dontSendNotification);
    addAndMakeVisible(nameLabel_);

    nameEditor_.setText("My New Song");
    nameEditor_.setFont(theme.getBodyFont(13.0f));
    nameEditor_.onTextChange = [this] { updatePathPreview(); };
    nameEditor_.onReturnKey = [this] { attemptCreate(); };
    nameEditor_.onEscapeKey = [this] { if (onDismiss) onDismiss(); };
    addAndMakeVisible(nameEditor_);

    locationLabel_.setFont(theme.getCaptionFont(11.0f));
    locationLabel_.setColour(juce::Label::textColourId, c.textMuted);
    locationLabel_.setText("DESTINATION FOLDER", juce::dontSendNotification);
    addAndMakeVisible(locationLabel_);

    std::string defaultDir = context_.getPreferences().getDefaultProjectsDirectory();
    if (defaultDir.empty()) {
        defaultDir = infrastructure::FileSystem::getDefaultProjectsDirectory();
    }
    locationEditor_.setText(defaultDir);
    locationEditor_.setFont(theme.getBodyFont(13.0f));
    locationEditor_.onTextChange = [this] { updatePathPreview(); };
    locationEditor_.onReturnKey = [this] { attemptCreate(); };
    locationEditor_.onEscapeKey = [this] { if (onDismiss) onDismiss(); };
    addAndMakeVisible(locationEditor_);

    browseBtn_.onClick = [this] {
        fileChooser_ = std::make_unique<juce::FileChooser>(
            "Select Project Directory",
            juce::File(locationEditor_.getText()),
            "*");

        auto folderFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
        fileChooser_->launchAsync(folderFlags, [this](const juce::FileChooser& chooser) {
            auto result = chooser.getResult();
            if (result.exists()) {
                locationEditor_.setText(result.getFullPathName());
                updatePathPreview();
            }
        });
    };
    addAndMakeVisible(browseBtn_);

    previewLabel_.setFont(theme.getCaptionFont(11.0f));
    previewLabel_.setColour(juce::Label::textColourId, c.textSecondary);
    addAndMakeVisible(previewLabel_);

    errorLabel_.setFont(theme.getCaptionFont(11.0f));
    errorLabel_.setColour(juce::Label::textColourId, c.error);
    addAndMakeVisible(errorLabel_);

    createBtn_.setColour(juce::TextButton::buttonColourId, c.selectionBlue);
    createBtn_.setColour(juce::TextButton::textColourOffId, c.textPrimary);
    createBtn_.onClick = [this] { attemptCreate(); };
    addAndMakeVisible(createBtn_);

    cancelBtn_.onClick = [this] { if (onDismiss) onDismiss(); };
    addAndMakeVisible(cancelBtn_);

    updatePathPreview();
}

void NewProjectDialog::updatePathPreview() {
    std::string name = nameEditor_.getText().trim().toStdString();
    std::string loc = locationEditor_.getText().trim().toStdString();

    if (name.empty() || loc.empty()) {
        previewLabel_.setText("Location preview: [incomplete path]", juce::dontSendNotification);
    } else {
        std::string fullPath = infrastructure::FileSystem::combinePath(loc, name);
        previewLabel_.setText("Will be created at: " + fullPath, juce::dontSendNotification);
    }
    errorLabel_.setText({}, juce::dontSendNotification);
}

void NewProjectDialog::attemptCreate() {
    std::string name = nameEditor_.getText().trim().toStdString();
    std::string loc = locationEditor_.getText().trim().toStdString();

    if (name.empty()) {
        errorLabel_.setText("Error: Project name cannot be empty.", juce::dontSendNotification);
        return;
    }
    if (loc.empty()) {
        errorLabel_.setText("Error: Destination folder cannot be empty.", juce::dontSendNotification);
        return;
    }

    std::string err;
    if (context_.createNewProject(name, loc, err)) {
        if (onDismiss) onDismiss();
    } else {
        errorLabel_.setText("Error: " + juce::String(err), juce::dontSendNotification);
    }
}

bool NewProjectDialog::keyPressed(const juce::KeyPress& key) {
    if (key == juce::KeyPress::escapeKey) {
        if (onDismiss) onDismiss();
        return true;
    }
    return false;
}

void NewProjectDialog::paint(juce::Graphics& g) {
    auto& theme = theme::SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    auto bounds = getLocalBounds().toFloat();
    g.setColour(c.surfaceElevated);
    g.fillRoundedRectangle(bounds, theme::SonoraTheme::kRadiusLarge);

    g.setColour(c.border);
    g.drawRoundedRectangle(bounds.reduced(0.5f), theme::SonoraTheme::kRadiusLarge, 1.0f);
}

void NewProjectDialog::resized() {
    auto area = getLocalBounds().reduced(24, 20);

    titleLabel_.setBounds(area.removeFromTop(28));
    area.removeFromTop(16);

    nameLabel_.setBounds(area.removeFromTop(16));
    nameEditor_.setBounds(area.removeFromTop(32));
    area.removeFromTop(12);

    locationLabel_.setBounds(area.removeFromTop(16));
    auto locRow = area.removeFromTop(32);
    browseBtn_.setBounds(locRow.removeFromRight(80));
    locRow.removeFromRight(8);
    locationEditor_.setBounds(locRow);
    area.removeFromTop(10);

    previewLabel_.setBounds(area.removeFromTop(20));
    errorLabel_.setBounds(area.removeFromTop(20));
    area.removeFromTop(16);

    auto btnRow = area.removeFromTop(34);
    createBtn_.setBounds(btnRow.removeFromRight(120));
    btnRow.removeFromRight(10);
    cancelBtn_.setBounds(btnRow.removeFromRight(80));
}

} // namespace sonora::ui::dialogs
