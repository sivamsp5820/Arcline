# ARCLINE Feature Maturity Matrix
**Product:** ARCLINE ("Shape Your Sound.")  
**Document ID:** `DOC-ROAD-002`  
**Status:** Approved & Frozen for Phase 1  
**Release Target:** Phase 1 Sign-Off (v0.1.0)

---

## 1. Classification Definitions

To maintain architectural integrity and prevent code bloat or premature complexity, every capability in ARCLINE is assigned one of three maturity classifications:

1. **Implemented & Verified (Production Baseline):**
   Fully developed, backed by unit/integration tests, zero fake stubs, documented in architecture records, and passing all CI checks.
2. **Experimental (Gated / Development Phase):**
   Functional prototypes or developer utilities subject to API iteration. Not exposed as permanent user contracts.
3. **Future (Roadmapped Phases 2–7):**
   Deliberately excluded from current phase to protect real-time audio boundaries and architectural focus.

---

## 2. Comprehensive Feature Matrix

| Feature / Subsystem | Phase Target | Maturity Status | Architectural Owner | Automated Verification |
| :--- | :---: | :---: | :--- | :--- |
| **Logic Pro X Dark Theme** | 1 | **Implemented** | `ui::theme::SonoraTheme` | Visual audit, contrast verified (WCAG AA) |
| **Custom LookAndFeel & Controls**| 1 | **Implemented** | `ui::theme::SonoraLookAndFeel` | Component rendering tests |
| **Native Window Shell & Resizing**| 1 | **Implemented** | `app::MainWindow` | `LayoutVerificationTests` (960x600 to 4K) |
| **High-DPI Display Scaling (1x–2x)**| 1 | **Implemented** | `domain::Preferences` | `PreferencesTests`, `LayoutVerificationTests` |
| **Configurable Docked Layout** | 1 | **Implemented** | `ui::components::MainLayoutComponent` | Geometry layout tests |
| **Start Studio Workspace** | 1 | **Implemented** | `ui::workspaces::StartStudioView` | Navigation & UI tests |
| **Arrangement Multitrack View** | 1 | **Implemented** | `ui::workspaces::ArrangementView` | Workspace switching integration tests |
| **Dual Channel Strip Inspector** | 1 | **Implemented** | `ui::components::InspectorPanel` | Layout clamp tests |
| **Library / Media Browser Panel**| 1 | **Implemented** | `ui::components::LibraryPanel` | Panel toggle integration tests |
| **Lower Editor Panel (Tabs)** | 1 | **Implemented** | `ui::components::LowerEditorPanel` | Mixer, Piano Roll, Audio Editor tab tests |
| **Navigation State Controller** | 1 | **Implemented** | `app::NavigationController` | `NavigationTests` (Stack history) |
| **Domain Model (Tracks, Regions)**| 1 | **Implemented** | `domain::Project`, `Track`, `Region` | `ProjectTests` |
| **Atomic Crash-Safe Persistence**| 1 | **Implemented** | `infrastructure::FileSystem` | `ProjectFormatDurabilityTests` |
| **POSIX Platform-Neutral Paths** | 1 | **Implemented** | `infrastructure::FileSystem` | Relative path tests, cross-OS serialization |
| **Versioned Preferences** | 1 | **Implemented** | `infrastructure::PreferencesRepository` | `PreferencesTests` (Corrupt recovery) |
| **Recent Projects Cache** | 1 | **Implemented** | `domain::RecentProjects` | `RecentProjectsTests` |
| **Undo / Redo Transaction Stack**| 1 | **Implemented** | `domain::UndoHistory` | `UndoHistoryTests` |
| **Command Registry & Shortcuts** | 1 | **Implemented** | `core::CommandRegistry` | `CommandRegistryTests` |
| **Legacy Shortcut Migration** | 1 | **Implemented** | `core::CommandRegistry` | Migration tests |
| **Command Palette (Ctrl+K)** | 1 | **Implemented** | `ui::dialogs::CommandPaletteOverlay` | Search and invocation tests |
| **Localization String System** | 1 | **Implemented** | `core::Localization` | `LocalizationTests` (Fallback & format) |
| **Automated Test Harness** | 1 | **Implemented** | `tests/TestRunner` | 10 test suites (100% pass) |
| | | | | |
| **Interactive Draggable Dividers**| 1 | **Experimental** | `ui::components` | Panel bounds clamped; splitter drag polish in Phase 3 |
| **External JSON Translation Bundles**| 1 | **Experimental** | `core::Localization` | Dynamic catalog loading functional; file bundle watcher in Phase 2 |
| | | | | |
| **Real Audio Device Management** | 2 | **Future (Phase 2)** | `audio::DeviceManager` | WASAPI, CoreAudio, ALSA, PipeWire backends |
| **Real-Time Audio Engine Callback**| 2 | **Future (Phase 2)** | `audio::AudioEngine` | Lock-free audio buffer rendering |
| **Sample-Accurate Playhead Transport**| 2 | **Future (Phase 2)** | `audio::Transport` | Play, Stop, Loop, Scrubbing |
| **Audio Mixer Routing & DSP Gain** | 3 | **Future (Phase 3)** | `audio::Mixer` | Volume faders, pan pots, aux buses |
| **Peak & RMS Level Metering** | 3 | **Future (Phase 3)** | `audio::Meters` | Ballistic peak hold metering |
| **Disk Streaming Audio Buffers** | 4 | **Future (Phase 4)** | `audio::DiskStreamer` | Non-blocking file I/O thread |
| **Non-Destructive Clip Editing** | 4 | **Future (Phase 4)** | `editing::AudioEditor` | Clip splitting, fades, crossfades |
| **MIDI Engine & Real-Time Recording**| 5 | **Future (Phase 5)** | `midi::MidiEngine` | Hardware MIDI in, clock sync |
| **Piano Roll Note Sequencer** | 5 | **Future (Phase 5)** | `ui::pianoroll` | Note draw, resize, velocity editing |
| **VST3 / AU / CLAP Plugin Hosting** | 6 | **Future (Phase 6)** | `plugins::PluginHost` | Out-of-process isolation, GUI embedding |
| **Offline Audio Export / Stems** | 7 | **Future (Phase 7)** | `export::AudioExporter` | Offline rendering (WAV, FLAC, MP3) |
| **Cross-Platform Installers** | 7 | **Future (Phase 7)** | `packaging::Installer` | MSI, DMG, AppImage, Arch PKGBUILD |

---

## 3. Scope Gate Rule

Under no circumstances will Phase 2-7 functionality (such as dummy audio loops or mock DSP threads) be introduced prematurely into Phase 1 source files. All real-time audio implementation begins cleanly in Phase 2 upon verification of the frozen foundation.
