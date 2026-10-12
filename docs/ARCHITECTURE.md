# ARCLINE — Architectural Specification & Guidelines

> **Product:** ARCLINE Desktop Digital Audio Workstation  
> **Tagline:** "Shape Your Sound."  
> **Technology Stack:** C++20, JUCE 8, CMake, Ninja  

---

## 1. High-Level Architecture Overview

ARCLINE enforces a clean separation of concerns across four distinct architectural layers:

```
┌────────────────────────────────────────────────────────┐
│                      UI Layer                          │
│  MainLayoutComponent, TransportBar, ArrangementView,   │
│  InspectorPanel, LowerEditorPanel, SonoraLookAndFeel   │
└───────────────────────────┬────────────────────────────┘
                            │ Dispatches Commands / Observes State
┌───────────────────────────▼────────────────────────────┐
│                    Application Layer                   │
│   AppContext, NavigationController, CommandRegistry    │
└───────────────────────────┬────────────────────────────┘
                            │ Orchestrates
┌───────────────────────────▼────────────────────────────┐
│                      Domain Layer                      │
│       Project, Track, Region, MidiNote, Preferences    │
└───────────────────────────┬────────────────────────────┘
                            │ Persists via
┌───────────────────────────▼────────────────────────────┐
│                 Infrastructure Layer                   │
│   ProjectRepository (Atomic JSON), PreferencesRepo,    │
│            FileSystem, Structured Logger               │
└────────────────────────────────────────────────────────┘
```

For complete specifications of the future real-time audio subsystem, see:
- [DAW_ENGINE_ARCHITECTURE.md](file:///e:/SonaraDaw/docs/DAW_ENGINE_ARCHITECTURE.md)
- [PROJECT_FORMAT.md](file:///e:/SonaraDaw/docs/PROJECT_FORMAT.md)
- [PACKAGING.md](file:///e:/SonaraDaw/docs/PACKAGING.md)

---

## 2. Real-Time Audio Engine Discipline (Mandatory for Future Phases)

The audio rendering thread is a hard real-time context. The following invariants are strictly enforced:

1. **Zero Dynamic Allocation on the Audio Thread:**
   - No `malloc`, `new`, `std::make_shared`, `std::vector::push_back`, or string operations inside `processBlock()` or any audio callback.
   - All buffers, delay lines, and scratch arrays must be preallocated during `prepareToPlay()`.

2. **Zero Blocking or Synchronization Primitives:**
   - No `std::mutex`, `juce::CriticalSection`, `std::condition_variable`, or file I/O on the audio thread.
   - Communication between the UI thread and the audio thread uses wait-free lockless ring buffers (`juce::AbstractFifo`, atomic pointers, or triple buffering).

3. **Sample-Accurate Parameter Automation:**
   - Parameter adjustments from automation curves or MIDI CC are interpolated on sample boundaries, avoiding zipper noise and artifacts.

---

## 3. Project Persistence (Schema v2)

Project files (`.arcline`) use a versioned JSON schema written atomically to disk:

- **Atomic File Writing:** Projects are written to a `.tmp` file and renamed upon successful write to prevent file corruption in case of power loss or sudden crash.
- **Waveform Peak Cache:** Audio regions store pre-computed peak pairs (`min`, `max` per block) to enable instantaneous waveform rendering without re-scanning disk files.
- **MIDI Event Sequences:** Software instrument regions store discrete `MidiNote` lists (`pitch`, `startTick`, `lengthTicks`, `velocity`).
- **Tick Timebase:** Musical timing uses a standard high-resolution base of 960 ticks per quarter note (`kTicksPerBeat = 960`).
- **Transparent Migration:** Legacy `.sonora` projects load automatically and serialize cleanly to modern `.arcline` Schema v2.

---

*ARCLINE — Shape Your Sound.*
