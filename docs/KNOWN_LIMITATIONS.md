# ARCLINE Known Limitations & Architectural Boundaries
**Product:** ARCLINE ("Shape Your Sound.")  
**Document ID:** `DOC-ARCH-003`  
**Status:** Approved & Frozen for Phase 1  
**Applies To:** Release 0.1.0 (Phase 1 Baseline)

---

## 1. Overview

This document explicitly defines the boundaries and current known limitations of ARCLINE at the conclusion of Phase 1. These constraints are intentional architectural decisions designed to guarantee a rock-solid, crash-safe, and thoroughly tested desktop foundation before introducing real-time multi-threaded audio execution in Phase 2.

---

## 2. Audio & DSP Limitations (Phase 1 Boundary)

1. **No Real-Time Audio Output:**
   - Real-time audio device management (WASAPI Exclusive, CoreAudio, ALSA, JACK, PipeWire) and real-time buffer callbacks are deferred to Phase 2.
   - Pressing `Space` (Play/Pause) or transport controls modifies internal playback state and fires navigation/command events, but does not output audio samples to hardware DACs.
2. **Audio File Streaming:**
   - Audio regions reference asset paths and display peak visual representations from project cache data. Disk streaming ring buffers and multi-threaded audio decoding are introduced in Phase 4.
3. **No Audio Effects or Mixing DSP:**
   - Channel faders and pan sliders in the Inspector and Lower Editor Mixer update track domain parameters, but do not yet compute audio buffer gain or stereo panning.

---

## 3. UI & Workflow Boundaries

1. **Panel Resizing Mechanism:**
   - Inspector width (180–400px), Library width (180–450px), and Lower Editor height (160–600px) are fully configurable and persistent via the Preferences Dialog (`Ctrl+,`).
   - Interactive live mouse-drag splitters between docked panels will be introduced during mixer refinement in Phase 3.
2. **Timeline Clip Interaction:**
   - Phase 1 multitrack timeline displays tracks and colored regions with header controls (Mute, Solo, Record Arm) and playhead positioning. Advanced interactive clip dragging, edge trimming, and slip editing are scheduled for Phase 3/4.
3. **Modal Dialog Centering:**
   - Modal dialogs (New Project, Preferences, Command Palette) center dynamically on the main application window. At extreme minimum resolutions (<800px width), dialog margins clamp to window borders.

---

## 4. File System & Portability Considerations

1. **Cross-Drive Project Media (Windows):**
   - When project audio assets reside on the same drive volume as the `.arcline` project file, relative POSIX paths (`Audio/sample.wav`) are strictly enforced.
   - If an asset resides on a separate physical Windows drive letter (e.g. project on `D:` and sample on `E:`), the path cannot be expressed as a relative POSIX path. The system gracefully records the normalized POSIX absolute path (`E:/Samples/sample.wav`) and logs a portability warning. Projects bundled within self-contained project folders are 100% portable across Windows, macOS, and Linux.
2. **Crash-Safe Write Temporary Files:**
   - Safe atomic writing writes to a temporary file (`<target>.tmp`) before atomic renaming. In environments with aggressive external antivirus locking, the atomic rename routine falls back to a safe two-step remove/rename sequence.

---

## 5. Plugin Hosting Boundary

- Phase 1 does not load, scan, or instantiate third-party VST3, AU, or CLAP plugins.
- The plugin-hosting architecture, out-of-process crash isolation sandbox, and JUCE AudioPluginFormatManager integration are formally scheduled for Phase 6.
