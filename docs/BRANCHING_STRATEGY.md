# ARCLINE — Git Branching Strategy & Multi-Contributor Governance

> **Product:** ARCLINE Desktop Digital Audio Workstation  
> **Tagline:** "Shape Your Sound."  
> **Architecture:** C++20 | JUCE 8 | CMake | Ninja  
> **Document ID:** `DOC-WORKFLOW-002`  
> **Status:** Active & Enforced  

---

## 1. Architectural Philosophy

Developing a native digital audio workstation involves multiple concurrent disciplines:
1. **Desktop GUI Engineering:** JUCE components, dark theme styling, dockable layout, high-DPI rendering.
2. **Hard Real-Time Audio DSP:** Lock-free circular buffers, zero-allocation rendering loops, device drivers (WASAPI, ASIO, CoreAudio, ALSA).
3. **Domain & Persistence:** Versioned `.arcline` project persistence, undo/redo transactions, timeline models.
4. **Third-Party Integration:** Out-of-process VST3 hosting, plugin discovery, parameter automation.
5. **Cross-Platform Release Engineering:** Multi-OS toolchains, packaging (`.iss`, `.dmg`, `.deb`, `.AppImage`), CI matrices.

Without strict branch architecture and clear interface boundaries, parallel development by multiple human engineers and autonomous AI agents inevitably results in merge conflicts, broken builds, or regressions where UI code corrupts real-time audio threads.

The ARCLINE branching model provides:
- **Isolation:** Developers work in isolated, short-lived feature branches.
- **Traceability:** Every change links to a documented Task ID in [`docs/CONTRIBUTOR_TASKS.md`](file:///e:/SonaraDaw/docs/CONTRIBUTOR_TASKS.md).
- **Stability:** `main` remains permanently releasable; `develop` serves as the verified staging ground.
- **Preservation:** The Phase 1 foundation is immutably archived on `phase-1-frozen` (`v0.1.0-phase1`).

---

## 2. Repository Topology & Branch Hierarchy

```
[main] ◄──────────────────────────────────────── Release Tag (v0.1.0, v0.2.0, ...)
  ▲                                                      ▲
  │ (Scheduled Release PR)                               │
  │                                                      │
[develop] ◄── Integration Staging                        │
  ▲                                                      │
  ├─ feature/ui-piano-roll ─────────────┐ (PR to develop) │
  ├─ feature/audio-devices ─────────────┤                 │
  ├─ build/ci ──────────────────────────┤                 │
  └─ fix/memory-leak ───────────────────┘                 │
                                                          │
[phase-1-frozen] ─────────────────────────────────────────┴── Tag v0.1.0-phase1 (Immutable)
```

### Branch Responsibilities

| Branch | Lifecycle | Source Branch | Target Branch | Merge Policy |
| :--- | :--- | :--- | :--- | :--- |
| **`main`** | Permanent | `develop` | N/A | Release PR with full regression signoff |
| **`develop`** | Permanent | `main` | `main` | Fast-forward or `--no-ff` release merge |
| **`phase-1-frozen`** | Permanent (Archive) | `main` (`b2ed1c1`) | N/A | **LOCKED / READ-ONLY** |
| **`feature/*`** | Ephemeral (Task life) | `develop` | `develop` | Squash or Merge commit via reviewed PR |
| **`fix/*`** | Ephemeral | `develop` (or `main`) | `develop` (or both) | PR with regression test |
| **`refactor/*`** | Ephemeral | `develop` | `develop` | PR preserving behavior 100% |
| **`test/*`** | Ephemeral | `develop` | `develop` | PR adding test coverage |
| **`docs/*`** | Ephemeral | `develop` | `develop` | PR updating documentation |
| **`build/*`** | Ephemeral | `develop` | `develop` | PR modifying CMake, CI, or packaging |

---

## 3. Detailed Branching Lifecycle

### Step 1: Starting a Feature Branch
Always base feature branches on the latest commit of `develop`:
```powershell
# Ensure develop is up to date
git checkout develop
git pull origin develop

# Create your dedicated branch
git checkout -b feature/audio-devices
```

### Step 2: Keeping in Sync (Rebasing)
If `develop` advances while your branch is in progress, rebase regularly to keep history linear and identify conflicts early:
```powershell
git checkout feature/audio-devices
git fetch origin
git rebase origin/develop
```
If conflicts arise:
1. Resolve the conflicted markers in your code editor.
2. Verify that `ArclineTests.exe` still builds and passes.
3. Stage resolved files: `git add <file>`.
4. Continue rebase: `git rebase --continue`.

### Step 3: Submitting a Pull Request
Push your branch to your remote and open a Pull Request targeting `develop`:
```powershell
git push -u origin feature/audio-devices
```
Complete the 9 required sections in [`.github/pull_request_template.md`](file:///e:/SonaraDaw/.github/pull_request_template.md).

### Step 4: Deleting Stale Feature Branches
Once merged into `develop`, delete your local and remote feature branch to keep the repository tidy:
```powershell
git checkout develop
git pull origin develop
git branch -d feature/audio-devices
git push origin --delete feature/audio-devices
```

---

## 4. Conflict Avoidance & Central File Governance

Merge conflicts in software projects typically concentrate around a handful of shared files. ARCLINE designates strict ownership and protocols:

```
┌───────────────────────────────────────────────┐
│              High-Conflict Files              │
├────────────────────────┬──────────────────────┤
│ CMakeLists.txt         │ Build Team           │
│ src/app/AppContext.h   │ Architecture Leads   │
│ src/core/CoreDefines.h │ Architecture Leads   │
│ src/ui/theme/*         │ UI Theme Team        │
│ ProjectRepository.cpp  │ Project System Team  │
└────────────────────────┴──────────────────────┘
```

### Anti-Conflict Guidelines
1. **No Drive-By Edits:** Do not edit `CMakeLists.txt` or `AppContext.h` unless your claimed task explicitly lists them under **Files Expected to Change**.
2. **Interface RFC Protocol:** If your feature requires exposing a new service in `AppContext.h` or adding a constant in `CoreDefines.h`, submit an **Interface RFC issue** (`.github/ISSUE_TEMPLATE/03_interface_contract_rfc.md`) before writing implementation code.
3. **No Blanket Formatting Changes:** Do not run whole-file formatters (e.g. clang-format) across files you did not modify. Format only your newly added lines.

---

## 5. Repository Administrator Guide: Branch Protection Rules

When hosting the ARCLINE repository on GitHub or GitLab, project administrators must configure the following branch protection settings:

### Protection Rules for `main`
- **Branch Name Pattern:** `main`
- [x] **Require a pull request before merging**
  - [x] Require approvals: 2
  - [x] Dismiss stale pull request approvals when new commits are pushed
- [x] **Require status checks to pass before merging**
  - [x] Require branches to be up to date before merging
  - Required checks:
    - `Build & Test (windows-latest)`
    - `Build & Test (ubuntu-latest)`
    - `Build & Test (macos-latest)`
- [x] **Do not allow bypassing the above settings**
- [x] **Lock branch:** Allow read-only access (direct pushes strictly blocked).

### Protection Rules for `develop`
- **Branch Name Pattern:** `develop`
- [x] **Require a pull request before merging**
  - [x] Require approvals: 1
- [x] **Require status checks to pass before merging**
  - Required checks:
    - `Build & Test (windows-latest)`
    - `Build & Test (ubuntu-latest)`
    - `Build & Test (macos-latest)`
- [x] **Do not allow force pushes**
- [x] **Do not allow deletions**

---

*ARCLINE — Shape Your Sound.*
