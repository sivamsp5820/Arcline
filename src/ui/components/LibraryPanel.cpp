#include "ui/components/LibraryPanel.h"
#include "ui/theme/SonoraTheme.h"

namespace sonora::ui::components {

int LibraryPanel::ItemListModel::getNumRows() {
    return static_cast<int>(items.size());
}

void LibraryPanel::ItemListModel::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) {
    if (rowNumber < 0 || rowNumber >= static_cast<int>(items.size())) return;

    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();
    const auto& item = items[rowNumber];

    if (rowIsSelected) {
        g.setColour(c.selectionBlue);
        g.fillRect(0, 0, width, height);
        g.setColour(juce::Colours::white);
    } else {
        g.setColour(rowNumber % 2 == 0 ? c.surface : c.surfaceElevated);
        g.fillRect(0, 0, width, height);
        g.setColour(c.textPrimary);
    }

    g.setFont(theme.getLabelFont(11.0f));
    g.drawText(item.name, 8, 2, width - 80, height - 4, juce::Justification::centredLeft, true);

    g.setFont(theme.getCaptionFont(9.5f));
    g.setColour(rowIsSelected ? juce::Colour(0xffd0e0ff) : c.textSecondary);
    g.drawText(item.category, width - 74, 2, 68, height - 4, juce::Justification::centredRight, true);
}

void LibraryPanel::ItemListModel::listBoxItemClicked(int /*row*/, const juce::MouseEvent& /*e*/) {}

void LibraryPanel::ItemListModel::listBoxItemDoubleClicked(int row, const juce::MouseEvent& /*e*/) {
    if (row >= 0 && row < static_cast<int>(items.size())) {
        if (onItemDoubleClicked) onItemDoubleClicked(items[row]);
    }
}

// ============================================================================
// LibraryPanel Implementation
// ============================================================================

LibraryPanel::LibraryPanel(app::AppContext& context)
    : context_(context) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    headerLabel_.setFont(theme.getHeadingFont(12.0f));
    headerLabel_.setColour(juce::Label::textColourId, c.textSecondary);
    addAndMakeVisible(headerLabel_);

    closeButton_.setTooltip("Close Library Panel");
    closeButton_.onClick = [this] { context_.toggleLibrary(); };
    addAndMakeVisible(closeButton_);

    searchFilter_.setTextToShowWhenEmpty("Search Library / Loops...", c.textMuted);
    searchFilter_.onTextChange = [this] {
        populateItems();
    };
    addAndMakeVisible(searchFilter_);

    listBox_.setModel(&listModel_);
    listBox_.setRowHeight(24);
    addAndMakeVisible(listBox_);

    listModel_.onItemDoubleClicked = [this](const ItemListModel::Item& item) {
        if (auto* p = context_.getActiveProject()) {
            if (item.type == "inst") {
                auto track = p->addTrack(domain::TrackType::SoftwareInstrument, item.name);
                track->setColor(0xff3a7d44);
            } else if (item.type == "audio") {
                auto track = p->addTrack(domain::TrackType::Audio, item.name);
                track->setColor(0xff2e86ab);
                auto reg = std::make_shared<domain::Region>(
                    "reg_" + juce::Uuid().toString().toStdString().substr(0, 8),
                    track->getId(), item.name, domain::RegionType::Audio, 0, domain::Project::kTicksPerBeat * 16);
                reg->generateDemoWaveform(120, 3.5f);
                track->addRegion(reg);
            }
            context_.notifyProjectChanged();
        }
    };

    populateItems();
}

void LibraryPanel::populateItems() {
    std::vector<ItemListModel::Item> all = {
        { "Keyboards", "Steinway Grand Piano", "inst" },
        { "Keyboards", "Vintage Electric Piano", "inst" },
        { "Keyboards", "Classic B3 Tonewheel Organ", "inst" },
        { "Bass", "Liverpool Finger Bass", "inst" },
        { "Bass", "Analog Sub 37 Bass", "inst" },
        { "Synthesizer", "Retro 80s Brass Synth", "inst" },
        { "Synthesizer", "Modern Pluck Lead", "inst" },
        { "Drums", "Studio Drum Kit (Birch)", "inst" },
        { "Drums", "808 Hip-Hop Drum Machine", "inst" },
        { "Loops", "Deep House Groove 124", "audio" },
        { "Loops", "Acoustic Strum 95 BPM", "audio" },
        { "Loops", "Soulful Vocal Phrase C", "audio" },
        { "Loops", "Vintage Vinyl Breakbeat", "audio" },
        { "FX", "White Noise Sweep Up", "audio" },
        { "FX", "Analog Sub Drop", "audio" }
    };

    std::string query = searchFilter_.getText().toLowerCase().toStdString();
    listModel_.items.clear();

    for (const auto& item : all) {
        if (query.empty() ||
            juce::String(item.name).toLowerCase().contains(query) ||
            juce::String(item.category).toLowerCase().contains(query)) {
            listModel_.items.push_back(item);
        }
    }

    listBox_.updateContent();
    listBox_.repaint();
}

void LibraryPanel::paint(juce::Graphics& g) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    g.fillAll(c.panelBackground);

    // Left vertical border dividing multitrack timeline from library
    g.setColour(c.border);
    g.drawVerticalLine(0, 0.0f, static_cast<float>(getHeight()));
}

void LibraryPanel::resized() {
    auto bounds = getLocalBounds().reduced(6, 6);

    auto topRow = bounds.removeFromTop(24);
    headerLabel_.setBounds(topRow.removeFromLeft(120));
    closeButton_.setBounds(topRow.removeFromRight(24));
    bounds.removeFromTop(4);

    searchFilter_.setBounds(bounds.removeFromTop(24));
    bounds.removeFromTop(6);

    listBox_.setBounds(bounds);
}

} // namespace sonora::ui::components
