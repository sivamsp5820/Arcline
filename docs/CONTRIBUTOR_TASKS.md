# ARCLINE — Contributor Task Catalog & Work Breakdown Structure

> **Product:** ARCLINE Desktop Digital Audio Workstation  
> **Tagline:** "Shape Your Sound."  
> **Architecture:** C++20 | JUCE 8 | CMake | Ninja  
> **Target Platforms:** Windows 10/11, macOS (Apple Silicon/Intel), Ubuntu/Debian Linux, Arch Linux  
> **Document ID:** `DOC-WORKFLOW-001`  
> **Status:** Active & Enforced  

---

## 1. How to Use This Task Catalog

All work in the ARCLINE codebase—whether performed by human contributors, engineering teams, or AI coding agents—must originate from a designated task in this catalog.

### Task Lifecycle & Status Definitions

| Status | Meaning | Action Permitted |
| :--- | :--- | :--- |
| `AVAILABLE` | Preconditions and dependencies are satisfied. Ready to be claimed. | Any contributor may claim this task, create the designated feature branch from `develop`, and begin work. |
| `IN PROGRESS` | A contributor is actively developing this task. | **Do not duplicate work.** Coordinate with the assigned contributor before touching related files. |
| `BLOCKED` | Dependent on an unfinished task, missing architectural contract, or external gate. | **Do not begin.** Resolve blocking dependencies first. |
| `IN REVIEW` | Implementation is complete, tests pass, and PR is awaiting team review. | Reviewers audit code, test results, visual/audio evidence, and CI gates. |
| `COMPLETED` | Merged into `develop` or frozen into baseline (`v0.1.0-phase1`). Fully verified. | Preserved as verified baseline. |

### Contributor Rules of Engagement
1. **Never guess an interface:** If your task interacts with another layer, read the interface contracts in [ARCHITECTURE.md](file:///e:/SonaraDaw/docs/ARCHITECTURE.md), [DAW_ENGINE_ARCHITECTURE.md](file:///e:/SonaraDaw/docs/DAW_ENGINE_ARCHITECTURE.md), and [INPUT_COMMAND_BOUNDARY.md](file:///e:/SonaraDaw/docs/INPUT_COMMAND_BOUNDARY.md).
2. **Observe file boundaries:** Each task explicitly documents **"Files Expected to Change"** and **"Files That Must Not Be Changed"**. Violating file boundaries causes merge conflicts and will result in pull-request rejection.
3. **One task per branch:** Feature branches must implement exactly one task. Never bundle unrelated bug fixes or cosmetic refactorings into a feature branch.

---

## 2. Master Task Catalog

### Category A: UI Foundation & Shell

#### `UI-001`: Main Desktop Application Shell & Workspaces
- **Category:** UI Foundation
- **Git Branch Name:** `feature/ui-shell`
- **Objective:** Establish native desktop window, multi-workspace routing (Start Studio, Arrangement), responsive scaling, and top-level layout docking.
- **Scope:** Window lifecycle, window bounds persistence, `MainLayoutComponent`, `NavigationController`, workspace switching (`Alt+1`, `Alt+2`).
- **Dependencies:** None
- **Files Expected to Change:** `src/app/MainWindow.*`, `src/ui/components/MainLayoutComponent.*`, `src/app/NavigationController.*`
- **Files That Must Not Be Changed:** `src/future_audio/*`, `src/infrastructure/ProjectRepository.*`
- **Required Skills:** C++20, JUCE `DocumentWindow` & `Component` lifecycle, UI layout math.
- **Acceptance Criteria:** Launches native borderless/custom titlebar window; switches cleanly between Start Studio and Arrangement; clean shutdown < 100 ms.
- **Required Tests:** `tests/unit/NavigationTests.cpp`, `tests/unit/LayoutVerificationTests.cpp`
- **Current Status:** `COMPLETED`
- **Assigned Contributor:** ARCLINE Core Team (Verified in Phase 1 Baseline `v0.1.0-phase1`)

#### `UI-002`: Logic Pro X Dark Look & Feel & Design Tokens
- **Category:** UI Foundation
- **Git Branch Name:** `feature/ui-theme`
- **Objective:** Implement cohesive Logic Pro X dark graphite theme, centralized color tokens, custom component styling, and typographic hierarchy.
- **Scope:** `SonoraTheme`, `SonoraLookAndFeel`, faders with dB ticks, rotary pan pots, transport buttons, recessed LCD panels.
- **Dependencies:** None
- **Files Expected to Change:** `src/ui/theme/*`
- **Files That Must Not Be Changed:** `src/domain/*`, `src/future_audio/*`
- **Required Skills:** JUCE `LookAndFeel_V4` customization, 2D vector graphics rendering (`juce::Graphics`).
- **Acceptance Criteria:** Full compliance with [DESIGN_SYSTEM.md](file:///e:/SonaraDaw/docs/DESIGN_SYSTEM.md); high-contrast legible dark UI; zero default OS widgets.
- **Required Tests:** Visual inspection against reference screens; `LayoutVerificationTests`
- **Current Status:** `COMPLETED`
- **Assigned Contributor:** ARCLINE Core Team (Verified in Phase 1 Baseline `v0.1.0-phase1`)

#### `UI-003`: Centralized Command Palette & Fuzzy Search
- **Category:** UI Foundation
- **Git Branch Name:** `feature/ui-command-palette`
- **Objective:** Provide a fast, keyboard-driven fuzzy command launcher (`Ctrl+K` / `Cmd+K`) listing all registered DAW actions.
- **Scope:** `CommandPaletteOverlay`, keyboard focus capture, search filtering, category badges, execution dispatch.
- **Dependencies:** `PRJ-001`
- **Files Expected to Change:** `src/ui/dialogs/CommandPaletteOverlay.*`, `src/core/CommandRegistry.*`
- **Files That Must Not Be Changed:** `src/domain/Project.*`, `src/infrastructure/*`
- **Required Skills:** JUCE keyboard listeners, fuzzy search algorithms, modal component overlay.
- **Acceptance Criteria:** Opens in < 16 ms; searches 30+ registered commands; arrows + Enter executes command and closes modal.
- **Required Tests:** `tests/unit/CommandRegistryTests.cpp`
- **Current Status:** `COMPLETED`
- **Assigned Contributor:** ARCLINE Core Team (Verified in Phase 1 Baseline `v0.1.0-phase1`)

#### `UI-004`: Timeline Waveform Rendering & Peak Cache Streaming
- **Category:** Arrangement & Editing UI
- **Git Branch Name:** `feature/ui-arrangement`
- **Objective:** Optimize timeline audio region waveform rendering using block-based peak caches for instantaneous display during horizontal zoom/scroll.
- **Scope:** Region waveform geometry caching, multi-resolution mipmapped peaks, LOD rendering in `ArrangementView`.
- **Dependencies:** `UI-001`, `PRJ-001`
- **Files Expected to Change:** `src/ui/workspaces/ArrangementView.*`
- **Files That Must Not Be Changed:** `src/domain/Track.*`, `src/infrastructure/ProjectRepository.*`, `src/future_audio/*`
- **Required Skills:** JUCE 2D graphics performance, geometry caching, path rendering.
- **Acceptance Criteria:** 60 FPS scrolling with 64 tracks; zero heap allocations inside `paint()`.
- **Required Tests:** Waveform rendering unit tests; UI benchmark suite.
- **Current Status:** `AVAILABLE`
- **Assigned Contributor:** Unassigned

#### `UI-005`: Piano Roll Interactive Note Grid & Velocity Editor
- **Category:** Arrangement & Editing UI
- **Git Branch Name:** `feature/ui-piano-roll`
- **Objective:** Transform the dockable Piano Roll lower panel into an interactive MIDI note sequencer with note creation, dragging, resizing, and velocity stalks.
- **Scope:** 88-key vertical keyboard interaction, pitch grid drag/drop, note selection, velocity lane adjustment, quantize trigger.
- **Dependencies:** `UI-001`, `PRJ-001`
- **Files Expected to Change:** `src/ui/components/LowerEditorPanel.*`
- **Files That Must Not Be Changed:** `src/future_audio/*`, `src/domain/UndoHistory.*`
- **Required Skills:** JUCE mouse drag handling, MIDI pitch mathematics, coordinate mapping.
- **Acceptance Criteria:** Add note on double click; drag to pitch/time with snap; update region's `MidiNote` list via undoable command.
- **Required Tests:** Piano roll interaction tests; command dispatch tests.
- **Current Status:** `AVAILABLE`
- **Assigned Contributor:** Unassigned

---

### Category B: Audio Engine & Subsystems (Phase 2 Focus)

#### `AUD-001`: Audio Device Enumeration & Backend Service
- **Category:** Audio Engine
- **Git Branch Name:** `feature/audio-devices`
- **Objective:** Implement hardware audio device enumeration and configuration backend across Windows (WASAPI/ASIO), macOS (CoreAudio), and Linux (ALSA/PipeWire/JACK).
- **Scope:** Implement concrete `AudioDeviceService` conforming to [IAudioDeviceService.h](file:///e:/SonaraDaw/src/future_audio/IAudioDeviceService.h), sample rate and buffer size querying, device hot-plug callbacks.
- **Dependencies:** `PRJ-001` (for `Preferences` persistence)
- **Files Expected to Change:** `src/audio/AudioDeviceService.*` (new), `src/future_audio/IAudioDeviceService.h`
- **Files That Must Not Be Changed:** `src/ui/*`, `src/domain/Project.*`
- **Required Skills:** JUCE `AudioDeviceManager`, OS audio driver APIs, thread safety.
- **Acceptance Criteria:** Enumerates available input/output devices; changes buffer size without crash; recovers from device disconnection.
- **Required Tests:** Device enumeration headless tests; mock device driver unit test.
- **Current Status:** `AVAILABLE`
- **Assigned Contributor:** Unassigned

#### `AUD-002`: Hard Real-Time Audio Engine & Callback Pipeline
- **Category:** Audio Engine
- **Git Branch Name:** `feature/audio-engine`
- **Objective:** Implement the master real-time rendering callback conforming to [IAudioEngine.h](file:///e:/SonaraDaw/src/future_audio/IAudioEngine.h) with strict real-time safety invariants.
- **Scope:** Master processing callback (`processBlock`), zero dynamic memory allocation, zero locks, preallocated scratch buffers, xrun telemetry counter.
- **Dependencies:** `AUD-001`
- **Files Expected to Change:** `src/audio/AudioEngine.*` (new), `src/future_audio/IAudioEngine.h`
- **Files That Must Not Be Changed:** `src/ui/*`, `src/domain/ProjectRepository.*`
- **Required Skills:** Modern C++ real-time audio programming, lock-free data structures, cache locality.
- **Acceptance Criteria:** Verified zero allocations on audio thread (monitored via test hook); buffer throughput benchmark < 15% CPU load at 128 samples / 48 kHz.
- **Required Tests:** Real-time safety test assertion; audio rendering test suite.
- **Current Status:** `AVAILABLE` (Can be developed concurrently once `AUD-001` interface stub is ready)
- **Assigned Contributor:** Unassigned

#### `AUD-003`: Multitrack Disk Streaming & WAV Recording Engine
- **Category:** Audio Engine
- **Git Branch Name:** `feature/audio-dsp`
- **Objective:** Implement background disk streaming thread with double-buffered circular memory pools for dropout-free recording and playback of linear audio files.
- **Scope:** Background I/O thread, ringbuffer FIFO, broadcast WAV header formatting, disk queue exhaustion warning.
- **Dependencies:** `AUD-002`
- **Files Expected to Change:** `src/audio/streaming/*` (new)
- **Files That Must Not Be Changed:** `src/ui/*`, `src/core/CommandRegistry.*`
- **Required Skills:** Asynchronous file I/O, wait-free ringbuffers (`juce::AbstractFifo`), audio format decoding.
- **Acceptance Criteria:** Stream 32 simultaneous 24-bit/48kHz channels without xruns; record 4 channels continuously without audio glitches.
- **Required Tests:** Stream throughput benchmark; loopback fidelity test.
- **Current Status:** `BLOCKED` (Waiting on `AUD-002`)
- **Assigned Contributor:** Unassigned

#### `TRN-001`: Sample-Accurate Transport Controller & Musical Clock
- **Category:** Audio Engine
- **Git Branch Name:** `feature/audio-transport`
- **Objective:** Implement transport controller conforming to [ITransportController.h](file:///e:/SonaraDaw/src/future_audio/ITransportController.h) linking UI playhead and audio sample counter.
- **Scope:** Atomic transport state (Playing, Stopped, Recording, Looping), sample-to-musical-timebase conversion (960 PPQ), cycle range looping, smooth scrubbing.
- **Dependencies:** `AUD-001`
- **Files Expected to Change:** `src/audio/TransportController.*` (new), `src/ui/components/TransportBar.cpp` (bridge integration)
- **Files That Must Not Be Changed:** `src/infrastructure/*`, `src/domain/UndoHistory.*`
- **Required Skills:** Discrete math, lock-free atomic synchronization, sample rate conversion.
- **Acceptance Criteria:** Drift-free musical clock over 60 minutes; loop point wrapping sample-accurate; transport state readable lock-free by UI.
- **Required Tests:** Transport clock accuracy tests; cycle wrap timing tests.
- **Current Status:** `AVAILABLE`
- **Assigned Contributor:** Unassigned

---

### Category C: Project & Session Management

#### `PRJ-001`: Durable `.arcline` Project Persistence & Atomic IO
- **Category:** Project Management
- **Git Branch Name:** `feature/project-format`
- **Objective:** Design and implement durable, versioned JSON serialization format (`.arcline` Schema v2) with atomic file saving and legacy `.sonora` migration.
- **Scope:** `ProjectRepository`, schema migration, atomic write-to-temp-then-rename, forward schema rejection, waveform peak cache and MIDI note serialization.
- **Dependencies:** None
- **Files Expected to Change:** `src/infrastructure/ProjectRepository.*`, `src/domain/Project.*`
- **Files That Must Not Be Changed:** `src/future_audio/*`, `src/ui/*`
- **Required Skills:** C++20 serialization, file system atomicity, JSON parser safety.
- **Acceptance Criteria:** 100% data roundtrip integrity; power-loss simulation test leaves original file intact; legacy format loads cleanly.
- **Required Tests:** `tests/unit/ProjectTests.cpp`, `tests/unit/ProjectFormatDurabilityTests.cpp`
- **Current Status:** `COMPLETED`
- **Assigned Contributor:** ARCLINE Core Team (Verified in Phase 1 Baseline `v0.1.0-phase1`)

#### `PRJ-002`: Autosave Engine & Crash Recovery Journal
- **Category:** Project Management
- **Git Branch Name:** `feature/project-recovery`
- **Objective:** Provide automated background autosave at configurable intervals and crash detection with recovery prompt on next startup.
- **Scope:** Dedicated autosave thread, snapshot journal directory (`.arcline_autosave`), crash marker file, startup recovery dialog.
- **Dependencies:** `PRJ-001`
- **Files Expected to Change:** `src/infrastructure/AutoSaveService.*` (new), `src/app/AppContext.cpp`, `src/domain/Preferences.*`
- **Files That Must Not Be Changed:** `src/future_audio/*`, `src/ui/theme/*`
- **Required Skills:** Background workers, atomic file handling, recovery UI flow.
- **Acceptance Criteria:** Autosave runs without UI freeze; corrupted exit triggers recovery prompt; user can recover or discard unsaved state.
- **Required Tests:** Recovery simulation test; background autosave timing test.
- **Current Status:** `AVAILABLE`
- **Assigned Contributor:** Unassigned

---

### Category D: MIDI & Software Instruments

#### `MIDI-001`: Hardware MIDI Enumeration & Lock-Free Event FIFO
- **Category:** MIDI and Instruments
- **Git Branch Name:** `feature/midi-engine`
- **Objective:** Implement MIDI subsystem conforming to [IMidiSubsystem.h](file:///e:/SonaraDaw/src/future_audio/IMidiSubsystem.h) with hardware device scanning and lock-free message queues.
- **Scope:** Hardware MIDI In/Out discovery, lock-free ringbuffer passing `juce::MidiMessage` from OS callback to audio thread, timestamp alignment.
- **Dependencies:** `AUD-002`
- **Files Expected to Change:** `src/midi/MidiSubsystem.*` (new), `src/future_audio/IMidiSubsystem.h`
- **Files That Must Not Be Changed:** `src/ui/*`, `src/infrastructure/*`
- **Required Skills:** MIDI 1.0/2.0 specifications, lock-free ringbuffers, real-time audio scheduling.
- **Acceptance Criteria:** Sub-millisecond latency from hardware key press to audio thread; zero allocation on MIDI callback.
- **Required Tests:** MIDI throughput benchmark; timing jitter verification test.
- **Current Status:** `BLOCKED` (Waiting on `AUD-002`)
- **Assigned Contributor:** Unassigned

#### `MIDI-002`: Built-in Polyphonic Subtractive Synthesizer
- **Category:** MIDI and Instruments
- **Git Branch Name:** `feature/midi-instruments`
- **Objective:** Implement a lightweight, native 8-voice subtractive synthesizer for instrument tracks (oscillators, ladder filter, ADSR, amplifier).
- **Scope:** Voice management, band-limited BLEP oscillators, state-variable filter, envelope generators, preset serialization.
- **Dependencies:** `AUD-002`, `MIDI-001`
- **Files Expected to Change:** `src/synth/*` (new)
- **Files That Must Not Be Changed:** `src/ui/*`, `src/app/*`
- **Required Skills:** DSP synthesis algorithms, voice allocation, SIMD optimization.
- **Acceptance Criteria:** Clean aliasing-free sound from C1 to C8; CPU usage < 2% for 8 voices at 48kHz.
- **Required Tests:** Synthesis rendering test; voice stealing correctness test.
- **Current Status:** `BLOCKED` (Waiting on `MIDI-001`)
- **Assigned Contributor:** Unassigned

---

### Category E: VST3 & Third-Party Plugin Hosting

#### `VST-001`: Out-of-Process VST3 Plugin Scanner Foundation
- **Category:** VST3 & Plugin Hosting
- **Git Branch Name:** `feature/plugin-scanner`
- **Objective:** Implement isolated subprocess plugin scanner that discovers and validates installed VST3 plugins without crashing the main DAW process.
- **Scope:** Executable helper (`ArclinePluginScanner`), VST3 directory crawling (`%COMMONPROGRAMFILES%/VST3`, `/Library/Audio/Plug-Ins/VST3`), IPC metadata exchange, blacklist database.
- **Dependencies:** `PRJ-001` (to persist plugin cache)
- **Files Expected to Change:** `src/plugins/PluginScanner.*` (new), `src/plugins/PluginRegistry.*` (new)
- **Files That Must Not Be Changed:** `src/future_audio/*`, `src/ui/*`
- **Required Skills:** Subprocess IPC, VST3 SDK / JUCE hosting module, crash resilience.
- **Acceptance Criteria:** Bad plugin crashing during scan is blacklisted; scan completes in background; populates cached plugin list.
- **Required Tests:** Mock crash plugin scan test; plugin database serialization test.
- **Current Status:** `BLOCKED` (Waiting on `AUD-001`)
- **Assigned Contributor:** Unassigned

#### `VST-002`: VST3 Audio Effect & Instrument Hosting Engine
- **Category:** VST3 & Plugin Hosting
- **Git Branch Name:** `feature/vst3-host`
- **Objective:** Implement runtime VST3 plugin hosting conforming to [IVst3HostService.h](file:///e:/SonaraDaw/src/future_audio/IVst3HostService.h), managing plugin instances, audio/MIDI processing, and state serialization.
- **Scope:** Plugin instance instantiation, `juce::AudioPluginInstance` lifecycle, parameter mapping, state chunk saving in `.arcline`.
- **Dependencies:** `VST-001`, `AUD-002`
- **Files Expected to Change:** `src/plugins/Vst3HostService.*` (new), `src/future_audio/IVst3HostService.h`
- **Files That Must Not Be Changed:** `src/ui/workspaces/*`, `src/domain/Track.*`
- **Required Skills:** VST3 parameter trees, real-time plugin buffer preparation, plugin state chunk serialization.
- **Acceptance Criteria:** Instantiate compliant VST3 plugins; process audio block cleanly; save and restore full state chunk.
- **Required Tests:** Headless plugin test; parameter recall verification.
- **Current Status:** `BLOCKED` (Waiting on `VST-001`, `AUD-002`)
- **Assigned Contributor:** Unassigned

---

### Category F: Mixer & Audio Routing

#### `MIX-001`: Topological Audio Routing Graph & Summing Engine
- **Category:** Mixer and Audio Routing
- **Git Branch Name:** `feature/mixer-engine`
- **Objective:** Implement topological multitrack summing engine conforming to [IMixerEngine.h](file:///e:/SonaraDaw/src/future_audio/IMixerEngine.h) with channel strips, aux sends, master bus, and pan laws.
- **Scope:** Channel gain faders with sample-interpolated de-zippering, pan laws (-3dB equal power), aux send summing nodes, master limiter stage.
- **Dependencies:** `AUD-002`
- **Files Expected to Change:** `src/mixer/MixerEngine.*` (new), `src/future_audio/IMixerEngine.h`
- **Files That Must Not Be Changed:** `src/ui/*`, `src/infrastructure/*`
- **Required Skills:** Graph evaluation, audio summing, SIMD vectorization, parameter smoothing.
- **Acceptance Criteria:** 0 dB gain produces bit-exact output; pan curve satisfies equal-power law; fader adjustments cause zero clicks.
- **Required Tests:** Summing accuracy tests; parameter de-zippering verification.
- **Current Status:** `BLOCKED` (Waiting on `AUD-002`)
- **Assigned Contributor:** Unassigned

#### `MIX-002`: Real-Time Lock-Free Metering Telemetry
- **Category:** Mixer and Audio Routing
- **Git Branch Name:** `feature/ui-mixer`
- **Objective:** Bridge audio thread channel RMS/Peak levels to UI channel strips using lock-free atomic telemetry rings at 60 FPS.
- **Scope:** Sample peak hold, RMS accumulation window, atomic level readouts, UI meter component timer update.
- **Dependencies:** `MIX-001`, `UI-001`
- **Files Expected to Change:** `src/ui/components/LowerEditorPanel.cpp`, `src/ui/components/InspectorPanel.cpp`
- **Files That Must Not Be Changed:** `src/future_audio/IAudioEngine.h`, `src/domain/Project.*`
- **Required Skills:** Lock-free atomic data exchange, ballistic meter physics (IEC peak meter decay).
- **Acceptance Criteria:** Meters reflect audio levels without UI lag; audio thread executes zero locks while posting levels.
- **Required Tests:** Telemetry ringbuffer stress test; meter ballistics unit test.
- **Current Status:** `BLOCKED` (Waiting on `MIX-001`)
- **Assigned Contributor:** Unassigned

---

### Category G: Cross-Platform & Release Engineering

#### `BLD-001`: Multi-Platform GitHub Actions CI Matrix
- **Category:** Cross-Platform & Release Engineering
- **Git Branch Name:** `build/ci`
- **Objective:** Expand GitHub Actions workflow to build and test Windows (MSVC 2022), macOS (Clang), and Linux (GCC 12/Clang) on every push and PR to `develop` and `main`.
- **Scope:** `.github/workflows/ci.yml`, test report artifact archiving, matrix caching for JUCE modules.
- **Dependencies:** None
- **Files Expected to Change:** `.github/workflows/ci.yml`
- **Files That Must Not Be Changed:** `src/*`, `CMakeLists.txt`
- **Required Skills:** GitHub Actions YAML, multi-platform CMake orchestration, Linux dependency configuration.
- **Acceptance Criteria:** Green builds across Windows, Ubuntu, and macOS runners; automated failure notices on regressions.
- **Required Tests:** CI pipeline dry-run verification.
- **Current Status:** `AVAILABLE`
- **Assigned Contributor:** Unassigned

#### `PKG-001`: Linux AppImage & Debian Package Automation
- **Category:** Cross-Platform & Release Engineering
- **Git Branch Name:** `build/linux-packaging`
- **Objective:** Automate production packaging for Debian/Ubuntu (`.deb`) and portable Linux (`.AppImage`) with verified shared library dependencies.
- **Scope:** `packaging/debian/*`, `packaging/linux-portable/*`, automated bundle script.
- **Dependencies:** `BLD-001`
- **Files Expected to Change:** `packaging/debian/*`, `packaging/linux-portable/*`
- **Files That Must Not Be Changed:** `src/*`
- **Required Skills:** Linux packaging standards, `dpkg-deb`, `appimagetool`.
- **Acceptance Criteria:** Produces installable `.deb` and standalone `.AppImage` launching successfully on clean Ubuntu 22.04 and 24.04 systems.
- **Required Tests:** Linux package installation check.
- **Current Status:** `AVAILABLE`
- **Assigned Contributor:** Unassigned

---

### Category H: Quality Assurance & Testing

#### `QA-001`: Core Architecture & Durability Test Suite
- **Category:** Quality Assurance
- **Git Branch Name:** `test/project-system`
- **Objective:** Implement standalone headless automated test runner covering commands, navigation, preferences, undo/redo, and project durability.
- **Scope:** `TestRunner.cpp`, `TestAssert.h`, 10 automated test suites.
- **Dependencies:** None
- **Files Expected to Change:** `tests/*`, `CMakeLists.txt`
- **Files That Must Not Be Changed:** `src/*`
- **Required Skills:** C++ unit test design, test assertions, headless fixture design.
- **Acceptance Criteria:** 100% passing test execution in < 5 seconds; clean exit code 0.
- **Required Tests:** Self-verifying test runner.
- **Current Status:** `COMPLETED`
- **Assigned Contributor:** ARCLINE Core Team (Verified in Phase 1 Baseline `v0.1.0-phase1`)

#### `QA-002`: Automated UI Visual Regression & Screenshot Baseline
- **Category:** Quality Assurance
- **Git Branch Name:** `test/ui-regression`
- **Objective:** Implement headless off-screen rendering script that captures all application screens and compares them against verified Phase 1 visual baselines.
- **Scope:** Automated component paint-to-image harness, perceptual diff comparison, threshold failure alerts.
- **Dependencies:** `UI-001`, `UI-002`
- **Files Expected to Change:** `tests/regression/*` (new), `scripts/*`
- **Files That Must Not Be Changed:** `src/domain/*`, `src/future_audio/*`
- **Required Skills:** JUCE headless rendering, image diffing algorithms.
- **Acceptance Criteria:** Automatically detects 5% pixel shifts or color anomalies in arrangement, mixer, and dialogs.
- **Required Tests:** Off-screen rendering harness test.
- **Current Status:** `AVAILABLE`
- **Assigned Contributor:** Unassigned

---

## 3. Contributor Task Claiming & Coordination Protocol

To ensure zero conflicts between multiple human developers and AI coding agents:

1. **Verify Availability:** Check the table above. The task status must be `AVAILABLE`. If it is `BLOCKED`, resolve the dependency first.
2. **Claim the Task:** File or comment on an issue titled `[CLAIM] <Task ID>: <Task Title>` (using `.github/ISSUE_TEMPLATE/01_feature_task.md`).
3. **Set Status to IN PROGRESS:** Update this document's task entry to `IN PROGRESS` with your name/handle in your branch.
4. **Create Branch off `develop`:**
   ```powershell
   git checkout develop
   git pull origin develop
   git checkout -b <Git Branch Name>
   ```
5. **Implement Within Scope:** Edit only the files listed under **Files Expected to Change**.
6. **Submit PR:** Submit a pull request targeting `develop` using the `.github/pull_request_template.md`. Update status to `IN REVIEW`.
7. **Merge:** Upon passing CI and approved review, merge to `develop` and mark status as `COMPLETED`.

---

*ARCLINE — Shape Your Sound.*
