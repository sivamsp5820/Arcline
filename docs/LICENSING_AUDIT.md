# ARCLINE Engineering Audit: Third-Party Dependencies & Licensing
**Product:** ARCLINE ("Shape Your Sound.")  
**Document ID:** `DOC-LEGAL-001`  
**Status:** Approved & Frozen for Phase 1  
**Target Release:** 0.1.0

---

## 1. Executive Summary

A comprehensive legal and architectural audit of all dependencies linked or bundled within ARCLINE has been conducted prior to finalizing the Phase 1 application foundation. Every component has been traced to its upstream origin, verified for license compliance, and evaluated for impact on distribution across Windows, macOS, and Linux.

---

## 2. Dependency Audit Matrix

| Dependency / Component | Version | Bundled / Linked Method | Primary License | Upstream Origin | Distribution Compatibility |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **JUCE Framework** | 8.0.6 | Submodule / CMake FetchContent (`libs/JUCE`) | Dual: AGPLv3 or Commercial (PACE / Raw Material Software) | [github.com/juce-framework/JUCE](https://github.com/juce-framework/JUCE) | **Compatible.** Requires clean dual-license strategy. |
| **HarfBuzz** | Bundled with JUCE 8 | `juce_graphics_Harfbuzz.cpp` | Old MIT License (Permissive) | HarfBuzz Project | **Compatible.** Fully permissive; notice retained in binary about box. |
| **SheenBidi** | Bundled with JUCE 8 | `juce_graphics_Sheenbidi.c` | Apache License 2.0 | SheenBidi Project | **Compatible.** Permissive with attribution. |
| **zlib** | Bundled with JUCE 8 | `juce_core` archive subsystem | zlib License (Permissive) | Jean-loup Gailly & Mark Adler | **Compatible.** Fully permissive. |
| **libpng** | Bundled with JUCE 8 | `juce_graphics` image readers | libpng License (Permissive) | PNG Development Group | **Compatible.** Fully permissive. |
| **Standard C++ Library** | C++20 | Native compiler runtime (MSVC CRT / libc++ / libstdc++) | Runtime Exception (GPLv3 with GCC Runtime Library Exception / MSVC EULA) | ISO / Vendor Toolchain | **Compatible.** Standard dynamic/static CRT distribution rules apply. |

---

## 3. Deep-Dive Analysis of Core Dependency: JUCE 8

JUCE 8 introduces revised licensing terms compared to earlier versions:
1. **AGPLv3 (GNU Affero General Public License v3.0):**
   - Free for open-source distributions.
   - If distributed under AGPLv3, the complete source code of ARCLINE (and any network-accessible services derived from it) must be licensed under AGPLv3.
   - All copyright headers and copyleft notices must be preserved.
2. **Commercial License (Indie / Pro):**
   - Permits closed-source commercial distribution without disclosing proprietary DSP algorithms, cloud sync backend keys, or custom commercial VST3 plugins.
   - Royalty-free commercial desktop deployment with appropriate active subscription / perpetual entitlement.
3. **Starter Tier:**
   - Free for developers or entities earning under $20k USD revenue annually, subject to splash screen / analytics terms unless opted out via paid tier.

---

## 4. Final Distribution License Selection

To provide maximum freedom for open-source collaboration while maintaining enterprise and commercial release viability, ARCLINE adopts a **Dual-Licensing Model**:

### 4.1 Community / Open-Source Distribution: AGPLv3
- Binary builds made freely available to the public repository are licensed under **GNU Affero General Public License v3.0**.
- Ensures that improvements to the audio engine, UI workflow, and platform backends remain open to the creative audio engineering community.

### 4.2 Commercial Desktop Distribution: ARCLINE Commercial License
- For commercial releases distributed via app stores, enterprise packaging, or packaged with proprietary third-party VST3/AU instrument libraries:
- Built against an active **JUCE Commercial License**, exempting the binary from AGPLv3 copyleft provisions.

---

## 5. Compliance Checklist & Action Items

- [x] Pinned JUCE 8.0.6 tag in `CMakeLists.txt` with fallback to local `libs/JUCE` submodule.
- [x] Verified zero GPLv3-only or viral copyleft dependencies in the core `domain` or `infrastructure` layers.
- [x] Verified zero unvetted third-party npm / python / webview dependencies in the desktop runtime (`JUCE_WEB_BROWSER=0`, `JUCE_USE_CURL=0`).
- [x] Generated standard third-party attribution catalog for inclusion in the "About ARCLINE" dialog in Phase 2.
- [x] Confirmed compatibility across all target platforms: Windows 10/11 x64, macOS 12+ (Universal Intel/Apple Silicon), Debian-based Linux, and Arch-based Linux.
