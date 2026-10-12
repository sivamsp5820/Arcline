# ARCLINE — Future DAW Engine Architecture & Real-Time Specifications

> **Product:** ARCLINE Desktop Digital Audio Workstation  
> **Tagline:** "Shape Your Sound."  
> **Version:** 0.1.0-alpha (Phase 1 Application Shell & Architectural Blueprint)  
> **Document Status:** Authoritative Architectural Specification for Phase 2+ Audio Implementation  

---

## 1. Architectural Principles & Clean Separation

ARCLINE enforces strict architectural decoupling between the **User Interface**, the **Domain Model**, and the **Audio Engine Processing Graph**. 

```
┌────────────────────────────────────────────────────────────────────────┐
│                        USER INTERFACE LAYER                            │
│  MainLayoutComponent | ArrangementView | InspectorPanel | MixerPanel  │
│             TransportBar | CommandPalette | PreferencesDialog          │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │ Commands & State Subscriptions
                                    ▼
┌────────────────────────────────────────────────────────────────────────┐
│                     APPLICATION & DOMAIN MODEL                         │
│     AppContext | NavigationController | CommandRegistry | Project      │
│                  Track | Region | Preferences | Logger                 │
└──────────────────┬─────────────────────────────────┬───────────────────┘
                   │ Atomic Snapshots                │ Async Tasks
                   ▼                                 ▼
┌─────────────────────────────────────┐ ┌────────────────────────────────┐
│   REAL-TIME AUDIO ENGINE BOUNDARY   │ │    BACKGROUND WORKER THREADS   │
│ IAudioEngine   | ITransportControl  │ │ - Audio file disk streaming    │
│ IAudioDevice   | IMixerEngine       │ │ - Waveform peak generation     │
│ IMidiSubsystem | IVst3HostService   │ │ - VST3 background scanning     │
└─────────────────────────────────────┘ └────────────────────────────────┘
```

### Key Architectural Boundaries:
1. **The UI Never Calls Audio Processing Methods Directly:**  
   UI components dispatch user intents as typed commands to `AppContext` or post parameter changes into lock-free single-producer single-consumer (SPSC) ring buffers.
2. **The Audio Thread Never Reads UI State Directly:**  
   The audio callback operates exclusively on pre-allocated, snapshot-isolated state buffers.
3. **No Fabricated DSP in Phase 1:**  
   In Phase 1, UI controls that target future audio stages (such as VST3 insert slots, mixer bus sends, and hardware record arming) are visually present and clearly disabled or labeled with their target phase (e.g. `[VST3 Hosting - Phase 6]`, `[Mixer Routing - Phase 5]`). No empty audio loop or dummy audio thread is instantiated until Phase 2.

---

## 2. Hard Real-Time Safety Rules (Golden Rules of Audio DSP)

In native digital audio workstations, the audio rendering callback runs at an elevated real-time priority directly driven by operating system hardware interrupts. A late audio buffer results in an audible dropout ("buffer underrun" or "glitch").

To ensure glitch-free operation at buffer sizes down to 64 samples (1.33 ms at 48 kHz), the audio thread in ARCLINE must adhere to the following non-negotiable rules:

### Rule 1: Zero Dynamic Memory Allocations
- **Strictly Prohibited:** `malloc()`, `free()`, `new`, `delete`, `realloc()`, `std::vector::push_back()` (when reallocation occurs), `std::string` concatenation, `std::make_shared()`, `std::make_unique()`.
- **Enforcement:** All voice buffers, DSP scratchpads, delay lines, and channel matrices are pre-allocated during `IAudioEngine::initialize()` or track configuration on the application thread prior to playback starting.

### Rule 2: Zero Blocking Synchronization
- **Strictly Prohibited:** `std::mutex::lock()`, `juce::CriticalSection::enter()`, `std::condition_variable::wait()`, `std::binary_semaphore::acquire()`.
- **Enforcement:** Communication across threads uses lock-free, wait-free ring buffers (SPSC queues) and atomic memory primitives with acquire-release memory semantics (`std::atomic<float>`, `std::atomic<bool>`).

### Rule 3: Zero File System or Disk I/O
- **Strictly Prohibited:** `fopen()`, `std::ifstream`, `juce::File::loadStrings()`, POSIX `read()`/`write()`, Win32 `ReadFile()`/`WriteFile()`.
- **Enforcement:** Audio streaming from disk is offloaded to a dedicated background Disk I/O worker thread. The worker streams PCM chunks into a double-buffered circular memory pool ahead of the playhead.

### Rule 4: Zero Network I/O and Inter-Process Communication
- **Strictly Prohibited:** Sockets, HTTP requests, named pipes, RPC, system loggers, stdout/stderr output.
- **Enforcement:** Diagnostics and telemetry are written to memory ring buffers; background threads consume these buffers and write to `%APPDATA%\Arcline\arcline_debug.log`.

### Rule 5: Deterministic Bounded Algorithmic Complexity
- All inner loops must run in $O(N)$ time with deterministic upper bounds, where $N$ is the hardware block size (e.g., 64, 128, 256 samples). No unbounded iterations, while-loops without exit guarantees, or complex graph searches in the callback.

---

## 3. Subsystem Boundaries and Interfaces

### 3.1 Audio Device Manager (`IAudioDeviceService`)
- **Responsibility:** Enumerates, configures, and handles low-level OS audio drivers:
  - Windows: WASAPI (Exclusive/Shared) and native ASIO.
  - macOS: CoreAudio HAL.
  - Linux: ALSA, PipeWire (via JACK emulation or native SPA), and JACK.
- **Failover:** Handles hardware disconnection (e.g., USB interface unplugged) cleanly by halting the engine and notifying the UI thread via safe event queues, without crashing the process.

### 3.2 Transport Controller (`ITransportController`)
- **Responsibility:** Sample-accurate clock generation, playback state tracking, loop/cycle boundaries, and musical time calculation.
- **Synchronization:** The audio thread updates an atomic 64-bit sample counter on each block. The UI computes Bars:Beats:Ticks at 60 Hz without taking locks.

### 3.3 Mixer Summing Engine (`IMixerEngine`)
- **Responsibility:** Multitrack audio graph evaluation, fader gain, stereo panning, solo-in-place (SIP) logic, and aux send routing.
- **De-zippering:** All fader and pan changes are smoothed over block intervals using linear or exponential interpolation to prevent audible zipper noise.
- **Telemetry:** Peak and RMS metering values are periodically pushed to lock-free UI telemetry queues.

### 3.4 MIDI Subsystem (`IMidiSubsystem`)
- **Responsibility:** Ingests hardware MIDI messages (Note On/Off, CC, Pitch Bend) and dispatches them with sample-accurate timestamps into the audio buffer processing cycle.
- **Virtual Instruments:** Converts incoming messages to VST3 `IEventList` structures without heap allocations.

### 3.5 VST3 Host Engine (`IVst3HostService`)
- **Responsibility:** Discovers, instantiates, and executes third-party audio plugins conforming to the Steinberg VST3 specification.
- **Crash Containment:** Plugin scanning occurs in a child process or guarded background worker thread to shield ARCLINE from corrupt third-party binaries.
- **Processing:** Invocations of `IAudioProcessor::process()` run directly in the real-time audio thread using pre-allocated `ProcessData` buffers.

---

## 4. Phase Migration Roadmap

| Phase | Milestone | Real-Time Scope |
|---|---|---|
| **Phase 1** | Application Shell, Dark Theme, Project Durability | Interface shell, mock-free UI state, architectural boundaries, `.arcline` format |
| **Phase 2** | Audio Device Engine & Transport | WASAPI/CoreAudio/ALSA backends, sample clock, playback loop |
| **Phase 3** | Audio Recording & Waveform Streaming | Disk I/O worker, double-buffered streaming, live WAV recording |
| **Phase 4** | MIDI Subsystem & Piano Roll Engine | Hardware MIDI capture, virtual instrument dispatch, note editor sync |
| **Phase 5** | Mixer Routing & Summing Engine | Aux sends, summing bus, de-zippered faders, telemetry meters |
| **Phase 6** | VST3 Hosting Engine | Out-of-process scanner, plugin GUI hosting, automation lanes |
| **Phase 7** | Automation & Offline Export | Sample-accurate automation curves, multi-threaded stem bounce |

---

*ARCLINE — Shape Your Sound.*
