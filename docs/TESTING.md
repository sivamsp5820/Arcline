# ARCLINE — Automated Test Suite & Verification Strategy

> **Product:** ARCLINE Desktop Digital Audio Workstation  
> **Tagline:** "Shape Your Sound."  
> **Test Executable:** `build/bin/ArclineTests.exe`  

---

## 1. Testing Architecture

ARCLINE employs a comprehensive, multi-layer testing strategy:
1. **Domain Unit Tests (`tests/unit/`):** Pure C++20 tests exercising business logic, track and region manipulation, sample rate/tempo constraints, preferences, command registry, and navigation.
2. **Project Format Durability Tests (`tests/unit/ProjectFormatDurabilityTests.cpp`):** Fault injection testing for malformed JSON, missing/corrupt metadata, unsupported future schemas, backward compatibility, write failure handling, and full roundtrip data preservation.
3. **Integration Tests (`tests/integration/ProjectLifecycleTests.cpp`):** End-to-end project lifecycle validation (disk creation in temp directory, atomic write-then-rename, roundtrip waveform peak cache and MIDI note restoration, recent projects update).
4. **Native Shell Launch Verification:** Headless and native GUI invocation tests validating clean startup, window creation, structured logging to `%APPDATA%\Arcline\arcline_debug.log`, and clean shutdown.

---

## 2. Running Automated Tests

### Via CTest:
```cmd
ctest --test-dir build --output-on-failure
```

### Direct Invocation:
```cmd
.\build\bin\ArclineTests.exe
```

---

## 3. Test Suite Coverage (7 Test Suites)

| Suite Name | Scope & Scenarios Verified | Status |
|---|---|---|
| **1. ProjectTests** | Default tempo (120 BPM), tempo boundaries (20–999 BPM), time signatures (4/4, 3/4), tracks, audio regions with waveform peaks, MIDI regions with note events | **PASS** |
| **2. PreferencesTests** | Default theme, sample rate, buffer size, JSON serialization, fallback defaults on empty/corrupt files | **PASS** |
| **3. RecentProjectsTests** | MRU ordering, max recent count pruning, deduplication, path removal | **PASS** |
| **4. CommandRegistryTests** | Command registration, duplicate replacement, fuzzy search filtering, canExecute availability, execution callbacks | **PASS** |
| **5. NavigationTests** | Initial workspace (StartStudio), navigation to Arrangement, navigation history stack, back navigation | **PASS** |
| **6. ProjectFormatDurabilityTests** | Nonexistent/empty path rejection, malformed JSON rejection, missing format/schema rejection, future schema rejection (e.g. 999), invalid tempo/time sig, legacy `.sonora` backward compatibility, 200 waveform peak pairs and 16 MIDI notes data preservation, unwritable path error reporting | **PASS** |
| **7. ProjectLifecycleTests** | Full disk integration test: save project to `%APPDATA%\Arcline\TestWorkspace`, atomic `.tmp` replace, load from disk, verify bit-exact tracks, audio peaks, and MIDI notes | **PASS** |

---

*ARCLINE — Shape Your Sound.*
