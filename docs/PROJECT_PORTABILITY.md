# ARCLINE Project Portability and Media Management Policy

## Document Metadata
- **Status:** Active Standard
- **Scope:** Project Structure, Media References, Path Normalization, Cross-Platform Relinking
- **Product:** ARCLINE Desktop Digital Audio Workstation ("Shape Your Sound.")
- **Target OS:** Windows 10/11, macOS (Apple Silicon / Intel), Linux (Debian / Arch)

---

## 1. Core Principles

Project files and audio media created in ARCLINE must be effortlessly shareable across machines, operating systems, and directory locations. A project folder copied from a Windows developer workstation to a macOS laptop or a Linux audio workstation must open immediately without manual path reconfiguration.

To achieve this, all ARCLINE project handling code, serialization logic, and media importers adhere to four immutable rules:

1. **Zero Hardcoded Absolute Paths:** Project files (`.arcline`) must never mandate developer-specific drive letters (`C:\Users\...`) or machine-specific home folders (`/home/username/...`, `/Users/developer/...`).
2. **POSIX Path Normalization:** All serialized file paths within project metadata must strictly use forward slashes (`/`). Backslashes (`\`) are normalized at ingestion and serialization boundaries.
3. **Self-Contained Bundle Architecture:** By default, all newly recorded or imported audio assets are copied or referenced relative to the project directory.
4. **Resilient Four-Tier Media Resolution:** If an external asset cannot be found at its relative path, the engine initiates automated heuristic discovery before prompting the user.

---

## 2. Standard Project Bundle Layout

When a new session is created or an existing project is collected for sharing, ARCLINE structures the project folder as follows:

```
MySong_Project/
├── MySong.arcline                 # Primary project descriptor (JSON format)
├── Audio Files/                   # Dedicated audio media directory
│   ├── Vocal_Lead_Take1.wav
│   ├── Kick_Drum_48k.wav
│   └── Bass_Direct.wav
├── Bounces/                       # Offline mixdowns and stems
│   └── MySong_Master_20261012.wav
├── Project File Backups/          # Timestamped auto-save snapshots
│   ├── MySong_AutoSave_001.arcline
│   └── MySong_AutoSave_002.arcline
└── Sampler Instruments/           # Instrument definitions and multisamples (Phase 3)
```

---

## 3. Media Path Storage and Resolution Policy

### 3.1 Relative Path Serialization

When referencing media in a region, the engine normalizes the path:

```json
{
  "id": "reg_vocal_lead",
  "name": "Vocal Lead Take 1",
  "type": 0,
  "startTick": 0,
  "lengthTicks": 3840,
  "audioFilePath": "Audio Files/Vocal_Lead_Take1.wav"
}
```

If an asset originates outside the project bundle (e.g., an external sound library) and the user has not checked "Copy external media to project folder", the path is stored relative to the project directory if on the same volume, or with a normalized portable URI identifier.

### 3.2 Four-Tier Asset Resolution Cascade

When opening a `.arcline` project, the media locator executes the following sequential search:

```
+-------------------------------------------------------------+
| Step 1: Relative Path Check                                 |
| - Combine projectDir + region.audioFilePath                 |
| - Verify physical file existence                            |
+-------------------------------------------------------------+
                              | Not Found
                              v
+-------------------------------------------------------------+
| Step 2: Project-Local "Audio Files" Heuristic               |
| - Extract filename: e.g. "Vocal_Lead_Take1.wav"             |
| - Check projectDir + "/Audio Files/" + filename             |
+-------------------------------------------------------------+
                              | Not Found
                              v
+-------------------------------------------------------------+
| Step 3: Global Sound Library Directories                    |
| - Query paths configured in Preferences -> Library Search   |
| - Search recursive sound library roots                      |
+-------------------------------------------------------------+
                              | Not Found
                              v
+-------------------------------------------------------------+
| Step 4: Missing Media Relink Modal                          |
| - Present missing asset list with duration/size metadata    |
| - Provide interactive "Locate Folder..." picker             |
| - Batch-update all missing references upon relocation       |
+-------------------------------------------------------------+
```

---

## 4. Cross-Platform Filesystem Conventions

Different host operating systems enforce different filesystem semantics. ARCLINE mandates the following safeguards:

### 4.1 Case-Sensitivity Protection
- **Issue:** Windows (NTFS) and macOS (APFS by default) are case-insensitive, whereas Linux filesystems (ext4, btrfs) are case-sensitive (`Vocal.wav` vs `vocal.wav`).
- **Policy:** The engine enforces exact case matching when writing filenames. When resolving on case-sensitive filesystems, the locator performs a fallback case-insensitive scan if exact match fails.

### 4.2 Prohibited Filename Characters
- **Forbidden Characters:** Across all operating systems, filenames generated by ARCLINE (tracks, stems, takes, bounces) must never contain:
  `\` `/` `:` `*` `?` `"` `<` `>` `|`
- **Sanitization Rule:** Any track or marker name containing these characters is automatically sanitized with an underscore (`_`) when generating media filenames on disk.

### 4.3 Drive Letters and Mount Points
- Drive letter prefixes (`C:`, `D:`) are strictly forbidden inside relative `audioFilePath` strings.
- Projects stored on cloud-synced folders (Dropbox, iCloud Drive, OneDrive) or removable external NVMe drives resolve identically regardless of local mount paths.

---

## 5. Verification and Compliance Checklist

Before committing any project persistence code or media processing features:
- [x] All serialized file paths use forward slashes (`/`).
- [x] Relative paths are calculated relative to `project.getFilePath()`.
- [x] Project files load successfully even if the parent directory has been renamed.
- [x] Project files load successfully if transferred to another operating system.
- [x] Missing media produces a clean non-fatal warning and preserves session state without crashing or dropping regions.
