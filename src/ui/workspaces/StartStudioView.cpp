#include "ui/workspaces/StartStudioView.h"
#include "ui/theme/SonoraTheme.h"
#include "infrastructure/FileSystem.h"

namespace sonora::ui::workspaces {

StartStudioView::StartStudioView(app::AppContext& context)
    : context_(context) {
    context_.addProjectListener(this);

    auto& theme = theme::SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    // Primary action buttons
    newProjectBtn_.setColour(juce::TextButton::buttonColourId, c.selectionBlue);
    newProjectBtn_.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    newProjectBtn_.onClick = [this] {
        if (context_.onRequestNewProjectDialog) context_.onRequestNewProjectDialog();
    };
    addAndMakeVisible(newProjectBtn_);

    openProjectBtn_.onClick = [this] {
        if (context_.onRequestOpenProjectDialog) context_.onRequestOpenProjectDialog();
    };
    addAndMakeVisible(openProjectBtn_);

    loadDemoSessionBtn_.setTooltip("Jump into a complete Logic Pro X 4-track vocal, piano, bass, and drum arrangement");
    loadDemoSessionBtn_.onClick = [this] {
        createProjectFromTemplate("Audio Recording", "Logic Demo Multitrack");
    };
    addAndMakeVisible(loadDemoSessionBtn_);

    // Studio Templates
    auto setupTmpl = [this](juce::TextButton& btn, const std::string& tmpl, const std::string& name) {
        btn.onClick = [this, tmpl, name] {
            createProjectFromTemplate(tmpl, name);
        };
        addAndMakeVisible(btn);
    };

    setupTmpl(recordingTemplateBtn_, "Audio Recording", "Recording Session");
    setupTmpl(electronicTemplateBtn_, "Electronic / Beat Production", "Electronic Track");
    setupTmpl(songwriterTemplateBtn_, "Songwriter", "Songwriter Acoustic");
    setupTmpl(emptyTemplateBtn_, "Empty Project", "Blank Session");

    // Headers
    recentHeaderLabel_.setFont(theme.getHeadingFont(13.0f));
    recentHeaderLabel_.setColour(juce::Label::textColourId, c.textPrimary);
    recentHeaderLabel_.setText("RECENT SESSIONS", juce::dontSendNotification);
    addAndMakeVisible(recentHeaderLabel_);

    templatesHeaderLabel_.setFont(theme.getHeadingFont(13.0f));
    templatesHeaderLabel_.setColour(juce::Label::textColourId, c.textPrimary);
    templatesHeaderLabel_.setText("PROJECT TEMPLATES & STARTERS", juce::dontSendNotification);
    addAndMakeVisible(templatesHeaderLabel_);

    emptyRecentMessageLabel_.setFont(theme.getBodyFont(13.0f));
    emptyRecentMessageLabel_.setColour(juce::Label::textColourId, c.textMuted);
    emptyRecentMessageLabel_.setText("No recent projects found. Choose a template or click '＋ New Project'.", juce::dontSendNotification);
    addAndMakeVisible(emptyRecentMessageLabel_);

    // Viewport for recent projects
    recentViewport_.setViewedComponent(&recentContainer_, false);
    recentViewport_.setScrollBarsShown(true, false);
    addAndMakeVisible(recentViewport_);

    refreshRecentProjects();
}

StartStudioView::~StartStudioView() {
    context_.removeProjectListener(this);
}

void StartStudioView::createProjectFromTemplate(const std::string& templateName, const std::string& defaultName) {
    auto defaultDir = infrastructure::FileSystem::getDefaultProjectsDirectory();
    std::string err;
    context_.createNewProjectFromTemplate(templateName, defaultName, defaultDir, err);
}

void StartStudioView::onActivated() {
    refreshRecentProjects();
}

void StartStudioView::onProjectChanged(const domain::Project* /*project*/) {
    refreshRecentProjects();
}

void StartStudioView::refreshRecentProjects() {
    recentContainer_.removeAllChildren();
    recentRows_.clear();

    const auto& entries = context_.getRecentProjects().getEntries();
    bool hasEntries = !entries.empty();
    emptyRecentMessageLabel_.setVisible(!hasEntries);
    recentViewport_.setVisible(hasEntries);

    if (!hasEntries) {
        resized();
        return;
    }

    auto& theme = theme::SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    int rowHeight = 44;
    int totalHeight = static_cast<int>(entries.size()) * (rowHeight + 6);
    recentContainer_.setSize(recentViewport_.getWidth() > 0 ? recentViewport_.getWidth() - 16 : 400, totalHeight);

    int y = 0;
    for (const auto& entry : entries) {
        RecentRowItem item;
        item.name = entry.name;
        item.path = entry.filePath;

        item.label = std::make_unique<juce::Label>();
        item.label->setFont(theme.getBodyFont(13.0f));
        item.label->setColour(juce::Label::textColourId, entry.existsOnDisk ? c.textPrimary : c.textMuted);
        juce::String txt = juce::String(entry.name);
        if (!entry.existsOnDisk) {
            txt += " [Path Missing]";
        }
        item.label->setText(txt, juce::dontSendNotification);
        item.label->setTooltip(entry.filePath);
        recentContainer_.addAndMakeVisible(*item.label);

        item.openBtn = std::make_unique<juce::TextButton>("Open");
        item.openBtn->setEnabled(entry.existsOnDisk);
        item.openBtn->onClick = [this, path = entry.filePath] {
            std::string err;
            context_.openProject(path, err);
        };
        recentContainer_.addAndMakeVisible(*item.openBtn);

        item.removeBtn = std::make_unique<juce::TextButton>("✕");
        item.removeBtn->setTooltip("Remove from recent list (does not delete file)");
        item.removeBtn->onClick = [this, path = entry.filePath] {
            context_.getRecentProjects().remove(path);
            context_.saveRecentProjects();
            refreshRecentProjects();
        };
        recentContainer_.addAndMakeVisible(*item.removeBtn);

        recentRows_.push_back(std::move(item));
        y += rowHeight + 6;
    }

    resized();
}

void StartStudioView::paint(juce::Graphics& g) {
    auto& theme = theme::SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    g.fillAll(c.background);

    // Subtle dark gradient background glow
    juce::ColourGradient grad(c.surfaceElevated.withAlpha(0.2f), static_cast<float>(getWidth() / 2), 60.0f,
                              c.background, static_cast<float>(getWidth() / 2), 260.0f, true);
    g.setGradientFill(grad);
    g.fillEllipse(static_cast<float>(getWidth() / 2 - 250), -50.0f, 500.0f, 300.0f);

    // Brand and Welcome Heading
    g.setFont(theme.getDisplayFont(28.0f));
    g.setColour(c.textPrimary);
    g.drawText("ARCLINE", 48, 46, 200, 34, juce::Justification::centredLeft);

    g.setFont(theme.getHeadingFont(14.0f));
    g.setColour(c.selectionBlue);
    g.drawText("Shape Your Sound.", 185, 54, 250, 24, juce::Justification::centredLeft);

    g.setFont(theme.getSubheadingFont(14.0f));
    g.setColour(c.textSecondary);
    g.drawText("Professional Native Digital Audio Workstation", 48, 84, 400, 22, juce::Justification::centredLeft);

    g.setFont(theme.getBodyFont(12.5f));
    g.setColour(c.textMuted);
    g.drawText("Make music, not menus. Engineered for fast arranging, precision mixing, and native desktop stability.",
               48, 108, 640, 20, juce::Justification::centredLeft);

    // Divider line between hero and body
    g.setColour(c.borderSubtle);
    g.fillRect(48, 142, getWidth() - 96, 1);

    // Footer info
    g.setColour(c.borderSubtle);
    g.fillRect(48, getHeight() - 36, getWidth() - 96, 1);

    g.setFont(theme.getCaptionFont(11.0f));
    g.setColour(c.textMuted);
    g.drawText("ARCLINE v0.1.0 • Shape Your Sound. • Native Desktop Architecture • Logic Pro Dark Theme • C++20 / JUCE 8",
               48, getHeight() - 28, getWidth() - 96, 20, juce::Justification::centredLeft);
}

void StartStudioView::resized() {
    int startY = 160;

    // Actions row
    newProjectBtn_.setBounds(48, startY, 140, 36);
    openProjectBtn_.setBounds(196, startY, 140, 36);
    loadDemoSessionBtn_.setBounds(344, startY, 260, 36);

    int contentY = startY + 54;
    int colWidth = (getWidth() - 96 - 32) / 2;
    if (colWidth < 280) colWidth = 280;

    // Left Column: Recent Projects
    recentHeaderLabel_.setBounds(48, contentY, colWidth, 24);
    emptyRecentMessageLabel_.setBounds(48, contentY + 32, colWidth, 32);

    int vpHeight = getHeight() - contentY - 80;
    if (vpHeight < 100) vpHeight = 100;
    recentViewport_.setBounds(48, contentY + 32, colWidth, vpHeight);

    int itemY = 0;
    int containerW = recentViewport_.getWidth() > 0 ? recentViewport_.getWidth() - 12 : colWidth - 12;
    for (auto& row : recentRows_) {
        int rHeight = 36;
        if (row.label && row.openBtn && row.removeBtn) {
            row.label->setBounds(0, itemY, containerW - 100, rHeight);
            row.openBtn->setBounds(containerW - 90, itemY + 4, 52, 28);
            row.removeBtn->setBounds(containerW - 32, itemY + 4, 28, 28);
        }
        itemY += rHeight + 6;
    }
    recentContainer_.setSize(containerW, itemY);

    // Right Column: Templates & Quick Start
    int rightColX = 48 + colWidth + 32;
    templatesHeaderLabel_.setBounds(rightColX, contentY, colWidth, 24);

    int tmplY = contentY + 32;
    recordingTemplateBtn_.setBounds(rightColX, tmplY, colWidth - 20, 36);
    tmplY += 44;
    electronicTemplateBtn_.setBounds(rightColX, tmplY, colWidth - 20, 36);
    tmplY += 44;
    songwriterTemplateBtn_.setBounds(rightColX, tmplY, colWidth - 20, 36);
    tmplY += 44;
    emptyTemplateBtn_.setBounds(rightColX, tmplY, colWidth - 20, 36);
}

} // namespace sonora::ui::workspaces
