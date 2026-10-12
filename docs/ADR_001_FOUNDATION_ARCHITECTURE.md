# ADR 001: Foundation Architecture, Subsystems, and Platform Boundaries

## Document Metadata
- **Status:** Accepted (Phase 1 Application Foundation Baseline)
- **Deciders:** ARCLINE Core Engineering Team
- **Date:** 2026-10-12
- **Product:** ARCLINE Desktop Digital Audio Workstation ("Shape Your Sound.")
- **Target Platforms:** Windows 10/11 x64 (verified native), macOS (Universal), Debian-based Linux, Arch-based Linux

---

## 1. Context and Problem Statement

Building a professional native digital audio workstation inspired by the ergonomic standards of Logic Pro X requires balancing long-term architectural stability with rapid desktop development. In Phase 1, the core desktop application shell, design system, project persistence, command infrastructure, undo/redo mechanics, and navigation controllers were established.

To prevent architectural drift and premature coupling before introducing real-time audio I/O in Phase 2, this Architecture Decision Record (ADR) codifies the definitive architectural choices, boundaries, licensing terms, and platform packaging strategies.

---

## 2. Decision Summary

| Domain | Decision | Rationale |
|---|---|---|
| **Framework Integration** | Pinned JUCE 8.0.6 via CMake | Native cross-platform C++20 standard; eliminates custom OS wrappers while retaining bare-metal performance. |
| **Audio-Engine Boundary** | Strict Model-View-Controller & Lock-Free Ring Buffers | Prevents UI/main-thread stalls from blocking real-time DSP; guarantees zero fake audio in Phase 1. |
| **Project File Format** | Schema-Versioned Atomic JSON (`.arcline`) | Human-readable, auditable, git-friendly, crash-resilient against partial file writes. |
| **Plugin-Hosting Strategy** | Out-of-Process Crash-Isolated Sandboxing (Phase 3/4) | Isolates 3rd-party VST3/AU crashes from the main DAW process. |
| **Audio Backends** | Windows (WASAPI / ASIO), macOS (CoreAudio), Linux (ALSA / JACK / PipeWire) | Native low-latency driver models tailored per OS. |
| **Linux Packaging** | Native `.deb` (Debian/Ubuntu), `PKGBUILD` (Arch), Standalone AppImage | Meets requirements for both Debian-based and Arch-based distribution ecosystems. |
| **Licensing** | Permissive clean-room core with JUCE compliance | Self-contained, 100% offline operation; no telemetry or unvetted external dependencies. |

---

## 3. Detailed Architectural Decisions

### 3.1 JUCE Framework Integration Strategy

- **Version & Build Configuration:** JUCE 8.0.6 is pinned and consumed via standard CMake (`CMakeLists.txt`), with optional local submodule fallback (`libs/JUCE`) or CMake `FetchContent`.
- **Targeted Module Consumption:**
  - `juce_core`: String handling, dynamic objects, file streams, time/ticks primitives.
  - `juce_data_structures`: Value trees and undoable action managers.
  - `juce_events`: Message loops, timers, dispatch threads.
  - `juce_graphics`: 2D rendering engine, vectors, font caching.
  - `juce_gui_basics`: Native windows, DocumentWindow, mouse/keyboard focus traversal, LookAndFeel.
  - `juce_gui_extra`: Enhanced file choosers and dialog overlays.
- **Explicit Exclusions:**
  - `JUCE_WEB_BROWSER=0`: Zero embedded Chromium/CEF engines to preserve tight memory footprints (<100 MB idle).
  - `JUCE_USE_CURL=0`: Eliminates external network stacks; application operates entirely offline.
  - `juce_audio_devices` / `juce_audio_processors` / `juce_dsp`: **Deferred exclusively to Phase 2** to avoid premature, half-baked mock audio callbacks.

### 3.2 Audio-Engine Boundary and Real-Time Concurrency

The boundary between the Application / UI Layer and the Audio Engine is strictly enforced:
1. **Domain Isolation:** `domain::Project`, `domain::Track`, `domain::Region`, and `domain::UndoHistory` represent pure domain models. They own no raw hardware device handles and execute zero real-time audio DSP.
2. **Phase 2 Audio Engine Boundary:**
   - The audio thread will operate at high priority / real-time priority.
   - Communication between the audio thread and the UI/Domain thread will occur exclusively across **lock-free single-producer single-consumer (SPSC) ring buffers** and atomic state flags.
   - The audio render thread will **never** allocate dynamic memory (`malloc`/`new`), acquire OS mutexes, or perform file I/O during process callbacks.
   - Phase 1 strictly forbids placeholder or fake DSP loops to ensure full architectural authenticity.

```
+-------------------------------------------------------------+
|                     ARCLINE UI Layer                        |
|  (HeaderBar, ArrangementView, Mixer, Inspector, TrackList)  |
+-------------------------------------------------------------+
                              |
                     Method Calls / Events
                              v
+-------------------------------------------------------------+
|                    ARCLINE Domain Layer                     |
|      (Project, Track, Region, UndoHistory, Preferences)     |
+-------------------------------------------------------------+
                              |
                    Phase 2 Lock-Free Queue
                              v
+-------------------------------------------------------------+
|                 Audio Engine Boundary (Phase 2)             |
|   - Real-time Audio Device Manager                          |
|   - Lock-Free SPSC Parameter / Event Queues                 |
|   - Non-allocating Processing Graph Render Callback         |
+-------------------------------------------------------------+
                              |
           Platform Low-Latency Audio Drivers
   (ASIO/WASAPI on Windows | CoreAudio on macOS | ALSA/JACK on Linux)
```

### 3.3 Project File Format and Storage Model

- **Container Specification:** Projects are saved with the `.arcline` extension (JSON format).
- **Format Identifier:** `"format": "ARCLINE_PROJECT"` (with automatic backward-compatibility fallback to `"SONORA_PROJECT"`).
- **Schema Versioning:** Fixed schema integer (`schemaVersion: 2`). Forward-compatibility guards reject project files with schema versions newer than the running binary with a user-friendly upgrade prompt.
- **Atomic Persistence:** Files are saved using write-to-temporary followed by atomic rename (`FileSystem::writeTextAtomic`). This prevents file corruption in case of unexpected termination or power loss.
- **Portability & Media Bundling:** Media references (`audioFilePath`) are stored with portable relative paths and normalized forward slashes (`/`), avoiding absolute drive letter dependencies (`C:\...`).

### 3.4 Plugin-Hosting Architecture Plan (VST3 / AU / CLAP)

Plugin hosting is scheduled for Phase 3/4. The architecture commits to:
1. **Out-of-Process Worker Architecture:**
   - To prevent crashing plugins from terminating the user's entire DAW session, 3rd-party plugins will be hosted in dedicated worker processes communicating via low-latency shared memory and IPC.
2. **Format Support Strategy:**
   - **VST3:** Universal across Windows, macOS, and Linux via Steinberg VST3 SDK.
   - **Audio Units (AU v2/v3):** Native macOS support.
   - **CLAP:** Modern open-standard plugin architecture.
3. **Scan Validation:** Dedicated plugin scanner tool executed out-of-process with timeout limits and blacklist generation for faulty binaries.

### 3.5 Platform-Specific Audio Backends

When audio engine implementation begins in Phase 2, backend device drivers will be targeted natively per operating system:
- **Windows (x64):**
  - Primary Pro Audio: **ASIO** (Steinberg low-latency hardware protocol).
  - Consumer / Fallback: **WASAPI Exclusive Mode** with automatic fallback to Shared Mode.
- **macOS (arm64 / x86_64):**
  - Universal Low-Latency: **CoreAudio** with aggregate device support and native sample-rate tracking.
- **Linux (x86_64 / arm64):**
  - Professional: **JACK Audio Connection Kit** and modern **PipeWire** (`pipewire-jack`).
  - Native Standard: **ALSA direct hardware** (Advanced Linux Sound Architecture).

### 3.6 Linux Packaging and Distribution Strategy

To satisfy the multi-distro targeting requirements:
1. **Debian / Ubuntu (.deb):**
   - Packaged with standard `dpkg-deb` / CPack Debian generator.
   - Dependencies: `libasound2-dev`, `libjack-jackd2-dev`, `libfreetype6-dev`, `libx11-dev`, `libxinerama-dev`, `libxrandr-dev`, `libxcursor-dev`, `libgl1-mesa-dev`.
2. **Arch Linux (PKGBUILD):**
   - Native Arch package built via `makepkg` (`.pkg.tar.zst`).
   - Declares official repository dependencies: `alsa-lib`, `jack2`, `freetype2`, `libx11`, `libxrandr`, `libxinerama`, `mesa`.
3. **Distro-Agnostic Portable Bundle:**
   - Standalone **AppImage** bundle incorporating bundled shared libraries and desktop entry metadata for zero-install execution across any Linux distribution.

### 3.7 Dependency, Licensing, and Offline Guarantees

1. **Self-Contained & Fully Offline:** ARCLINE requires zero Internet connectivity to start, load, edit, undo, or save sessions.
2. **JUCE Licensing:** Complies with JUCE Commercial or GNU GPL v3 terms based on deployment tier.
3. **No Unvetted Third-Party Bloat:** All UI components, vector widgets, models, and repositories are handwritten in clean C++20 standard library and standard JUCE modules.

---

## 4. Consequences

### Positive
- Predictable, maintainable codebase with clean modular boundaries.
- Ultra-low memory baseline (<100 MB) and instantaneous cold startup (<1.0 s).
- Absolute resilience against project file corruption through atomic JSON operations and schema versioning.
- Reusable domain undo/redo actions decoupled from UI widgets.

### Mitigations & Trade-offs
- Deferring audio device handling until Phase 2 ensures the foundation is rock solid, preventing technical debt in the real-time DSP pipeline.
- Out-of-process plugin hosting requires IPC engineering in Phase 3/4, but guarantees industry-grade crash immunity.
