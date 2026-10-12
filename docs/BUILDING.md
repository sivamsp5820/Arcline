# ARCLINE — Build and Setup Guide

> **Product:** ARCLINE Desktop Digital Audio Workstation  
> **Tagline:** "Shape Your Sound."  
> **Version:** 0.1.0-alpha  

---

## 1. System Requirements

- **C++ Standard:** C++20 compliant compiler:
  - MSVC 2022 (v14.34+ / v14.44 recommended) on Windows x64
  - Apple Clang 14+ on macOS (Apple Silicon & Intel)
  - GCC 12+ or Clang 15+ on Debian, Ubuntu, and Arch Linux
- **Build System:** CMake 3.22+ and Ninja
- **Framework:** JUCE 8.0.6 (modular submodule under `libs/JUCE` or fetched automatically via CMake FetchContent)

---

## 2. Windows Native Build (Verified Host)

### Recommended:
Run `build_native.cmd`:
```cmd
build_native.cmd
```

### Manual Configuration:
```cmd
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DARCLINE_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

### Executable Output:
- App: `build\Arcline_artefacts\Release\ARCLINE.exe`
- Tests: `build\bin\ArclineTests.exe`

---

## 3. macOS Build (Architected & Preset Ready)

```bash
cmake --preset macos-universal-release
cmake --build --preset macos-universal-release
ctest --preset macos-tests
```
Bundle: `build-macos/Arcline_artefacts/Release/ARCLINE.app`

---

## 4. Linux Builds (Architected & Preset Ready)

### Debian / Ubuntu:
```bash
sudo apt-get update && sudo apt-get install -y cmake ninja-build build-essential \
    libasound2-dev libjack-jackd2-dev libfreetype6-dev libx11-dev libxinerama-dev libxrandr-dev libxcursor-dev libgl1-mesa-dev
cmake --preset linux-debian-release
cmake --build --preset linux-debian-release
ctest --preset linux-tests
```

### Arch Linux:
```bash
sudo pacman -S --needed cmake ninja gcc alsa-lib freetype2 libx11 libxinerama libxrandr libxcursor mesa
cmake --preset linux-arch-release
cmake --build --preset linux-arch-release
ctest --test-dir build-arch --output-on-failure
```

---

*ARCLINE — Shape Your Sound.*
