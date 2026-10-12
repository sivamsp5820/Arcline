# ARCLINE Architecture: Input Abstraction & Command Boundary
**Tagline:** "Shape Your Sound."  
**Document ID:** `DOC-ARCH-002`  
**Status:** Approved & Frozen for Phase 1  
**Applies To:** Core, UI, Domain, Audio Boundary

---

## 1. Executive Summary & Purpose

The ARCLINE application strictly separates physical user input (keyboard presses, mouse gestures, MIDI input in future phases, and OS menu invocations) from application business logic and audio execution. Direct coupling between UI event listeners and audio engine or domain mutations creates fragile, untestable architectures that impede customization, accessibility, localization, and headless testing.

The **Input-Command Boundary** formalizes this decoupling through a centralized, searchable, and migratable Command pattern implemented in `sonora::core::CommandRegistry`.

---

## 2. Architectural Boundary & Data Flow

```
                      [ User Input Sources ]
     +--------------------------+--------------------------+
     |                          |                          |
[ Hardware Keys ]       [ Mouse / UI Gestures ]    [ Command Palette ]
  (KeyPress events)       (Buttons, Menu items)      (Fuzzy Search Modal)
     |                          |                          |
     +--------------------------+--------------------------+
                                |
                                v
               +----------------------------------+
               |    sonora::core::CommandRegistry  |
               |  - ID Lookup & Alias Migration   |
               |  - Shortcut Mapping & Overrides  |
               |  - canExecute() Preconditions     |
               +----------------------------------+
                                |
                   Dispatches to Command Handler
                                |
        +-----------------------+-----------------------+
        |                                               |
        v                                               v
[ Domain Mutation ]                           [ Workspace Navigation ]
  - Add Track / Delete Region                   - Switch to Arrangement
  - Undo / Redo Transactions                    - Toggle Inspector / Library
  - Save / Load Project                         - Open Settings / Palette
        |                                               |
        +-----------------------+-----------------------+
                                |
                                v
                  [ Phase 2 Real-Time Engine ]
                  - Transport Start / Stop / Scrub
                  - Audio Buffer Processing (Headless)
```

---

## 3. Core Abstractions

### 3.1 `core::Command` Specification

Every user-triggerable action in ARCLINE is defined as a `core::Command` struct:

```cpp
namespace sonora::core {

struct Command {
    std::string id;                  // Unique canonical string key (e.g. "transport.play_stop")
    std::string title;               // Localized human-readable label
    std::string category;            // Categorization group ("Transport", "File", "View", "Edit")
    std::string shortcutText;        // Default display accelerator (e.g. "Space", "Ctrl+S")
    std::function<void()> execute;   // Invocation lambda or member bind
    std::function<bool()> isEnabled; // Dynamic predicate evaluating preconditions

    bool canExecute() const {
        return execute && (!isEnabled || isEnabled());
    }
};

} // namespace sonora::core
```

### 3.2 Canonical Command Identifiers (Phase 1 Catalog)

| Canonical Command ID | Category | Default Shortcut | Primary Action |
| :--- | :--- | :--- | :--- |
| `file.new` | File | `Ctrl+N` / `Cmd+N` | Spawns New Project modal dialog |
| `file.open` | File | `Ctrl+O` / `Cmd+O` | Spawns native project file open dialog |
| `file.save` | File | `Ctrl+S` / `Cmd+S` | Performs safe atomic save to disk |
| `edit.undo` | Edit | `Ctrl+Z` / `Cmd+Z` | Pops Undo stack and restores previous project snapshot |
| `edit.redo` | Edit | `Ctrl+Shift+Z` / `Ctrl+Y` | Pops Redo stack and reapplies command |
| `view.command_palette` | View | `Ctrl+K` / `Cmd+K` | Opens fuzzy search command palette overlay |
| `view.toggle_inspector` | View | `I` | Toggles left-side Inspector panel visibility |
| `view.toggle_library` | View | `Y` | Toggles right-side Library panel visibility |
| `view.toggle_mixer` | View | `X` | Toggles bottom Lower Editor Mixer view |
| `view.toggle_pianoroll` | View | `P` | Toggles bottom Lower Editor Piano Roll view |
| `view.toggle_audio_editor` | View | `E` | Toggles bottom Lower Editor Audio Editor view |
| `view.preferences` | View | `Ctrl+,` / `Cmd+,` | Opens versioned preferences configuration dialog |
| `transport.play_pause` | Transport | `Space` | Toggles playback state (Phase 2 boundary) |
| `transport.return_to_zero` | Transport | `Enter` / `Return` | Resets playhead position to tick 0 |
| `transport.toggle_loop` | Transport | `/` | Toggles cycle loop state |
| `transport.toggle_metronome`| Transport | `C` | Toggles click metronome enabled state |

---

## 4. Shortcut Migration & User Customization

### 4.1 Legacy Identifier Migration
When ARCLINE migrated away from experimental navigation concepts, command aliases were preserved via `CommandRegistry::migrateCommandId`:

```cpp
std::string CommandRegistry::migrateCommandId(std::string_view legacyId) {
    if (legacyId == "view.musical_universe" || legacyId == "app.toggle_universe") {
        return "view.start_studio";
    }
    if (legacyId == "action.new_project") return "file.new";
    if (legacyId == "action.save_project") return "file.save";
    return std::string(legacyId);
}
```

This guarantees backward compatibility for user keymap preference files created during earlier builds.

### 4.2 Custom Shortcut Overrides
- User keybindings are saved in `Preferences::customShortcuts`.
- `CommandRegistry::getEffectiveShortcut(id)` prioritizes custom user keybindings over default hardcoded accelerators.
- `CommandRegistry::resetCustomShortcuts()` provides a 1-click restore to standard Logic Pro X workflow bindings.

---

## 5. UI Integration & Key Event Propagation

1. **Top-Level Key Dispatcher:** `MainLayoutComponent::keyPressed` intercepts keys before child components consume them.
2. **Text Field Exemption:** When an active text editor (such as track renaming or tempo input) has focus, raw alphanumeric keystrokes are passed to the text box; only global accelerators (e.g. `Ctrl+S`, `Ctrl+Z`, `Escape`) are processed at the boundary.
3. **Command Palette (`CommandPaletteOverlay`):** Allows users to execute any registered command by typing natural-language queries (e.g. typing "Mixer" or "Save"), removing dependence on memorizing keyboard shortcuts.

---

## 6. Phase 2 Real-Time Audio Boundary Contract

In Phase 2, the Command Boundary protects real-time constraints:
1. **Never block the Audio Callback:** UI commands never call audio hardware methods synchronously.
2. **Atomic / Lock-Free Ringbuffers:** Transport commands (`transport.play_pause`, `transport.seek`) post lock-free atomic transport state updates to the real-time audio thread.
3. **Deterministic Headless Execution:** Unit and integration tests can execute commands programmatically without rendering a GUI or spinning an OS window message pump.
