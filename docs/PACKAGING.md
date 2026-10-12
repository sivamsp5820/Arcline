# ARCLINE — Cross-Platform Build & Packaging Guide

> **Product:** ARCLINE Desktop Digital Audio Workstation  
> **Tagline:** "Shape Your Sound."  
> **Version:** 0.1.0-alpha (Phase 1)  

---

## 1. Platform Verification & Testing Status

In strict adherence to engineering integrity, this status matrix records the **actual verified state** of each target platform in Phase 1:

| Target Platform | Toolchain / Environment | Build Status | Automated Tests | Packaging Spec | Verification Notes |
|---|---|---|---|---|---|
| **Windows 10/11 x64** | MSVC 2022 (v14.44), Ninja 1.12, CMake 3.22+ | **PASS (100%)** | **PASS (7/7 Suites)** | Inno Setup (`arcline.iss`) | **Fully Verified Natively.** Native binary launched, GUI rendered, `.arcline` created/saved, tests passed. |
| **macOS (Apple Silicon & Intel)** | Clang 15+, Ninja, CMake 3.22+, macOS 11.0+ SDK | *Architected & Preset Ready* | *Configured in CTest* | App Bundle & DMG (`build_dmg.sh`, `Info.plist`) | Target configuration complete; verified against JUCE 8 macOS standards. Native runner not executed on current Windows host. |
| **Debian / Ubuntu Linux** | GCC 12+ / Clang 15+, Ninja, ALSA/X11 SDKs | *Architected & Preset Ready* | *Configured in CTest* | Debian package (`debian/control`, `debian/rules`) | Build configuration complete; dependencies pinned. Native runner not executed on current Windows host. |
| **Arch Linux** | GCC 13+, Ninja, ALSA/X11 SDKs | *Architected & Preset Ready* | *Configured in CTest* | Arch PKGBUILD (`packaging/arch/PKGBUILD`) | Rolling release build spec complete. Native runner not executed on current Windows host. |
| **Portable Linux (AppImage)** | AppImageTool, glibc 2.31+ | *Architected & Preset Ready* | *Configured in CTest* | AppImage AppDir (`AppRun`, `.desktop`) | Portable distribution bundle complete. Native runner not executed on current Windows host. |

> [!IMPORTANT]
> Linux and macOS platforms are fully specified with reproducible CMake presets, dependencies, and packaging manifests, but are marked as **Architected & Preset Ready** rather than verified, as builds were performed on a native Windows workstation.

---

## 2. Reproducible Build Instructions by Platform

### 2.1 Windows (Native x64)

#### Prerequisites:
- Visual Studio 2022 (with "Desktop development with C++")
- CMake 3.22 or higher
- Ninja build tool (or MSBuild)

#### Building with CMake Preset:
```cmd
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
cmake --preset windows-x64-release
cmake --build --preset windows-x64-release
ctest --preset windows-tests
```

#### Manual CMake CLI:
```cmd
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DARCLINE_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

---

### 2.2 macOS (Apple Silicon & Intel x86_64)

#### Prerequisites:
- Xcode 14+ or Command Line Tools
- CMake 3.22+ (`brew install cmake ninja`)

#### Building with CMake Preset:
```bash
cmake --preset macos-universal-release
cmake --build --preset macos-universal-release
ctest --preset macos-tests
```

---

### 2.3 Debian / Ubuntu Linux

#### Prerequisites:
```bash
sudo apt-get update
sudo apt-get install -y cmake ninja-build build-essential \
    libasound2-dev libjack-jackd2-dev libfreetype6-dev \
    libx11-dev libxinerama-dev libxrandr-dev libxcursor-dev libgl1-mesa-dev
```

#### Building with CMake Preset:
```bash
cmake --preset linux-debian-release
cmake --build --preset linux-debian-release
ctest --preset linux-tests
```

---

### 2.4 Arch Linux

#### Prerequisites:
```bash
sudo pacman -S --needed cmake ninja gcc alsa-lib freetype2 libx11 libxinerama libxrandr libxcursor mesa
```

#### Building with CMake Preset:
```bash
cmake --preset linux-arch-release
cmake --build --preset linux-arch-release
ctest --test-dir build-arch --output-on-failure
```

---

## 3. Packaging Artifacts Creation

### 3.1 Windows Installer (`.exe`)
Using Inno Setup:
```cmd
iscc packaging\windows\arcline.iss
```
Generates `Output\arcline-setup-0.1.0-windows-x64.exe` with `.arcline` project file association, desktop icon, and uninstaller.

### 3.2 Debian Package (`.deb`)
```bash
dpkg-buildpackage -us -uc -b
```
Generates `arcline_0.1.0-1_amd64.deb`.

### 3.3 Arch Linux (`PKGBUILD`)
```bash
cd packaging/arch
makepkg -si
```
Generates `arcline-0.1.0-1-x86_64.pkg.tar.zst`.

### 3.4 Portable Linux AppImage
```bash
mkdir -p AppDir/usr/bin AppDir/usr/share/applications
cp build-debian/Arcline_artefacts/Release/ARCLINE AppDir/usr/bin/arcline
cp packaging/linux-portable/arcline.desktop AppDir/usr/share/applications/
cp packaging/linux-portable/AppRun AppDir/
chmod +x AppDir/AppRun
appimagetool AppDir arcline-0.1.0-x86_64.AppImage
```

### 3.5 macOS DMG Disk Image
```bash
chmod +x packaging/macos/build_dmg.sh
./packaging/macos/build_dmg.sh
```
Generates `arcline-0.1.0-macos-universal.dmg`.

---

*ARCLINE — Shape Your Sound.*
