@echo off
setlocal

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 exit /b 1

set "PATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"

if exist build rmdir /s /q build

echo [INFO] Running CMake configure with NMake...
cmake -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DSONORA_BUILD_TESTS=ON
if errorlevel 1 exit /b 1

echo [INFO] Running CMake build...
cmake --build build --config Release
if errorlevel 1 exit /b 1

echo [INFO] Running CTest...
ctest --test-dir build --output-on-failure
if errorlevel 1 exit /b 1

echo [INFO] All done!
exit /b 0
