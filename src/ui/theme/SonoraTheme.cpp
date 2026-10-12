#include "ui/theme/SonoraTheme.h"

namespace sonora::ui::theme {

SonoraTheme& SonoraTheme::getInstance() noexcept {
    static SonoraTheme instance;
    return instance;
}

SonoraTheme::SonoraTheme() {
    setDarkMode(true);
}

void SonoraTheme::setDarkMode(bool dark) noexcept {
    isDark_ = dark;
    if (dark) {
        colors_ = ColorPalette{};
    } else {
        // Light mode fallbacks if ever requested
        colors_.background         = juce::Colour(0xfff5f6f8);
        colors_.timelineBackground = juce::Colour(0xffeaecef);
        colors_.panelBackground    = juce::Colour(0xffe2e5e9);
        colors_.surface            = juce::Colour(0xffffffff);
        colors_.surfaceElevated    = juce::Colour(0xfff0f2f5);
        colors_.surfaceHover       = juce::Colour(0xffe6e9ee);
        colors_.surfaceActive      = juce::Colour(0xffd8dce2);
        colors_.border             = juce::Colour(0xffc8ccd2);
        colors_.borderSubtle       = juce::Colour(0xffd5d8de);
        colors_.borderFocus        = juce::Colour(0xff2d72d2);
        colors_.textPrimary        = juce::Colour(0xff1a1c20);
        colors_.textSecondary      = juce::Colour(0xff5a626f);
        colors_.textMuted          = juce::Colour(0xff8a93a0);
    }
}

juce::Font SonoraTheme::getDisplayFont(float size) const {
    return juce::Font(juce::FontOptions{}.withHeight(size).withStyle("Bold"));
}

juce::Font SonoraTheme::getHeadingFont(float size) const {
    return juce::Font(juce::FontOptions{}.withHeight(size).withStyle("Bold"));
}

juce::Font SonoraTheme::getSubheadingFont(float size) const {
    return juce::Font(juce::FontOptions{}.withHeight(size).withStyle("Bold"));
}

juce::Font SonoraTheme::getBodyFont(float size) const {
    return juce::Font(juce::FontOptions{}.withHeight(size));
}

juce::Font SonoraTheme::getLabelFont(float size) const {
    return juce::Font(juce::FontOptions{}.withHeight(size).withStyle("Bold"));
}

juce::Font SonoraTheme::getCaptionFont(float size) const {
    return juce::Font(juce::FontOptions{}.withHeight(size));
}

juce::Font SonoraTheme::getNumericFont(float size) const {
    return juce::Font(juce::FontOptions{}.withName(juce::Font::getDefaultMonospacedFontName()).withHeight(size).withStyle("Bold"));
}

juce::Font SonoraTheme::getLcdFont(float size) const {
    return juce::Font(juce::FontOptions{}.withName(juce::Font::getDefaultMonospacedFontName()).withHeight(size).withStyle("Bold"));
}

} // namespace sonora::ui::theme
