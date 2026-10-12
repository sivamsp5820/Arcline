#pragma once

#include "ui/workspaces/WorkspaceView.h"
#include "app/AppContext.h"
#include <vector>

namespace sonora::ui::workspaces {

class StartStudioView : public WorkspaceView,
                        public app::AppContext::ProjectListener {
public:
    explicit StartStudioView(app::AppContext& context);
    ~StartStudioView() override;

    domain::WorkspaceType getWorkspaceType() const noexcept override {
        return domain::WorkspaceType::StartStudio;
    }

    void onActivated() override;
    void paint(juce::Graphics& g) override;
    void resized() override;

    void onProjectChanged(const domain::Project* project) override;

private:
    struct RecentRowItem {
        std::string name;
        std::string path;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::TextButton> openBtn;
        std::unique_ptr<juce::TextButton> removeBtn;
    };

    void refreshRecentProjects();
    void createProjectFromTemplate(const std::string& templateName, const std::string& defaultName);

    app::AppContext& context_;

    juce::TextButton newProjectBtn_{"＋ New Project..."};
    juce::TextButton openProjectBtn_{"📂 Open Project..."};
    juce::TextButton loadDemoSessionBtn_{"🎹 Open Demo Multitrack Session"};

    // Studio Templates
    juce::TextButton recordingTemplateBtn_{"Audio Recording (Vocals / EP / Bass / Drums)"};
    juce::TextButton electronicTemplateBtn_{"Electronic / Beat Production (124 BPM)"};
    juce::TextButton songwriterTemplateBtn_{"Songwriter (Piano / Acoustic / Vocals)"};
    juce::TextButton emptyTemplateBtn_{"Empty Session"};

    juce::Label recentHeaderLabel_;
    juce::Label templatesHeaderLabel_;
    juce::Label emptyRecentMessageLabel_;

    std::vector<RecentRowItem> recentRows_;
    juce::Component recentContainer_;
    juce::Viewport recentViewport_;
};

} // namespace sonora::ui::workspaces
