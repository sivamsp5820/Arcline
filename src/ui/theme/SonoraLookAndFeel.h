#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora::ui::theme {

class SonoraLookAndFeel : public juce::LookAndFeel_V4 {
public:
    SonoraLookAndFeel();
    ~SonoraLookAndFeel() override = default;

    void drawButtonBackground(juce::Graphics& g,
                            juce::Button& button,
                            const juce::Colour& backgroundColour,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override;

    void drawButtonText(juce::Graphics& g,
                       juce::TextButton& button,
                       bool shouldDrawButtonAsHighlighted,
                       bool shouldDrawButtonAsDown) override;

    void drawLinearSlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos,
                          float minSliderPos,
                          float maxSliderPos,
                          juce::Slider::SliderStyle style,
                          juce::Slider& slider) override;

    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPosProportional,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider& slider) override;

    void drawScrollbar(juce::Graphics& g,
                      juce::ScrollBar& scrollbar,
                      int x, int y, int width, int height,
                      bool isScrollbarVertical,
                      int thumbStartPosition,
                      int thumbSize,
                      bool isMouseOver,
                      bool isMouseDown) override;

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;

    void drawPopupMenuItem(juce::Graphics& g,
                          const juce::Rectangle<int>& area,
                          bool isSeparator,
                          bool isActive,
                          bool isHighlighted,
                          bool isTicked,
                          bool hasSubMenu,
                          const juce::String& text,
                          const juce::String& shortcutKeyText,
                          const juce::Drawable* icon,
                          const juce::Colour* textColour) override;

    void drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override;

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    juce::Font getPopupMenuFont() override;
};

} // namespace sonora::ui::theme
