@echo off
setlocal enabledelayedexpansion

echo ========================================================
echo    ARCLINE Phase 1 - MSVC Build and Test Pipeline
echo    "Shape Your Sound."
echo ========================================================

set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"

if not exist "%VCVARS%" (
    echo [ERROR] vcvarsall.bat not found at: %VCVARS%
    exit /b 1
)

echo [INFO] Initializing MSVC x64 Environment...
call "%VCVARS%" x64
if errorlevel 1 (
    echo [ERROR] Failed to initialize MSVC x64 environment!
    exit /b 1
)

set "PATH=E:\SonaraDaw\toolchain\bin;%PATH%"

echo [INFO] CMake version:
cmake --version
echo [INFO] Ninja version:
ninja --version
echo [INFO] MSVC compiler:
where cl.exe
cl

echo.
echo [INFO] Configuring ARCLINE with CMake and Ninja...
cmake -B build -G "Ninja" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_C_COMPILER=cl ^
    -DCMAKE_CXX_COMPILER=cl ^
    -DARCLINE_BUILD_TESTS=ON

if errorlevel 1 (
    echo [ERROR] CMake configuration failed!
    exit /b 1
)

echo.
echo [INFO] Building ARCLINE Desktop Executable and Test Suite...
cmake --build build --config Release

if errorlevel 1 (
    echo [ERROR] Build failed!
    exit /b 1
)

echo.
echo ========================================================
echo    Running Automated Test Suite
echo ========================================================
ctest --test-dir build --output-on-failure

if errorlevel 1 (
    echo [ERROR] Some automated tests failed!
    exit /b 1
)

echo.
echo ========================================================
echo    SUCCESS: All build targets and tests passed!
echo ========================================================
exit /b 0
