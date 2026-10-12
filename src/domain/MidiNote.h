#pragma once

#include <cstdint>

namespace sonora::domain {

struct MidiNote {
    int pitch{60};              // MIDI note number 0-127 (60 = C3 / Middle C)
    int64_t startTick{0};       // Note start position in ticks relative to region start
    int64_t lengthTicks{960};   // Duration in ticks (960 = quarter note at 960 PPQ)
    int velocity{100};          // MIDI velocity 1-127
    bool isSelected{false};

    bool operator==(const MidiNote& other) const noexcept {
        return pitch == other.pitch &&
               startTick == other.startTick &&
               lengthTicks == other.lengthTicks &&
               velocity == other.velocity;
    }
};

} // namespace sonora::domain
