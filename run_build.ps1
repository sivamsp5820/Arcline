$ErrorActionPreference = "Stop"

$env:PATH = "E:\SonaraDaw\toolchain\bin;" + $env:PATH

Write-Host "=========================================="
Write-Host "   Configuring SONORA with CMake & Ninja  "
Write-Host "=========================================="

if (Test-Path build) {
    Remove-Item -Recurse -Force build
}

cmake -B build -G "Ninja" `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_C_COMPILER=gcc.exe `
    -DCMAKE_CXX_COMPILER=g++.exe `
    -DSONORA_BUILD_TESTS=ON

Write-Host "`n=========================================="
Write-Host "   Building Targets (Sonora & SonoraTests) "
Write-Host "=========================================="

cmake --build build --config Release

Write-Host "`n=========================================="
Write-Host "   Running Automated Test Suite via CTest  "
Write-Host "=========================================="

cd build
ctest --output-on-failure
cd ..

Write-Host "`n=========================================="
Write-Host "   Direct Execution of SonoraTests.exe    "
Write-Host "=========================================="

& ".\build\bin\SonoraTests.exe"
