<!-- 
  ARCLINE PULL REQUEST TEMPLATE
  Please complete all sections before submitting your PR.
  Incomplete pull requests or those lacking verification evidence will be rejected.
-->

## 1. Summary
<!-- A clear, concise explanation of what this pull request implements and why. -->


## 2. Task
- **Task ID:** `<!-- e.g. AUD-001, UI-004, etc. from docs/CONTRIBUTOR_TASKS.md -->`
- **Branch Name:** `<!-- e.g. feature/audio-devices -->`
- **Target Branch:** `develop`

## 3. Scope & Exclusions
- **In-Scope Changes:**
  - 
- **Intentionally Excluded:**
  - 

## 4. Files Changed
- **Source Files:**
  - 
- **Test Files:**
  - 
- **Documentation:**
  - 

## 5. Verification & Build Commands
<!-- Paste the exact terminal commands executed and test runner output. -->
```powershell
# Exact build command:
cmake --build build --config Release

# Exact test command:
.\build\bin\ArclineTests.exe
```

**Test Execution Results:**
<!-- Paste the summary output showing 100% test pass. -->
```
```

## 6. Visual Evidence (UI Changes)
<!-- Required for UI tasks. Attach PNG screenshots or GIFs demonstrating the changes in the dark theme. -->
- [ ] Visual changes verified against Logic Pro X dark theme guidelines.
<!-- Attach image links here -->

## 7. Audio Evidence (Audio Engine / DSP Changes)
<!-- Required for audio engine, transport, MIDI, mixer, or VST3 tasks. -->
- [ ] **Hard Real-Time Safe:** Verified ZERO heap allocations (`malloc`/`new`) in the audio rendering callback (`processBlock`).
- [ ] **Zero Blocking:** Verified zero mutexes, zero file I/O, and zero locks on audio thread.
- [ ] **Signal Check:** Described hardware/loopback configuration or audio test fixture used:
<!-- Describe test setup / signal check results -->

## 8. Platform Status Matrix

| Platform | Build Status | Test Status | UI / Runtime Status |
| :--- | :--- | :--- | :--- |
| **Windows 10/11 x64** | `<!-- Built / Not Tested -->` | `<!-- Passed / Not Run -->` | `<!-- Verified Native / Not Run -->` |
| **macOS (Apple Silicon / Intel)** | `<!-- Built / Not Tested -->` | `<!-- Passed / Not Run -->` | `<!-- Verified / Headless Only -->` |
| **Ubuntu / Debian Linux** | `<!-- Built / Not Tested -->` | `<!-- Passed / Not Run -->` | `<!-- Verified / Headless Only -->` |
| **Arch Linux** | `<!-- Built / Not Tested -->` | `<!-- Passed / Not Run -->` | `<!-- Verified / Headless Only -->` |

## 9. Known Limitations & Follow-Up Tasks
<!-- Report any unfinished behavior, edge cases, or follow-up issues required. -->
- None / 

---
### Checklist
- [ ] My branch was branched from the latest `develop` commit.
- [ ] I followed the [Contributing Guidelines](file:///e:/SonaraDaw/CONTRIBUTING.md) and task scope in [Contributor Tasks](file:///e:/SonaraDaw/docs/CONTRIBUTOR_TASKS.md).
- [ ] Central files were not modified without prior RFC or authorization.
- [ ] All automated tests pass without errors or memory leaks.
