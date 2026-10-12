# SONORA — Logic Pro X-Style Dark Design System

## 1. Visual Philosophy & Palette Reference

SONORA implements a meticulous dark design system modeled directly after **Apple Logic Pro X**. The interface is designed for 10+ hour studio sessions, emphasizing contrast clarity, minimal visual fatigue, and instant spatial recognition.

### Color Tokens

| Semantic Token | Hex Value | Logic Pro Usage |
|---|---|---|
| `background` | `#191B1F` | Deep dark charcoal for window and timeline background |
| `surface` | `#24262B` | Medium charcoal for panels, inspector, mixer surface |
| `surfaceElevated`| `#2E3138` | Headers, track headers, active editor frames |
| `surfaceHighlight`| `#383C45` | Hovered controls, divider lines, inset frames |
| `border` | `#33373F` | Subtle 1px structural panel borders |
| `accent` | `#2D72D2` | Studio Blue for primary selections, focus rings, active cycle |
| `accentMuted` | `#1D4A88` | Inactive cycle bar, subtle focus highlights |
| `textPrimary` | `#ECEEF2` | Primary labels, fader numbers, bar numbers |
| `textSecondary` | `#9096A2` | Secondary headers, unit labels, inactive states |
| `textMuted` | `#5C626E` | Inactive hints, tick marks, grid subdivisions |
| `lcdBackground` | `#14171A` | Recessed center transport display |
| `lcdText` | `#E8EAED` | Crisp transport display digits and symbols |
| `lcdSubtext` | `#70757E` | Small unit labels inside the LCD (BAR, BEAT, BPM) |
| `recordActive` | `#D9383A` | Record-enable buttons and transport recording state |
| `soloActive` | `#E5A93B` | Track solo button active amber |
| `muteActive` | `#4B5563` | Track mute button state |
| `cycleActive` | `#F5A623` | Logic Pro signature yellow cycle locator strip |
| `playhead` | `#ECEEF2` | Timeline scrubber playhead line and triangle cap |

### Track Palette

Muted, studio-grade track colors replace garish neon hues:
- **Audio Tracks:** Studio Blue (`#2E86AB`) / Oceanic Cyan (`#1B9AAA`)
- **Software Instruments / MIDI:** Royal Amber (`#D68C45`) / Forest Jade (`#3C8D65`)
- **Drums & Percussion:** Coral Ochre (`#B85D3B`)
- **Auxiliary & Busses:** Studio Violet (`#7D65A8`)

---

## 2. Typography

SONORA uses crisp native rendering with standard system studio fonts (Inter, SF Pro Display, Segoe UI, or Helvetica Neue):
- **Title / Headers:** 14px Semi-Bold, letter-spacing +0.2px.
- **Body / Labels:** 12px Regular, high legibility.
- **Micro / Subtext:** 10px Semi-Bold uppercase for channel strip markings and LCD field headers.
- **LCD Numbers:** 13px–16px Monospace for rock-solid alignment during playback.

---

## 3. Component Geometry & Layout Density

1. **Top Header & Transport:**
   - Fixed height: 48px.
   - Left: SONORA wordmark, project title, tool selector (`Pointer`, `Scissors`, `Pencil`, `Eraser`).
   - Center: Recessed LCD display (300×36px) featuring Time Position, Tempo, Signature/Key.
   - Transport controls: Rewind, Fast-Forward, Stop, Play, Record, Cycle, Metronome.
   - Right: Dock toggles (`Inspector [I]`, `Mixer [X]`, `Piano Roll [P]`, `Audio Editor [E]`, `Library [Y]`), Master volume fader.

2. **Multitrack Timeline:**
   - Ruler height: 26px, displaying Bars (bold) and Beats.
   - Yellow draggable Cycle locator band (4px top indicator with shaded bar).
   - Track Header width: 220px, featuring Track Name, Icon/Color stripe, M/S/R buttons, Volume fader, Pan knob.
   - Track Lane minimum height: 60px–80px.
   - Audio Region: Solid color header, rounded border, embedded waveform peak visualization.
   - MIDI Region: Translucent fill, embedded mini-piano roll note preview blocks.

3. **Dual-Channel Inspector:**
   - Fixed width: 240px.
   - Upper section: Region Parameters box (Quantize, Gain dB, Mute, Loop).
   - Lower section: Dual vertical channel strips (Selected Track Strip + Master Output Strip).
   - Includes EQ thumbnail, 4 Insert FX slots, 2 Send knobs, Pan, Level Fader with dB readout, Stereo Peak Meter.

4. **Lower Editor Panel:**
   - Dockable height: 260px (collapsible).
   - Tabs: `Mixer Console`, `Piano Roll`, `Audio Editor`.
   - Mixer Console: Horizontal scroll containing all project tracks + Master output.
   - Piano Roll: Left-hand 88-key vertical piano keyboard with note grid, playhead, and velocity display.
   - Audio Editor: High-zoom waveform sample viewer with region boundary trim handles.
