@echo off
setlocal enabledelayedexpansion
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 (
    echo [ERROR] Failed to initialize MSVC environment.
    exit /b 1
)

set "PATH=C:\Program Files\CMake\bin;%PATH%"

if "%1"=="clean" (
    if exist build (
        echo [INFO] Cleaning previous build directory...
        rmdir /s /q build
    )
)

echo [INFO] Configuring ARCLINE with CMake and Ninja...
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DARCLINE_BUILD_TESTS=ON
if errorlevel 1 (
    echo [ERROR] CMake configuration failed.
    exit /b 1
)

echo [INFO] Compiling ARCLINE Desktop DAW and Tests with Ninja...
cmake --build build --config Release
if errorlevel 1 (
    echo [ERROR] Compilation failed.
    exit /b 1
)

echo [INFO] Running CTest test suite...
ctest --test-dir build --output-on-failure
if errorlevel 1 (
    echo [ERROR] Test suite failed.
    exit /b 1
)

echo ========================================================
echo   SUCCESS: All build targets and tests passed!
echo ========================================================
exit /b 0
