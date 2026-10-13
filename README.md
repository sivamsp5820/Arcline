# ARCLINE — Professional Native Desktop DAW

> **"Shape Your Sound."**  
> *Next-Generation Cross-Platform Digital Audio Workstation engineered in C++20, JUCE 8, and CMake.*  
> Inspired by the ergonomics, visual density, and production workflows of **Apple Logic Pro X**.

---

## 🧭 Table of Contents

1. [Overview & Design Philosophy](#-overview--design-philosophy)
2. [Git Branch Architecture & Categories](#-git-branch-architecture--categories)
3. [Git Initialization & Branch Setup Guide](#-git-initialization--branch-setup-guide)
4. [Phase 1 Foundation — Completed & Verified](#-phase-1-foundation--completed--verified)
5. [Technology Stack & Prerequisites](#-technology-stack--prerequisites)
6. [Building & Running the Application](#-building--running-the-application)
7. [Running the Automated Test Suite](#-running-the-automated-test-suite)
8. [Cross-Platform Verification Matrix](#-cross-platform-verification-matrix)
9. [Keyboard Shortcuts Cheat Sheet](#-keyboard-shortcuts-cheat-sheet)
10. [Architectural Invariants & Layer Boundaries](#-architectural-invariants--layer-boundaries)
11. [Complete Contributor Guide](#-complete-contributor-guide)
    - [11.1 Quick-Start: Clone, Submodules & First Build](#111-quick-start-clone-submodules--first-build)
    - [11.2 Finding & Claiming Tasks](#112-finding--claiming-tasks)
    - [11.3 Branching Strategy & Topic Branch Workflow](#113-branching-strategy--topic-branch-workflow)
    - [11.4 Architectural Invariants & Hard Real-Time Safety Rules](#114-architectural-invariants--hard-real-time-safety-rules)
    - [11.5 Coding Standards & Commit Message Conventions](#115-coding-standards--commit-message-conventions)
    - [11.6 Running Tests & Validation Checklist](#116-running-tests--validation-checklist)
    - [11.7 Submitting a Pull Request](#117-submitting-a-pull-request)
12. [License](#-license)

---

## 🎨 Overview & Design Philosophy

ARCLINE is a native desktop Digital Audio Workstation built for professional music production, audio editing, and sound design. It strictly adheres to the established visual language, structural density, editing workflows, and ergonomic control placement of **Apple Logic Pro X**, delivered through a cohesive dark charcoal/graphite desktop interface.

### Key Visual & UX Principles
- **Dark Graphite & Charcoal Surfaces:** Custom palette (`#191B1F`, `#24262B`, `#2E3138`) with subtle 1px panel borders (`#33373F`) and recessed LCD displays (`#14171A`).
- **Restrained Studio Accents:** Studio Blue (`#2D72D2`) for active selections; amber (`#E5A93B`) for solo; vivid red (`#D9383A`) reserved for record-arming; yellow (`#F5A623`) for cycle ranges.
- **Strictly Honest Controls:** Controls targeting future audio milestones (e.g. VST3 insert slots, mixer bus sends) are labeled cleanly (`[VST3 Hosting - Phase 6]`, `[Mixer Routing - Phase 5]`). No simulated meters, fake audio processing, or placeholder threads.
- **Hard Real-Time Discipline:** Zero dynamic allocations (`new`/`malloc`) and zero locks (`std::mutex`) inside real-time audio rendering loops.

---

## 🌿 Git Branch Architecture & Categories

To enable multiple software engineers, sound designers, and AI coding agents to develop distinct subsystems simultaneously without merge conflicts, the ARCLINE repository is split into **6 dedicated category branches** alongside the integration branches:

```
[main]                     <- Stable, verified releases only (v0.1.0, v0.2.0)
  ▲
[develop]                  <- Active integration staging branch
  ▲
  ├── ui                   <- Arrangement timeline, Piano roll, dark theme, panels
  ├── plugins              <- VST3 hosting, out-of-process scanner, plugin windows
  ├── audio-engine         <- Drivers (WASAPI/ASIO/CoreAudio/ALSA), real-time buffer processing
  ├── mixer                <- Multitrack summing engine, routing graph, aux sends, faders
  ├── midi                 <- Hardware MIDI inputs, note event sequencing, instruments
  └── project-core         <- .arcline JSON saving/loading, track models, undo/redo

[phase-1-frozen]           <- Immutable historical snapshot of Phase 1 foundation (Tag: v0.1.0-phase1)
```

### Category Branch Directory & Responsibility Mapping

| Branch | Subsystem Domain | Primary Files & Directories | Core Responsibilities |
| :--- | :--- | :--- | :--- |
| **`ui`** | Arrangement & Editing UI | [`src/ui/`](file:///e:/SonaraDaw/src/ui), [`src/app/MainWindow.h`](file:///e:/SonaraDaw/src/app/MainWindow.h), [`src/app/AppContext.h`](file:///e:/SonaraDaw/src/app/AppContext.h) | Timeline lanes, waveform drawing, piano roll note grid, fader controls, LookAndFeel, modals, command palette |
| **`plugins`** | VST3 & Plugin Hosting | [`src/future_audio/IVst3HostService.h`](file:///e:/SonaraDaw/src/future_audio/IVst3HostService.h), `src/plugins/` | Out-of-process plugin scanner, VST3 parameter trees, native GUI window wrappers, plugin preset recall |
| **`audio-engine`** | Audio Core & Drivers | [`src/future_audio/IAudioEngine.h`](file:///e:/SonaraDaw/src/future_audio/IAudioEngine.h), [`src/future_audio/IAudioDeviceService.h`](file:///e:/SonaraDaw/src/future_audio/IAudioDeviceService.h) | OS audio backends (WASAPI/ASIO, CoreAudio, ALSA), real-time `processBlock` rendering, transport clock |
| **`mixer`** | Audio Routing & Summing | [`src/future_audio/IMixerEngine.h`](file:///e:/SonaraDaw/src/future_audio/IMixerEngine.h), `src/mixer/` | Topological routing graph, aux bus sends, equal-power pan laws, channel summing, ballistic peak/RMS meters |
| **`midi`** | MIDI Engine & Synthesizers | [`src/future_audio/IMidiSubsystem.h`](file:///e:/SonaraDaw/src/future_audio/IMidiSubsystem.h), [`src/domain/MidiNote.h`](file:///e:/SonaraDaw/src/domain/MidiNote.h) | Hardware MIDI device enumeration, lock-free MIDI FIFO, virtual subtractive synth, sampler engine |
| **`project-core`** | Domain & Persistence | [`src/domain/`](file:///e:/SonaraDaw/src/domain), [`src/infrastructure/ProjectRepository.cpp`](file:///e:/SonaraDaw/src/infrastructure/ProjectRepository.cpp) | `.arcline` JSON Schema v2, atomic file saving, undo/redo history, track & region data models, preferences |

---

## ⚡ Git Initialization & Branch Setup Guide

### 1. Working in This Repository (Branches Already Created)

All category branches are already configured in this repository. To switch between them:

```powershell
# Check current active branches
git branch

# Switch to UI development
git checkout ui

# Switch to VST3 plugin hosting
git checkout plugins

# Switch to real-time audio playback & drivers
git checkout audio-engine

# Switch to mixer summing & routing
git checkout mixer

# Switch to MIDI sequencing & instruments
git checkout midi

# Switch to project format & undo/redo
git checkout project-core

# Return to active integration branch
git checkout develop
```

### 2. How to Initialize from Scratch (`git init` Protocol)

If you clone the source tree or initialize an identical repository from scratch, run these commands:

```powershell
# 1. Initialize Git repository
git init

# 2. Configure Git identity
git config user.name "Your Name"
git config user.email "your.email@example.com"

# 3. Pull or initialize submodules (JUCE 8)
git submodule update --init --recursive

# 4. Stage and commit baseline foundation
git add .
git commit -m "feat: Initialize ARCLINE desktop foundation"

# 5. Tag Phase 1 milestone
git tag -a v0.1.0-phase1 -m "ARCLINE Phase 1 Baseline Freeze"
git branch phase-1-frozen

# 6. Create integration branch (develop)
git branch develop
git checkout develop

# 7. Create the 6 isolated category branches
git branch ui
git branch plugins
git branch audio-engine
git branch mixer
git branch midi
git branch project-core
```

### 3. Merging Category Work Back into `develop`

When a feature in a category branch is completed and verified:

```powershell
# 1. Switch to develop
git checkout develop

# 2. Merge your category branch
git merge plugins

# 3. Compile and verify automated test suite
.\build\bin\ArclineTests.exe
```

---

## 🌟 Phase 1 Foundation — Completed & Verified

The baseline foundation delivers a production-grade application shell, Logic Pro X dark theme, multitrack arrangement workspace, centralized command registry, durable versioned project persistence, and 10 passing test suites:

### 1. Logic Pro X Dark Design System
- **Studio Palette:** Studio surfaces (`#191B1F`, `#24262B`, `#2E3138`), subtle 1px panel borders (`#33373F`), and recessed LCD panel (`#14171A`).
- **Custom Look & Feel:** Faders with calibrated dB scale ticks, rotary pan pots, transport buttons, calibration meters, and LCD panels.
- **Strictly Honest Controls:** Controls targeting future audio stages are labeled cleanly with their designated milestone.

### 2. Main Arrangement Timeline & Transport
- **Top Header Bar & Transport:**
  - Recessed center LCD display showing **Bar / Beat / Div / Tick**, **Tempo (BPM)**, and **Key / Time Signature**.
  - Transport controls: Rewind, Fast Forward, Stop, Play, Record, Cycle, Metronome.
  - Tool palette: Pointer, Scissors (split tool), Pencil, Eraser.
  - Dock toggles: Inspector (`I`), Mixer (`X`), Piano Roll (`P`), Audio Editor (`E`), Sound Library (`Y`).
  - Master volume slider.
- **Musical Ruler & Cycle Range:**
  - Horizontal bar and beat divisions.
  - Yellow draggable Cycle locator with start/end handles and scrubbing.
- **Track Headers & Controls:**
  - Color stripes, track type icons, Track Name.
  - Mute (`M`), Solo (`S`), Record-Arm (`R`), Input Monitoring (`I`).
  - Volume slider and Rotary Pan knob with numeric tooltips.
  - Add Audio Track (`+ Audio`) and Add Instrument Track (`+ Instrument`) buttons.
- **Timeline Lanes & Region Editing:**
  - Audio regions with rendered waveform peaks.
  - MIDI regions with rendered mini piano-roll note previews.
  - Drag-to-move with beat snapping, edge trimming, and scissors splitting (`Ctrl+T`).
  - Delete selected regions (`Delete`), duplicate regions (`Ctrl+D`).

### 3. Dual-Channel Inspector Panel
- **Region Parameters Box:** Controls for Quantize (1/16, 1/8, 1/4), Gain dB, Mute, and Looping.
- **Selected Track Channel Strip:** EQ curve thumbnail, 4 Insert slots (`[VST3 Hosting - Phase 6]`), 2 Send knobs (`[Mixer Routing - Phase 5]`), Pan knob, Level Fader with dB readout, Stereo Calibration Meter.
- **Master Stereo Output Strip:** Master bus level fader and calibration meter.

### 4. Dockable Lower Editor Panel
- **Mixer Console (`X`):** Full horizontal channel-strip mixing view for all project tracks plus Master output.
- **Piano Roll MIDI Editor (`P`):** Left-hand 88-key piano keyboard with pitch grid, note displays, and velocity preview.
- **Sample Audio Editor (`E`):** High-resolution waveform sample viewer with region boundary controls.

### 5. Sound Library & Browser Panel (`Y`)
- Categorized sound and preset library (Acoustic Drums, Synthesizers, Electric Pianos, Bass, Guitars, Orchestral) with one-click track creation.

### 6. Application Shell, Dialogs & Command Palette
- **Start Studio Screen (`Alt+2`):** Project chooser with template selection (Empty Project, Multi-track Band, Electronic Producer, Vocal Recording), recent projects list with timestamp and path display, and new project modal.
- **Preferences Dialog (`Ctrl+,`):** Audio device settings, buffer size, sample rate, theme density, autosave frequency.
- **Command Palette (`Ctrl+K`):** Fuzzy searchable command registry with category badges, keyboard shortcuts, and instant execution.

### 7. Durable Project Format (`.arcline` Schema v2)
- Project files stored as `.arcline` (Format: `ARCLINE_PROJECT`, Schema v2) with atomic write-and-replace to prevent corruption.
- Transparent backward compatibility loading legacy `.sonora` files.
- Forward rejection protecting user data if a newer schema is encountered.
- 100% data preservation roundtrip for waveforms and MIDI note events.

---

## 🛠️ Technology Stack & Prerequisites

- **Language:** C++20 (`std::format`, concepts, `string_view`, modern ranges)
- **Framework:** JUCE 8.0.6 (Modular CMake)
- **Build System:** CMake 3.22+ and Ninja
- **Supported Compilers:**
  - Windows: MSVC 2022 (Visual Studio 17.10+ / Build Tools)
  - macOS: Apple Clang (Xcode 14+)
  - Linux: GCC 12+ or Clang 15+

### Linux Dependencies
On Ubuntu/Debian, install the required audio and GUI development libraries:
```bash
sudo apt-get update
sudo apt-get install -y ninja-build libasound2-dev libjack-jackd2-dev \
  libfreetype6-dev libx11-dev libxcomposite-dev libxcursor-dev \
  libxinerama-dev libxrandr-dev libxrender-dev libglu1-mesa-dev mesa-common-dev
```

---

## 🚀 Building & Running the Application

### Windows (Quick Start)

Run the automated native build script:
```cmd
build_native.cmd
```

Or configure and build manually with CMake and Ninja:
```cmd
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DARCLINE_BUILD_TESTS=ON
cmake --build build --config Release
```

### Launch the Desktop Application
```cmd
.\build\Arcline_artefacts\Release\ARCLINE.exe
```

---

## 🧪 Running the Automated Test Suite

ARCLINE includes 10 comprehensive unit and integration test suites:

```cmd
.\build\bin\ArclineTests.exe
```

Or via CTest:
```cmd
ctest --test-dir build --output-on-failure
```

### Test Suites Included
1. `ProjectTests` — Domain model invariants, tempo, time signatures, tracks, and regions.
2. `UndoHistoryTests` — Multi-level undo/redo transactions and action grouping.
3. `PreferencesTests` — Audio driver preferences, sample rate, buffer size persistence.
4. `RecentProjectsTests` — Recent projects list serialization and MRU order.
5. `CommandRegistryTests` — Centralized command lookup, execution, and shortcuts.
6. `NavigationTests` — Workspace switching (Start Studio, Arrangement).
7. `LocalizationTests` — Multi-language string bundle lookups and fallbacks.
8. `LayoutVerificationTests` — Responsive layout bounds, header bar, and dock docking.
9. `ProjectFormatDurabilityTests` — Schema v2 serialization, atomic saving, and corruption recovery.
10. `ProjectLifecycleTests` — End-to-end project creation, file save, and disk reload.

---

## 📊 Cross-Platform Verification Matrix

| Component | Windows 10/11 x64 | macOS (Apple Silicon & Intel) | Debian / Ubuntu Linux | Arch Linux |
| :--- | :--- | :--- | :--- | :--- |
| **Compilation & Link** | **PASSED** (MSVC 2022 v14.44) | Architected & Preset Ready | Architected & Preset Ready | Architected & Preset Ready |
| **Automated Tests** | **100% PASSED** (10 suites) | Configured in CTest | Configured in CTest | Configured in CTest |
| **Desktop GUI Launch** | **PASSED** (Verified Native) | Configured (JUCE 8) | Configured (JUCE 8) | Configured (JUCE 8) |
| **Dark Theme Aesthetics** | **PASSED** (Logic Pro X match) | Shared LookAndFeel | Shared LookAndFeel | Shared LookAndFeel |
| **Timeline & Waveforms** | **PASSED** (Peak cache render) | Shared Domain/UI | Shared Domain/UI | Shared Domain/UI |
| **MIDI Note Previews** | **PASSED** (Event rendering) | Shared Domain/UI | Shared Domain/UI | Shared Domain/UI |
| **Project Durability** | **PASSED** (Atomic Schema v2) | Shared POSIX / Win32 | Shared POSIX / Win32 | Shared POSIX / Win32 |
| **Packaging Foundation** | **Inno Setup (`arcline.iss`)** | DMG (`build_dmg.sh`) | Deb (`debian/control`) | Arch (`PKGBUILD`) |

---

## 🎹 Keyboard Shortcuts Cheat Sheet

| Shortcut | Action |
| :--- | :--- |
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

## 🏛️ Architectural Invariants & Layer Boundaries

ARCLINE enforces strict decoupling between four distinct architectural layers:

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

1. **UI Layer Boundary:** UI components never access audio threads or raw memory buffers. UI events dispatch exclusively through `sonora::core::CommandRegistry`.
2. **Audio Engine Boundary:** Real-time processing callbacks are strictly real-time safe (zero allocations, zero mutexes, zero file I/O).
3. **Project Persistence Boundary:** File saving is atomic (write to `.tmp` file and rename) to guarantee zero file corruption.

---

## 🤝 Complete Contributor Guide

Whether you are a human audio engineer, a C++ systems programmer, or an autonomous AI coding agent, this guide outlines the end-to-end workflow for contributing to ARCLINE.

### 11.1 Quick-Start: Clone, Submodules & First Build

#### 1. Clone the Repository
Clone the repository with recursive submodules to ensure JUCE 8 is fetched:
```bash
git clone https://github.com/sivamsp5820/Arcline.git
cd Arcline
git submodule update --init --recursive
```

#### 2. Native Compilation by Operating System

* **Windows 10/11 (MSVC 2022 + Ninja):**
  Open the *Visual Studio x64 Native Tools Command Prompt* and execute:
  ```powershell
  # Configure Release build with automated tests enabled
  cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DARCLINE_BUILD_TESTS=ON

  # Compile DAW executable and test runner
  cmake --build build --config Release

  # Run test suite
  ctest --test-dir build --output-on-failure
  ```
  *(Or execute `.\build_native.cmd` directly from the project root).*

* **macOS 12+ (Apple Silicon & Intel):**
  Ensure Xcode and command line tools are installed:
  ```bash
  cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DARCLINE_BUILD_TESTS=ON
  cmake --build build --config Release
  ctest --test-dir build --output-on-failure
  ```

* **Ubuntu / Debian Linux:**
  Install required audio development packages:
  ```bash
  sudo apt-get update
  sudo apt-get install -y build-essential cmake ninja-build \
      libasound2-dev libjack-jackd2-dev libfreetype-dev \
      libfontconfig1-dev libx11-dev libxrandr-dev libxinerama-dev \
      libxcursor-dev libgl1-mesa-dev
  cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DARCLINE_BUILD_TESTS=ON
  cmake --build build --config Release
  ctest --test-dir build --output-on-failure
  ```

* **Arch Linux:**
  ```bash
  sudo pacman -S --needed base-devel cmake ninja alsa-lib jack2 freetype2 libx11 libxrandr libxinerama libxcursor
  cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DARCLINE_BUILD_TESTS=ON
  cmake --build build --config Release
  ctest --test-dir build --output-on-failure
  ```

---

### 11.2 Finding & Claiming Tasks

To prevent overlapping efforts across contributors, all development is tracked via structured tasks in [`docs/CONTRIBUTOR_TASKS.md`](docs/CONTRIBUTOR_TASKS.md):

1. **Browse Available Tasks:** Check [`docs/CONTRIBUTOR_TASKS.md`](docs/CONTRIBUTOR_TASKS.md) to locate tasks marked `[AVAILABLE]`.
2. **Task ID Categorization:**
   - `UI-*`: Desktop UI, Arrangement timeline, Piano roll, Inspector, Dark theme styling.
   - `AUD-*`: Real-time audio engine, hardware device negotiation, buffer processing, transport clock.
   - `MIX-*`: Summing engine, bus routing graph, faders, equal-power pan, ballistic metering.
   - `MID-*`: Hardware MIDI discovery, MIDI file sequencing, virtual synth engines.
   - `VST-*`: Out-of-process VST3 plugin hosting, scanner, parameter recall.
   - `PRJ-*`: Project file schema, atomic persistence, undo/redo transactions.
3. **Claim the Task:** File a GitHub issue using [`.github/ISSUE_TEMPLATE/01_feature_task.md`](.github/ISSUE_TEMPLATE/01_feature_task.md) titled:
   `[CLAIM] <TASK-ID>: <Task Title>`
   *(e.g., `[CLAIM] UI-ARR-002: Implement Scissors Tool for Audio Region Splitting`)*.

---

### 11.3 Branching Strategy & Topic Branch Workflow

ARCLINE follows an isolated Git Flow model. **Never push directly to `main` or `develop`.**

```
[main]                     <- Production release tags only (v0.1.0, v0.2.0)
  ▲
[develop]                  <- Active integration branch (All PRs target here)
  ▲
  ├── feature/<task-name>  <- Your isolated topic branch
  │
[category branches]        <- ui, audio-engine, mixer, midi, plugins, project-core
```

#### Step-by-Step Branching Protocol:

1. **Always base your branch on the latest `develop`:**
   ```bash
   git checkout develop
   git pull origin develop
   ```

2. **Create a descriptive topic branch:**
   ```bash
   # Feature branch
   git checkout -b feature/ui-piano-roll-grid

   # Bug fix branch
   git checkout -b fix/wasapi-buffer-underrun

   # Documentation branch
   git checkout -b docs/mixer-architecture-guide
   ```

3. **Category Branches:**
   If you are working on a long-term dedicated subsystem, you can also branch off or sync with the dedicated category branches:
   - `ui` — UI screens and dark theme
   - `audio-engine` — Real-time audio engine & drivers
   - `mixer` — Summing and mixing graph
   - `midi` — MIDI input and instrument engines
   - `plugins` — VST3 hosting
   - `project-core` — Serialization and domain state

---

### 11.4 Architectural Invariants & Hard Real-Time Safety Rules

All code submitted to ARCLINE must respect strict architectural boundaries:

#### 1. Hard Real-Time Audio Callback Rules (Absolute Invariants)
Inside `audioDeviceIOCallbackWithContext` or any code executed on the real-time audio thread:
* ❌ **NO dynamic memory allocations:** Never call `new`, `delete`, `malloc`, `free`, or resize `std::vector`/`std::string`. Preallocate all buffers in `prepareToPlay()`.
* ❌ **NO thread blocking / locks:** Never acquire `std::mutex`, semaphores, or condition variables. Use lock-free atomics (`std::atomic` with relaxed or acquire/release semantics) or preallocated lock-free FIFOs.
* ❌ **NO File or Network I/O:** Never read/write files or sockets on the audio thread.
* ❌ **NO UI or synchronous logging:** Never call UI components or dispatch synchronous log messages. Use atomic counters or lightweight diagnostics buffers.

#### 2. UI Layer Rules
* UI components must never access audio threads or manipulate raw audio buffer pointers.
* UI actions must dispatch through `AppContext` or `sonora::core::CommandRegistry`.
* UI timers must not clock the audio transport; musical position is driven asynchronously by `TransportController`'s sample clock.

#### 3. Domain & Persistence Rules
* Every user-initiated change to `Project`, `Track`, or `Region` must be registered with `UndoHistory` to preserve undo/redo functionality.
* Project saving must use the atomic safe-write protocol: write to a temporary file (`.tmp`) and atomically rename to avoid partial writes or corrupt project files.

---

### 11.5 Coding Standards & Commit Message Conventions

#### C++20 Standards
* Target standard: **ISO C++20** (`-std=c++20`).
* Use RAII and smart pointers (`std::unique_ptr`, `std::shared_ptr`); avoid raw ownership pointers.
* Follow modern standard library conventions (`std::span`, `std::string_view`, `<numbers>`, `<concepts>`).
* Keep headers self-contained with `#pragma once`.

#### Commit Message Format (Conventional Commits)
All commit messages must follow the Conventional Commits specification:
```text
<type>(<scope>): <short description>
```

* **Types:**
  - `feat`: A new feature or subsystem addition
  - `fix`: A bug fix or regression repair
  - `refactor`: Code change that neither fixes a bug nor adds a feature
  - `test`: Adding or correcting tests
  - `docs`: Documentation changes only
  - `build`: Changes that affect the build system or dependencies (CMake, Ninja)
  - `ci`: Changes to CI configuration or scripts

* **Examples:**
  ```text
  feat(audio): implement continuous-phase sine tone generator
  fix(prefs): serialize sampleRate and bufferSize in preferences repository
  test(clock): add loop wrap boundary validation tests
  docs(contribute): expand README contributor guide
  ```

---

### 11.6 Running Tests & Validation Checklist

Before opening a Pull Request, verify that your changes compile cleanly without warnings and pass all test suites:

```powershell
# 1. Compile project and test runner
cmake --build build --config Release

# 2. Run automated tests via CTest
ctest --test-dir build --output-on-failure

# 3. Or execute the native test runner directly:
.\build\bin\ArclineTests.exe
```

All 15 automated suites must pass:
* `ProjectTests`
* `UndoHistoryTests`
* `PreferencesTests`
* `RecentProjectsTests`
* `CommandRegistryTests`
* `NavigationTests`
* `LocalizationTests`
* `LayoutVerificationTests`
* `ProjectFormatDurabilityTests`
* `ProjectLifecycleTests` (including Phase 1 + 2 integrated audio session)
* `AudioEngineLifecycleTests`
* `AudioClockTests`
* `TestToneSignalTests`
* `MetronomeTests`
* `DeviceSettingsTests`

---

### 11.7 Submitting a Pull Request

1. **Push your branch to GitHub:**
   ```bash
   git push -u origin feature/<task-name>
   ```
2. **Open a Pull Request:**
   Navigate to [https://github.com/sivamsp5820/Arcline](https://github.com/sivamsp5820/Arcline) and open a Pull Request.
   * **Base branch:** `develop` *(Never target `main` directly)*.
   * **Compare branch:** `feature/<task-name>`.
3. **Fill Out the PR Template:**
   Use the preconfigured [`.github/pull_request_template.md`](.github/pull_request_template.md):
   * Link the Task ID (e.g. `Resolves #42` or `Addresses UI-ARR-001`).
   * Describe your architectural changes.
   * Attach test verification logs showing 100% pass rate.
   * Confirm compliance with the real-time audio safety checklist if modifying audio code.
4. **Code Review & Merge:**
   Once reviewed and verified, your branch will be merged into `develop`.

---

## 📄 License

Proprietary — Copyright (c) 2026 ARCLINE Audio. All rights reserved.  
*"Shape Your Sound."*
