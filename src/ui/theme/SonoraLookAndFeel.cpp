#include "ui/theme/SonoraLookAndFeel.h"
#include "ui/theme/SonoraTheme.h"

namespace sonora::ui::theme {

SonoraLookAndFeel::SonoraLookAndFeel() {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    setColour(juce::ResizableWindow::backgroundColourId, c.background);
    setColour(juce::TextButton::buttonColourId, c.surfaceElevated);
    setColour(juce::TextButton::buttonOnColourId, c.selectionBlue);
    setColour(juce::TextButton::textColourOffId, c.textPrimary);
    setColour(juce::TextButton::textColourOnId, juce::Colours::white);

    setColour(juce::TextEditor::backgroundColourId, c.surface);
    setColour(juce::TextEditor::textColourId, c.textPrimary);
    setColour(juce::TextEditor::highlightColourId, c.surfaceActive);
    setColour(juce::TextEditor::highlightedTextColourId, c.textPrimary);
    setColour(juce::TextEditor::outlineColourId, c.border);
    setColour(juce::TextEditor::focusedOutlineColourId, c.borderFocus);

    setColour(juce::ScrollBar::backgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::ScrollBar::thumbColourId, c.border);

    setColour(juce::PopupMenu::backgroundColourId, c.panelBackground);
    setColour(juce::PopupMenu::textColourId, c.textPrimary);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, c.surfaceHover);
    setColour(juce::PopupMenu::highlightedTextColourId, c.textPrimary);
}

void SonoraLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                            juce::Button& button,
                                            const juce::Colour& backgroundColour,
                                            bool shouldDrawButtonAsHighlighted,
                                            bool shouldDrawButtonAsDown) {
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    juce::Colour baseColor = backgroundColour;
    if (button.getToggleState()) {
        baseColor = c.selectionBlue;
    } else if (shouldDrawButtonAsDown) {
        baseColor = c.surfaceActive;
    } else if (shouldDrawButtonAsHighlighted) {
        baseColor = c.surfaceHover;
    }

    // Logic Pro subtle top-to-bottom bevel gradient
    juce::ColourGradient grad(baseColor.brighter(0.06f), bounds.getX(), bounds.getY(),
                              baseColor.darker(0.08f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bounds, SonoraTheme::kRadiusSmall);

    // Subtle 1px border
    juce::Colour borderCol = button.getToggleState() ? c.selectionBlue.brighter(0.2f) : c.border;
    if (button.hasKeyboardFocus(true)) {
        borderCol = c.borderFocus;
    }
    g.setColour(borderCol);
    g.drawRoundedRectangle(bounds, SonoraTheme::kRadiusSmall, 1.0f);
}

void SonoraLookAndFeel::drawButtonText(juce::Graphics& g,
                                      juce::TextButton& button,
                                      bool /*shouldDrawButtonAsHighlighted*/,
                                      bool /*shouldDrawButtonAsDown*/) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    juce::Colour textColour = button.getToggleState() ? juce::Colours::white : c.textPrimary;
    if (!button.isEnabled()) {
        textColour = c.textMuted;
    }

    g.setFont(getTextButtonFont(button, button.getHeight()));
    g.setColour(textColour);
    g.drawFittedText(button.getButtonText(), button.getLocalBounds(),
                     juce::Justification::centred, 1);
}

void SonoraLookAndFeel::drawLinearSlider(juce::Graphics& g,
                                        int x, int y, int width, int height,
                                        float sliderPos,
                                        float /*minSliderPos*/,
                                        float /*maxSliderPos*/,
                                        juce::Slider::SliderStyle style,
                                        juce::Slider& /*slider*/) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    if (style == juce::Slider::LinearVertical) {
        // Vertical Studio Channel Fader
        float trackX = static_cast<float>(x + width / 2);
        // Fader well slot
        g.setColour(juce::Colour(0xff121417));
        g.fillRoundedRectangle(trackX - 2.0f, static_cast<float>(y), 4.0f, static_cast<float>(height), 2.0f);
        g.setColour(c.borderSubtle);
        g.drawRoundedRectangle(trackX - 2.0f, static_cast<float>(y), 4.0f, static_cast<float>(height), 2.0f, 1.0f);

        // Fader thumb handle (Logic metallic cap with dark grip line)
        float thumbW = static_cast<float>(width) - 4.0f;
        if (thumbW > 34.0f) thumbW = 34.0f;
        float thumbH = 18.0f;
        float thumbY = sliderPos - thumbH * 0.5f;
        float thumbLeft = trackX - thumbW * 0.5f;

        juce::Rectangle<float> thumbBounds(thumbLeft, thumbY, thumbW, thumbH);
        juce::ColourGradient capGrad(juce::Colour(0xff555a64), thumbLeft, thumbY,
                                     juce::Colour(0xff2d3137), thumbLeft, thumbY + thumbH, false);
        g.setGradientFill(capGrad);
        g.fillRoundedRectangle(thumbBounds, 3.0f);

        // Center white indicator stripe on fader cap
        g.setColour(juce::Colour(0xffe4e7eb));
        g.fillRect(thumbLeft + 4.0f, sliderPos - 1.0f, thumbW - 8.0f, 2.0f);

        g.setColour(juce::Colour(0xff1a1c20));
        g.drawRoundedRectangle(thumbBounds, 3.0f, 1.0f);
    } else {
        // Horizontal mini slider
        float trackY = static_cast<float>(y + height / 2);
        g.setColour(juce::Colour(0xff14161a));
        g.fillRoundedRectangle(static_cast<float>(x), trackY - 2.0f, static_cast<float>(width), 4.0f, 2.0f);

        // Thumb
        float thumbSize = 12.0f;
        juce::Rectangle<float> thumb(sliderPos - thumbSize * 0.5f, trackY - thumbSize * 0.5f, thumbSize, thumbSize);
        g.setColour(c.surfaceElevated);
        g.fillEllipse(thumb);
        g.setColour(c.border);
        g.drawEllipse(thumb, 1.0f);
    }
}

void SonoraLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                        int x, int y, int width, int height,
                                        float sliderPosProportional,
                                        float rotaryStartAngle,
                                        float rotaryEndAngle,
                                        juce::Slider& /*slider*/) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    float radius = static_cast<float>(std::min(width, height)) * 0.45f;
    float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
    float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;

    // Outer ring track
    juce::Path trackPath;
    trackPath.addCentredArc(centreX, centreY, radius + 2.0f, radius + 2.0f,
                           0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(c.borderSubtle);
    g.strokePath(trackPath, juce::PathStrokeType(2.5f));

    // Active arc
    float currentAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    float midAngle = (rotaryStartAngle + rotaryEndAngle) * 0.5f;
    juce::Path activePath;
    if (currentAngle >= midAngle) {
        activePath.addCentredArc(centreX, centreY, radius + 2.0f, radius + 2.0f,
                                 0.0f, midAngle, currentAngle, true);
    } else {
        activePath.addCentredArc(centreX, centreY, radius + 2.0f, radius + 2.0f,
                                 0.0f, currentAngle, midAngle, true);
    }
    g.setColour(c.selectionBlue);
    g.strokePath(activePath, juce::PathStrokeType(2.5f));

    // Knob body (Logic dark metallic)
    juce::ColourGradient knobGrad(juce::Colour(0xff3f444e), centreX - radius, centreY - radius,
                                  juce::Colour(0xff22252a), centreX + radius, centreY + radius, true);
    g.setGradientFill(knobGrad);
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

    g.setColour(c.border);
    g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 1.0f);

    // Indicator needle
    juce::Path needle;
    needle.startNewSubPath(centreX, centreY);
    needle.lineTo(centreX + std::sin(currentAngle) * (radius - 2.0f),
                  centreY - std::cos(currentAngle) * (radius - 2.0f));
    g.setColour(juce::Colours::white);
    g.strokePath(needle, juce::PathStrokeType(2.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
}

void SonoraLookAndFeel::drawScrollbar(juce::Graphics& g,
                                     juce::ScrollBar& /*scrollbar*/,
                                     int x, int y, int width, int height,
                                     bool isScrollbarVertical,
                                     int thumbStartPosition,
                                     int thumbSize,
                                     bool isMouseOver,
                                     bool /*isMouseDown*/) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    juce::Rectangle<float> thumbBounds;
    if (isScrollbarVertical) {
        thumbBounds = juce::Rectangle<float>(static_cast<float>(x) + 2.0f,
                                             static_cast<float>(thumbStartPosition),
                                             static_cast<float>(width) - 4.0f,
                                             static_cast<float>(thumbSize));
    } else {
        thumbBounds = juce::Rectangle<float>(static_cast<float>(thumbStartPosition),
                                             static_cast<float>(y) + 2.0f,
                                             static_cast<float>(thumbSize),
                                             static_cast<float>(height) - 4.0f);
    }

    juce::Colour thumbColour = isMouseOver ? c.textMuted : c.border;
    g.setColour(thumbColour);
    g.fillRoundedRectangle(thumbBounds, 2.5f);
}

void SonoraLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    juce::Rectangle<float> area(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    g.setColour(c.panelBackground);
    g.fillRoundedRectangle(area, SonoraTheme::kRadiusNormal);

    g.setColour(c.border);
    g.drawRoundedRectangle(area.reduced(0.5f), SonoraTheme::kRadiusNormal, 1.0f);
}

void SonoraLookAndFeel::drawPopupMenuItem(juce::Graphics& g,
                                         const juce::Rectangle<int>& area,
                                         bool isSeparator,
                                         bool isActive,
                                         bool isHighlighted,
                                         bool /*isTicked*/,
                                         bool /*hasSubMenu*/,
                                         const juce::String& text,
                                         const juce::String& shortcutKeyText,
                                         const juce::Drawable* /*icon*/,
                                         const juce::Colour* textColour) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    if (isSeparator) {
        auto r = area.reduced(5, 0);
        g.setColour(c.borderSubtle);
        g.fillRect(r.removeFromTop(1));
        return;
    }

    auto r = area.toFloat().reduced(2.0f, 1.0f);
    if (isHighlighted && isActive) {
        g.setColour(c.selectionBlue);
        g.fillRoundedRectangle(r, SonoraTheme::kRadiusSmall);
    }

    juce::Colour textCol = textColour ? *textColour : (isActive ? c.textPrimary : c.textMuted);
    if (isHighlighted && isActive) {
        textCol = juce::Colours::white;
    }
    g.setColour(textCol);
    g.setFont(getPopupMenuFont());

    auto textRect = area.reduced(10, 0);
    g.drawFittedText(text, textRect, juce::Justification::centredLeft, 1);

    if (shortcutKeyText.isNotEmpty()) {
        g.setColour(isHighlighted ? juce::Colour(0xffd0e0ff) : c.textSecondary);
        g.drawFittedText(shortcutKeyText, textRect, juce::Justification::centredRight, 1);
    }
}

void SonoraLookAndFeel::drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) {
    auto& theme = SonoraTheme::getInstance();
    const auto& c = theme.getColors();

    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(0.5f);
    if (textEditor.hasKeyboardFocus(true)) {
        g.setColour(c.borderFocus);
        g.drawRoundedRectangle(bounds, SonoraTheme::kRadiusSmall, 1.5f);
    } else {
        g.setColour(c.border);
        g.drawRoundedRectangle(bounds, SonoraTheme::kRadiusSmall, 1.0f);
    }
}

juce::Font SonoraLookAndFeel::getTextButtonFont(juce::TextButton&, int /*buttonHeight*/) {
    return SonoraTheme::getInstance().getLabelFont(11.5f);
}

juce::Font SonoraLookAndFeel::getPopupMenuFont() {
    return SonoraTheme::getInstance().getBodyFont(12.5f);
}

} // namespace sonora::ui::theme
