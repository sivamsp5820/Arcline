#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora::ui::theme {

struct ColorPalette {
    // Logic Pro X Dark Charcoal & Graphite surfaces
    juce::Colour background{0xff191b1f};         // Main DAW window backdrop
    juce::Colour timelineBackground{0xff1f2126}; // Multitrack lane grid background
    juce::Colour panelBackground{0xff24262b};   // Toolbars, inspectors, headers
    juce::Colour surface{0xff2a2d33};           // Elevated containers & track headers
    juce::Colour surfaceElevated{0xff33373f};   // Buttons, controls, cards
    juce::Colour surfaceHover{0xff3e434d};      // Hover state
    juce::Colour surfaceActive{0xff484e5a};     // Click/active state

    // Dividers & Borders
    juce::Colour border{0xff383b42};            // Standard panel separation
    juce::Colour borderSubtle{0xff2b2d33};      // Faint lane divider
    juce::Colour borderFocus{0xff2d72d2};       // Restrained studio blue focus/selection

    // Text & Labels
    juce::Colour textPrimary{0xffe4e7eb};       // High-contrast primary text
    juce::Colour textSecondary{0xff9ba1ab};     // Muted control captions
    juce::Colour textMuted{0xff6c727d};         // Inactive text

    // Center Logic LCD Display Screen
    juce::Colour lcdBackground{0xff101113};      // Recessed dark glass
    juce::Colour lcdBorder{0xff222428};          // Recessed bezel border
    juce::Colour lcdText{0xffe4e7eb};            // LCD primary digital values
    juce::Colour lcdSubtext{0xff6b7280};         // LCD caption units (BPM, BEAT, TICK)
    juce::Colour lcdAccent{0xff38bdf8};          // Soft blue indicator

    // Logic Pro X Conventional Workflow Accents (Restrained)
    juce::Colour playGreen{0xff27ae60};          // Logic transport play
    juce::Colour recordRed{0xffd32f2f};          // Logic transport record arm
    juce::Colour soloYellow{0xfff39c12};         // Logic solo amber
    juce::Colour muteBlue{0xff2980b9};           // Logic mute blue
    juce::Colour cycleYellow{0xffd4ac0d};        // Logic timeline cycle locator
    juce::Colour selectionBlue{0xff2d72d2};      // Selected clip / track outline

    // Professional Track Color Palette (Logic Muted Studio)
    juce::Colour trackAudioBlue{0xff2e86ab};     // Audio track/region
    juce::Colour trackMidiGreen{0xff3a7d44};     // Software instrument
    juce::Colour trackBassPurple{0xff705898};    // Bass / Synth
    juce::Colour trackDrumCoral{0xffb85d3b};     // Drums / Percussion
    juce::Colour trackVocalGold{0xffb0892a};     // Vocals / Acoustic
    juce::Colour trackAuxTeal{0xff17a2b8};       // Aux / FX Bus

    // Compatibility fields mapped to restrained Logic Pro tones
    juce::Colour accentBlue{0xff2d72d2};         // Primary studio action / selection
    juce::Colour accentLime{0xff27ae60};         // Muted studio green (no neon lime)
    juce::Colour error{0xffd32f2f};              // Alert / error red
    juce::Colour warning{0xfff39c12};            // Warning amber
};

class SonoraTheme {
public:
    static SonoraTheme& getInstance() noexcept;

    const ColorPalette& getColors() const noexcept { return colors_; }
    void setDarkMode(bool dark) noexcept;

    // Typography
    juce::Font getDisplayFont(float size = 24.0f) const;
    juce::Font getHeadingFont(float size = 16.0f) const;
    juce::Font getSubheadingFont(float size = 13.0f) const;
    juce::Font getBodyFont(float size = 12.0f) const;
    juce::Font getLabelFont(float size = 11.0f) const;
    juce::Font getCaptionFont(float size = 10.0f) const;
    juce::Font getNumericFont(float size = 13.0f) const;
    juce::Font getLcdFont(float size = 16.0f) const;

    // Spacing
    static constexpr int kSpace4  = 4;
    static constexpr int kSpace8  = 8;
    static constexpr int kSpace12 = 12;
    static constexpr int kSpace16 = 16;
    static constexpr int kSpace24 = 24;
    static constexpr int kSpace32 = 32;

    // Geometry
    static constexpr float kRadiusSmall  = 3.0f;
    static constexpr float kRadiusNormal = 5.0f;
    static constexpr float kRadiusLarge  = 8.0f;

private:
    SonoraTheme();
    ColorPalette colors_;
    bool isDark_{true};
};

} // namespace sonora::ui::theme

namespace sonora::ui {
    using theme::SonoraTheme;
    using theme::ColorPalette;
}
