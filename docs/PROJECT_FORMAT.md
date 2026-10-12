# ARCLINE — Project Format & Schema Migration Specification

> **Product:** ARCLINE Desktop Digital Audio Workstation  
> **Tagline:** "Shape Your Sound."  
> **Current Format:** `.arcline` (Format Identifier: `ARCLINE_PROJECT`, Schema Version: 2)  
> **Legacy Format Support:** `.sonora` (Format Identifier: `SONORA_PROJECT`, Schema Versions: 1, 2)  

---

## 1. Overview and Design Principles

ARCLINE project files encapsulate the complete arrangement state, track definitions, region boundaries, MIDI sequences, audio waveform peak caches, and mixer configuration.

### Core Guarantees:
1. **Durability & Crash Protection:** Saving a project uses an **atomic write-and-replace** strategy. Data is written to a temporary sibling file (`<filename>.arcline.tmp`), flushed to disk, and atomically renamed over the target file. If a crash or power failure occurs mid-write, the existing project file remains 100% intact.
2. **Strict Validation:** Malformed JSON syntax, missing metadata, out-of-range tempo or sample rates, corrupt track structures, or negative region coordinates are intercepted defensively with descriptive, actionable error dialogs.
3. **Forward-Compatible Rejection:** An older version of ARCLINE will never silently corrupt or truncate a project saved by a newer version. If a project has `schemaVersion > kCurrentSchemaVersion`, loading halts immediately with a clear prompt: *"Unsupported project schema version (N). Please upgrade ARCLINE."*
4. **Transparent Backward Compatibility:** Legacy `.sonora` project files (from prototype development) load seamlessly into ARCLINE and can be saved as modern `.arcline` projects.

---

## 2. Complete Project Schema Specification (Schema Version 2)

```json
{
  "format": "ARCLINE_PROJECT",
  "schemaVersion": 2,
  "id": "proj_a1b2c3d4e5f6",
  "name": "Cinematic Skyline",
  "sampleRate": 48000.0,
  "tempoBpm": 128.0,
  "timeSigNumerator": 4,
  "timeSigDenominator": 4,
  "keySignature": "D Min",
  "createdTimestamp": 1776038400,
  "modifiedTimestamp": 1776042000,
  "playheadTick": 0,
  "cycleStartTick": 0,
  "cycleEndTick": 15360,
  "isCycleActive": true,
  "masterVolume": 0.85,
  "masterPan": 0.0,
  "tracks": [
    {
      "id": "track_lead_vocal",
      "name": "Audio 1 (Lead Vocal)",
      "type": 0,
      "volume": 0.80,
      "pan": 0.0,
      "isMuted": false,
      "isSoloed": false,
      "isArmed": false,
      "isInputMonitored": false,
      "automationMode": 0,
      "colorArgb": 4279930847,
      "height": 72,
      "regions": [
        {
          "id": "reg_vox_01",
          "name": "Main Chorus Vox",
          "type": 0,
          "startTick": 3840,
          "lengthTicks": 7680,
          "offsetTicks": 0,
          "isMuted": false,
          "isLooped": false,
          "colorArgb": 4279930847,
          "gainDb": 0.0,
          "fadeInTicks": 120,
          "fadeOutTicks": 120,
          "peaks": [
            [0.12, -0.15],
            [0.65, -0.70],
            [0.45, -0.42]
          ]
        }
      ]
    },
    {
      "id": "track_synth_keys",
      "name": "Inst 1 (Analog Pad)",
      "type": 1,
      "volume": 0.72,
      "pan": 0.25,
      "isMuted": false,
      "isSoloed": false,
      "isArmed": false,
      "isInputMonitored": false,
      "automationMode": 0,
      "colorArgb": 4284589025,
      "height": 72,
      "regions": [
        {
          "id": "reg_pad_01",
          "name": "Pad Progression",
          "type": 1,
          "startTick": 0,
          "lengthTicks": 15360,
          "offsetTicks": 0,
          "isMuted": false,
          "isLooped": true,
          "colorArgb": 4284589025,
          "gainDb": 0.0,
          "fadeInTicks": 0,
          "fadeOutTicks": 0,
          "notes": [
            {
              "pitch": 62,
              "startTick": 0,
              "lengthTicks": 3840,
              "velocity": 96
            },
            {
              "pitch": 65,
              "startTick": 0,
              "lengthTicks": 3840,
              "velocity": 90
            }
          ]
        }
      ]
    }
  ]
}
```

---

## 3. Field Constraints & Validation Rules

| Field | Type | Constraint | Error on Violation |
|---|---|---|---|
| `format` | String | Must equal `"ARCLINE_PROJECT"` or `"SONORA_PROJECT"` | *"Not a valid ARCLINE project file (unsupported format)"* |
| `schemaVersion` | Integer | $1 \le \text{version} \le 2$ | *"Unsupported project schema version (N). Please upgrade ARCLINE."* |
| `name` | String | Non-empty, non-whitespace string | *"Malformed project file: missing or empty project name."* |
| `tempoBpm` | Double | $20.0 \le \text{BPM} \le 999.0$ | *"Project tempo out of valid range (20 - 999 BPM)"* |
| `sampleRate` | Double | $8000.0 \le \text{Hz} \le 384000.0$ | *"Invalid sample rate in project"* |
| `timeSigNumerator` | Integer | $1 \le \text{num} \le 32$ | *"Project has invalid time signature"* |
| `timeSigDenominator` | Integer | Power of 2 ($1, 2, 4, 8, 16, 32$) | *"Project has invalid time signature"* |
| `tracks` | Array | Must be a valid JSON array | *"Malformed project file: 'tracks' property must be a JSON array."* |
| `track.type` | Integer | 0 (Audio), 1 (Instrument), 2 (Aux/Bus) | *"Unknown track type identifier"* |
| `region.startTick` | Integer | $\ge 0$ | *"Invalid region bounds: start < 0"* |
| `region.lengthTicks` | Integer | $> 0$ | *"Invalid region bounds: length <= 0"* |

---

## 4. Schema Migration Strategy for Future Phases

### Schema Evolution Plan:
- **Schema Version 3 (Phase 3 & 4):**
  - Adds `audioFilePath` and `sampleHash` to Audio Regions.
  - Adds `midiChannel` and `midiControllerEvents` to MIDI Regions.
- **Schema Version 4 (Phase 5):**
  - Adds `sends` array to Tracks (`targetBusId`, `levelDb`, `isPreFader`).
  - Adds `routing` graph topology metadata.
- **Schema Version 5 (Phase 6):**
  - Adds `vst3Plugins` array to Tracks (`pluginUid`, `chunkBase64`, `presetName`, `bypass`).
- **Schema Version 6 (Phase 7):**
  - Adds `automationLanes` to Tracks (`parameterId`, `points: [{tick, value, curveType}]`).

### Migration Policy:
1. **Upward Migration (Backward Compatibility):**  
   When ARCLINE v0.5.0 opens a Schema v2 project:
   - Missing fields default to safe initialized values (e.g. empty plugin chains, unity send levels).
   - Once saved by the user, the file is upgraded to Schema v5 with an explicit user prompt.
2. **Forward Migration Rejection (Downgrade Protection):**  
   If an earlier version of ARCLINE attempts to open a Schema v5 project, loading stops before altering any data:
   > *"Unsupported project schema version (5). Maximum supported version is 2. Please upgrade ARCLINE to open this project."*

---

## 5. Automated Durability Test Matrix

Automated durability and fault-injection tests are executed in `ArclineTests` via `ProjectFormatDurabilityTests.cpp`:

| Test ID | Test Scenario | Expected Outcome | Status |
|---|---|---|---|
| `DUR-01` | Non-existent file path | Fails safely with "does not exist" message | Verified |
| `DUR-02` | Empty file path | Fails safely with "empty" error | Verified |
| `DUR-03` | Corrupt/Malformed JSON syntax | Intercepted with parse error | Verified |
| `DUR-04` | Missing `format` or wrong format string | Rejects file, prevents opening | Verified |
| `DUR-05` | Zero or negative `schemaVersion` | Rejects file with actionable message | Verified |
| `DUR-06` | Future `schemaVersion` (e.g. 999) | Rejects with upgrade prompt | Verified |
| `DUR-07` | Blank/whitespace-only project name | Rejects invalid metadata | Verified |
| `DUR-08` | Extreme out-of-bounds tempo (10 BPM, 1200 BPM) | Rejects with range constraints | Verified |
| `DUR-09` | Time signature with zero denominator | Rejects invalid rhythm signature | Verified |
| `DUR-10` | Corrupted `tracks` field (string instead of array) | Rejects malformed hierarchy | Verified |
| `DUR-11` | Inverted region bounds (negative start, 0 length) | Rejects invalid timeline geometry | Verified |
| `DUR-12` | Legacy `.sonora` (Schema v1) loading | Transparently migrates to ARCLINE | Verified |
| `DUR-13` | Waveform peak cache & MIDI note roundtrip | 100% bit-exact restoration | Verified |
| `DUR-14` | Write to unwritable path | Atomic save rolls back cleanly | Verified |

---

*ARCLINE — Shape Your Sound.*
