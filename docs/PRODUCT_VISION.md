# ARCLINE — Product Vision & Architectural Specification

**Product Name:** ARCLINE  
**Tagline:** "Shape Your Sound."  
**Reference Archetype:** Apple Logic Pro X Workflow, Spatial Density, and Aesthetics  
**Technology Stack:** C++20, JUCE 8, CMake, Native Desktop  
**Current Milestone:** Phase 1 (Application Shell, Dark Theme, Arrangement Workspace & Project Durability)

---

## 1. Executive Summary

**ARCLINE** is a native, professional cross-platform Digital Audio Workstation (DAW) engineered in C++20 and JUCE 8. It adheres strictly to the established visual language, structural density, editing workflows, and ergonomic control placement of **Apple Logic Pro X**, delivered through a refined, dark charcoal/graphite desktop interface.

All experimental, unconventional, or oversized web-dashboard paradigms (such as node-map "Musical Universe" concepts, floating connected graph bubbles, or neon green cards) have been permanently eliminated. ARCLINE is designed from the ground up as a mature desktop production environment for composers, recording engineers, sound designers, and producers.

---

## 2. Core Visual & Workflow Principles

1. **Logic Pro X Ergonomics & Spatial Density:**
   - Dark charcoal and graphite backgrounds (`#191B1F`, `#24262B`, `#2E3138`).
   - Compact, high-density professional top header bar and transport.
   - Recessed LCD display showing Bars/Beats/Divs/Ticks, Tempo (BPM), and Key/Time Signature.
   - Restrained studio blue (`#2D72D2`) for focus, selection, and active cycles; vivid red reserved strictly for record indicators.
   - Left-hand Inspector featuring Region Parameters and Dual Channel Strips (Selected Track + Master Stereo Out).
   - Conventional Multitrack Arrangement Timeline with horizontal musical ruler, yellow cycle locator range, track headers with Mute/Solo/Record-Arm/Monitor controls, volume faders, and rotary pan pots.
   - Dockable bottom panel hosting a full Mixer console, Piano Roll editor, and Sample Audio editor.
   - Collapsible Library panel for patch/preset browsing.

2. **Native Desktop Precision & Real-Time Safety:**
   - Pure C++20 and native JUCE desktop rendering with zero webviews, zero Electron, and zero remote CDN dependencies.
   - Full offline functionality.
   - Strict architectural separation between UI thread operations, Domain business models, and real-time audio threads.
   - Sample-accurate timing and tick-based timeline precision (960 ticks per quarter note).

3. **Honest Control States & Phase Discipline:**
   - Every UI interaction modifies persistent, validated domain models rather than merely toggling visual buttons.
   - Controls targeting future audio engine stages are clearly labeled or disabled (e.g. `[VST3 Hosting - Phase 6]`, `[Mixer Routing - Phase 5]`). No fake DSP audio processing or simulated meters are fabricated in Phase 1.
   - Schema v2 project storage (`.arcline`) serializes and deserializes track configurations, audio waveform peak caches, and MIDI note events with atomic, crash-proof disk writes.

---

## 3. Phase 1 Deliverables Summary

- **Native Desktop Window:** High-DPI responsive window with minimum 960×600 up to 4K displays.
- **Logic Pro X Dark Theme:** Centralized theme tokens (`SonoraTheme`, `SonoraLookAndFeel`) implementing dark charcoal surfaces, subtle borders, custom faders, pan knobs, buttons, and LCD displays.
- **Multitrack Arrangement Workspace:**
  - Musical ruler with bar, beat, and tick divisions.
  - Draggable Cycle range locator and playhead scrubbing.
  - Track headers with color stripes, Mute, Solo, Record-Arm, Input Monitor, Volume slider, and Pan knob.
  - Timeline lanes with waveform rendering for audio regions and note pattern previews for MIDI regions.
  - Editing operations: Region selection, drag-to-move with beat snapping, edge trimming, scissors splitting (`Ctrl+T`), duplicate (`Ctrl+D`), and deletion (`Delete`).
- **Dual-Channel Inspector:** Region parameters box, Selected Track channel strip, and Master Stereo Out strip.
- **Dockable Lower Panel:** Mixer console, Piano Roll MIDI editor, and Sample Audio waveform viewer.
- **Library / Browser Panel:** Categorized sound library with quick-insert triggers.
- **Start Studio Chooser:** Project chooser with recent projects, template choices, and new project modal.
- **Command Palette & Hotkeys:** Centralized registry with hotkeys (`Space`, `Enter`, `C`, `R`, `Ctrl+T`, `Ctrl+D`, `Delete`, `Ctrl+K`, `Ctrl+,`).
- **Durable Project Persistence:** Atomic `.arcline` project saving, transparent legacy `.sonora` loading, and defensive error reporting.
- **Automated Test Suite:** 7 automated test suites verifying domain models, preferences, command registry, navigation, lifecycle, and format durability.

---

*ARCLINE — Shape Your Sound.*
