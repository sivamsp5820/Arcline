#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace arcline::future_audio {

struct MidiDeviceInfo {
    std::string deviceId;
    std::string name;
    bool isInput = true;
    bool isEnabled = false;
};

struct RawMidiEvent {
    uint8_t statusByte = 0;
    uint8_t dataByte1 = 0;
    uint8_t dataByte2 = 0;
    int sampleOffset = 0;    // Sample offset within the current audio buffer block (0..blockSize-1)
    int64_t timestampTick = 0;
};

/**
 * @brief Extension point interface for MIDI hardware I/O and event scheduling (Phase 4).
 * 
 * Boundaries and Contracts:
 * - Hardware MIDI input driver events are placed into a lock-free ring buffer immediately upon receipt.
 * - Audio callback dequeues MIDI events and aligns them to sample-exact offsets within the render block.
 * - Virtual instrument plugins receive sample-accurate MIDI buffers (VST3 processData.inputEvents).
 * - Zero dynamic heap allocation in the MIDI input processing pathway.
 */
class IMidiSubsystem {
public:
    virtual ~IMidiSubsystem() = default;

    virtual std::vector<MidiDeviceInfo> getAvailableMidiInputs() = 0;
    virtual std::vector<MidiDeviceInfo> getAvailableMidiOutputs() = 0;

    virtual bool enableMidiInput(const std::string& deviceId, bool enable) = 0;
    virtual bool enableMidiOutput(const std::string& deviceId, bool enable) = 0;

    virtual void sendMidiEvent(const std::string& outputDeviceId, const RawMidiEvent& event) = 0;
};

} // namespace arcline::future_audio

namespace sonora::future_audio {
    using MidiDeviceInfo = arcline::future_audio::MidiDeviceInfo;
    using RawMidiEvent = arcline::future_audio::RawMidiEvent;
    using IMidiSubsystem = arcline::future_audio::IMidiSubsystem;
}
