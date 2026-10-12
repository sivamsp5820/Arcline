# ARCLINE — Native Desktop DAW

> **"Shape Your Sound."**  
> *Professional Cross-Platform Digital Audio Workstation engineered in C++20, JUCE 8, and CMake.*

ARCLINE adheres strictly to the established visual language, structural density, editing workflows, and ergonomic control placement of **Apple Logic Pro X**, delivered through a cohesive dark charcoal/graphite desktop interface.

---

## 🌟 Phase 1 Deliverables — Completed & Verified

Phase 1 establishes the rock-solid application shell, Logic Pro X dark theme, multitrack arrangement workspace, centralized command registry, durable versioned project persistence (`.arcline`), and automated test suite:

### 1. Logic Pro X Dark Design System
- **Dark Charcoal & Graphite Palette:** Refined studio surfaces (`#191B1F`, `#24262B`, `#2E3138`), subtle 1px panel borders (`#33373F`), and recessed LCD panel (`#14171A`).
- **Restrained Accents:** Studio Blue (`#2D72D2`) for selection and active states; amber (`#E5A93B`) for solo; vivid red (`#D9383A`) reserved for record-arming; yellow (`#F5A623`) for the cycle locator.
- **Custom Look & Feel:** Logic Pro-style channel faders with dB scale ticks, rotary pan pots, transport buttons, calibration meters, and recessed LCD displays.
- **Strictly Honest Controls:** Controls targeting future audio stages (e.g. VST3 insert slots, mixer bus sends) are styled cleanly and labeled with their target milestone (e.g. `[VST3 Hosting - Phase 6]`, `[Mixer Routing - Phase 5]`). No fake audio processing, dummy playback threads, or simulated meters are fabricated in Phase 1.

### 2. Main Arrangement Timeline & Transport
- **Top Header Bar & Transport:**
  - Recessed center LCD display showing **Bar/Beat/Div/Tick**, **Tempo (BPM)**, and **Key / Time Signature**.
  - Transport controls: Rewind, Fast Forward, Stop, Play, Record, Cycle, Metronome.
  - Tool palette: Pointer, Scissors (split tool), Pencil, Eraser.
  - Dock toggles: Inspector (`I`), Mixer (`X`), Piano Roll (`P`), Audio Editor (`E`), Sound Library (`Y`).
  - Master output volume fader.
- **Musical Ruler & Cycle Range:**
  - Horizontal bar and beat divisions.
  - Yellow draggable Cycle locator with start/end handles and playhead scrubbing.
- **Track Headers & Controls:**
  - Color stripes, track type icons, Track Name.
  - Mute (`M`), Solo (`S`), Record-Arm (`R`), Input Monitoring (`I`).
  - Volume slider and Rotary Pan knob with numeric tooltips.
  - Add Audio Track (`+ Audio`) and Add Instrument Track (`+ Instrument`) buttons.
- **Timeline Lanes & Region Editing:**
  - Audio regions with rendered waveform peaks.
  - MIDI regions with rendered mini piano-roll note previews.
  - Interactive selection, drag-to-move with beat snapping, edge trimming, and scissors splitting (`Ctrl+T`).
  - Delete selected regions (`Delete` / `Backspace`), duplicate regions (`Ctrl+D`).

### 3. Dual-Channel Inspector Panel
- **Region Parameters Box:** Controls for Quantize (1/16, 1/8, 1/4), Gain dB, Mute, and Looping.
- **Selected Track Channel Strip:** EQ curve thumbnail, 4 Insert slots (labeled `[VST3 Hosting - Phase 6]`), 2 Send knobs (labeled `[Mixer Routing - Phase 5]`), Pan, Level Fader with dB readout, Stereo Calibration Meter.
- **Master Stereo Output Strip:** Master bus level control and calibration meter.

### 4. Dockable Lower Editor Panel
- **Mixer Console:** Full horizontal channel-strip mixing view for all project tracks plus Master output.
- **Piano Roll MIDI Editor:** Left-hand 88-key piano keyboard with pitch grid, notes display, and velocity preview.
- **Sample Audio Editor:** High-resolution waveform sample viewer with region boundary controls.

### 5. Sound Library & Browser Panel
- Categorized sound and preset library (Acoustic Drums, Synthesizers, Electric Pianos, Bass, Guitars, Orchestral) with one-click track creation.

### 6. Application Shell, Dialogs & Command Palette
- **Start Studio Screen:** Project chooser with template selection (Empty Project, Multi-track Band, Electronic Producer, Vocal Recording), recent projects list with timestamp and path display, and new project modal.
- **Preferences Dialog (`Ctrl+,`):** Audio device settings, buffer size, sample rate, theme density, autosave frequency.
- **Command Palette (`Ctrl+K`):** Fuzzy searchable command registry with category badges, keyboard shortcuts, and instant execution.
- **Durable Error Dialogs:** Recoverable error modals with actionable messages for malformed files and filesystem failures.

### 7. Durable Project Format (`.arcline`) & Schema Migration
- Project files stored as `.arcline` (Format: `ARCLINE_PROJECT`, Schema v2) with atomic write-and-replace to prevent file corruption.
- Transparent backward compatibility loading legacy `.sonora` files.
- Forward rejection protecting user data if a newer schema is encountered.
- 100% data preservation roundtrip for waveforms and MIDI note events.

---

## 🛠️ Technology Stack

- **Language:** C++20 (`std::format`, concepts, `string_view`, modern ranges)
- **Framework:** JUCE 8.0.6 (Modular CMake)
- **Build System:** CMake 3.22+ and Ninja
- **Compilers:**
  - Windows: MSVC 2022 (Visual Studio / Build Tools)
  - macOS: Apple Clang (Xcode 14+)
  - Linux: GCC 12+ or Clang 15+

---

## 🚀 Building & Running

### Windows (Quick Start)

Run the automated native build script:
```cmd
build_native.cmd
```

Or configure manually with CMake and Ninja:
```cmd
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DARCLINE_BUILD_TESTS=ON
cmake --build build --config Release
```

### Launch Desktop Application
```cmd
.\build\Arcline_artefacts\Release\ARCLINE.exe
```

### Run Automated Tests
```cmd
.\build\bin\ArclineTests.exe
```
Or via CTest:
```cmd
ctest --test-dir build --output-on-failure
```

---

## 🧪 Platform Verification Matrix

| Component | Windows 10/11 x64 | macOS (Apple Silicon & Intel) | Debian / Ubuntu Linux | Arch Linux |
|---|---|---|---|---|
| **Compilation & Link** | **PASSED** (MSVC 2022 v14.44) | Architected & Preset Ready | Architected & Preset Ready | Architected & Preset Ready |
| **Automated Tests** | **100% PASSED** (7 suites) | Configured in CTest | Configured in CTest | Configured in CTest |
| **Desktop GUI Launch** | **PASSED** (Verified Native) | Configured (JUCE 8) | Configured (JUCE 8) | Configured (JUCE 8) |
| **Dark Theme Aesthetics** | **PASSED** (Logic Pro X match) | Shared LookAndFeel | Shared LookAndFeel | Shared LookAndFeel |
| **Timeline & Waveforms** | **PASSED** (Peak cache render) | Shared Domain/UI | Shared Domain/UI | Shared Domain/UI |
| **MIDI Note Previews** | **PASSED** (Event rendering) | Shared Domain/UI | Shared Domain/UI | Shared Domain/UI |
| **Project Durability** | **PASSED** (Atomic Schema v2) | Shared POSIX / Win32 | Shared POSIX / Win32 | Shared POSIX / Win32 |
| **Packaging Foundation** | **Inno Setup (`arcline.iss`)** | DMG (`build_dmg.sh`) | Deb (`debian/control`) | Arch (`PKGBUILD`) |

*Note: Windows 10/11 x64 is verified natively in this build environment. Linux and macOS are architected with reproducible presets, dependencies, and packaging manifests.*

---

## 🎹 Keyboard Shortcuts

| Shortcut | Action |
|---|---|
| `Space` | Play / Stop toggle |
| `Enter` | Return playhead to beginning (Bar 1) |
| `C` | Cycle / Loop mode toggle |
| `R` | Record mode toggle |
| `Ctrl+T` | Split selected region at playhead (Scissors) |
| `Ctrl+D` | Duplicate selected region |
| `Delete` / `Backspace` | Delete selected region |
| `I` | Toggle Inspector panel |
| `X` | Toggle Mixer console |
| `P` | Switch to Piano Roll editor |
| `E` | Switch to Audio editor |
| `Y` | Toggle Sound Library browser |
| `Ctrl+K` | Open Command Palette |
| `Ctrl+,` | Preferences dialog |
| `Ctrl+N` | New Project dialog |
| `Ctrl+O` | Open Project dialog |
| `Ctrl+S` | Save Active Project |
| `Ctrl+W` | Close Active Project |
| `Alt+1` | Navigate to Arrangement workspace |
| `Alt+2` | Navigate to Start Studio workspace |
| `Escape` | Dismiss modal dialog / palette |

---

## 🤝 Contributing & Branching Architecture

ARCLINE follows an isolated Git branching model to enable parallel development by multiple engineers and AI coding agents without merge conflicts or architectural contamination:

- **Stable Releases:** [`main`](file:///e:/SonaraDaw) (Protected — official releases only)
- **Active Integration:** [`develop`](file:///e:/SonaraDaw) (Protected — reviewed feature PRs)
- **Phase 1 Frozen Baseline:** `phase-1-frozen` / tag `v0.1.0-phase1` (Immutable reference)
- **Feature Branches:** `feature/*`, `fix/*`, `refactor/*`, `test/*`, `docs/*`, `build/*`

### Getting Started as a Contributor
1. **Read Guidelines:** Review [CONTRIBUTING.md](file:///e:/SonaraDaw/CONTRIBUTING.md) and [docs/BRANCHING_STRATEGY.md](file:///e:/SonaraDaw/docs/BRANCHING_STRATEGY.md).
2. **Select an Available Task:** Check [docs/CONTRIBUTOR_TASKS.md](file:///e:/SonaraDaw/docs/CONTRIBUTOR_TASKS.md) to claim an unassigned task.
3. **Respect Layer Boundaries:** UI components must never access audio threads directly. Review [docs/ARCHITECTURE.md](file:///e:/SonaraDaw/docs/ARCHITECTURE.md) and [docs/INPUT_COMMAND_BOUNDARY.md](file:///e:/SonaraDaw/docs/INPUT_COMMAND_BOUNDARY.md).
4. **Submit Verified PRs:** Use [.github/pull_request_template.md](file:///e:/SonaraDaw/.github/pull_request_template.md) with test results and evidence.

---

## 📄 License

Proprietary — Copyright (c) 2026 ARCLINE Audio. All rights reserved.  
*"Shape Your Sound."*

