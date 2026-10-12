# ARCLINE — Architectural Development Roadmap

> **Product:** ARCLINE Desktop Digital Audio Workstation  
> **Tagline:** "Shape Your Sound."  
> **Technology Stack:** C++20, JUCE 8, CMake, Ninja  

---

## Strict Phase Boundaries

In accordance with ARCLINE development principles:
- Feature completion must be demonstrated by real implementation, native launches, and automated tests.
- UI controls are never coupled to fake engines or fabricated threads. Unimplemented features remain cleanly labeled or disabled until their designated phase is implemented.

---

## Development Phases

| Phase | Milestone Title | Scope Summary | Acceptance Gate | Status |
|---|---|---|---|---|
| **Phase 1** | **Native Shell, Dark Theme & Project Durability** | Native desktop window, Logic Pro X dark graphite interface, multitrack arrangement workspace, track headers, ruler, audio waveform peaks, MIDI notes preview, dual-channel inspector, dockable lower panels, sound library, Start Studio chooser, `.arcline` Schema v2 persistence with transparent legacy `.sonora` migration, centralized command registry, cross-platform packaging foundation, 7 automated test suites | GUI launches natively, all panels render without mock images, 100% automated tests pass, zero fake audio processing | **COMPLETED & VERIFIED** |
| **Phase 2** | **Audio Device Manager & Real-Time Engine** | OS audio driver backends (WASAPI/ASIO on Windows, CoreAudio on macOS, ALSA/PipeWire/JACK on Linux), sample-accurate transport clock, hard real-time safe audio rendering callback | Audio device enumeration, buffer configuration, zero-allocation audio callback verified | *Next Phase* |
| **Phase 3** | **Multitrack Audio Recording & Disk Streaming** | Background Disk I/O worker, double-buffered circular memory pools, live WAV recording, punch-in/out, sample-accurate region trimming and fading | Latency-free multitrack capture, dropout-free recording | *Scheduled* |
| **Phase 4** | **MIDI Subsystem & Piano Roll Engine** | Hardware MIDI device enumeration, sample-accurate MIDI event scheduling, interactive piano roll note editing, velocity automation | Live MIDI keyboard recording, note quantization | *Scheduled* |
| **Phase 5** | **Mixer Summing & Routing Engine** | Topological audio graph, aux bus sends, summing engine, de-zippered faders, solo-in-place (SIP), lock-free metering telemetry | Multitrack summing with clean parameter interpolation | *Scheduled* |
| **Phase 6** | **VST3 Plugin Hosting** | Out-of-process/sandboxed plugin scanner, VST3 instrument & effect hosting, native GUI window wrappers, state serialization into `.arcline` chunks | Third-party VST3 instrument and effect execution | *Scheduled* |
| **Phase 7** | **Automation, Offline Export & Distribution** | Sample-accurate automation curves, multi-threaded stem bounce (WAV/FLAC), true-peak limiting, final cross-platform distribution signing | Bit-exact offline export, signed installers across all platforms | *Scheduled* |

---

*ARCLINE — Shape Your Sound.*
