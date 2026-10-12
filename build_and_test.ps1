# SONORA Phase 1 - MSVC Environment Setup, Build and Test Script
$ErrorActionPreference = "Stop"

Write-Host "========================================================" -ForegroundColor Cyan
Write-Host "   SONORA Phase 1 - MSVC Build and Test Pipeline        " -ForegroundColor Cyan
Write-Host "========================================================" -ForegroundColor Cyan

$vcvars = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
if (-not (Test-Path $vcvars)) {
    Write-Error "vcvarsall.bat not found at: $vcvars"
    exit 1
}

Write-Host "[INFO] Loading MSVC x64 Environment variables..." -ForegroundColor Yellow
$envLines = cmd /c "call `"$vcvars`" x64 >nul 2>&1 && set"
foreach ($line in $envLines) {
    if ($line -match "^([^=]+)=(.*)$") {
        $varName = $matches[1]
        $varVal = $matches[2]
        [System.Environment]::SetEnvironmentVariable($varName, $varVal, [System.EnvironmentVariableTarget]::Process)
    }
}

$vsCMakeBin = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
$env:PATH = "$vsCMakeBin;" + $env:PATH

$clPath = (Get-Command cl.exe -ErrorAction SilentlyContinue).Source
Write-Host "  MSVC cl.exe: $clPath"
$cmakePath = (Get-Command cmake.exe -ErrorAction SilentlyContinue).Source
Write-Host "  CMake:       $cmakePath"
$nmakePath = (Get-Command nmake.exe -ErrorAction SilentlyContinue).Source
Write-Host "  NMake:       $nmakePath"

# Clean build directory if exists
if (Test-Path "build") {
    Remove-Item -Recurse -Force "build" -ErrorAction SilentlyContinue
}

Write-Host "`n[INFO] Configuring SONORA with CMake and NMake Makefiles..." -ForegroundColor Yellow
& cmake -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DSONORA_BUILD_TESTS=ON
if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

Write-Host "`n[INFO] Building SONORA Desktop App and Test Suite with NMake..." -ForegroundColor Yellow
& cmake --build build --config Release
if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake build failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

Write-Host "`n========================================================" -ForegroundColor Cyan
Write-Host "   Running Automated Test Suite                         " -ForegroundColor Cyan
Write-Host "========================================================" -ForegroundColor Cyan
& ctest --test-dir build --output-on-failure
if ($LASTEXITCODE -ne 0) {
    Write-Error "Test suite failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

Write-Host "`n========================================================" -ForegroundColor Green
Write-Host "   SUCCESS: All build targets and tests passed!         " -ForegroundColor Green
Write-Host "========================================================" -ForegroundColor Green
exit 0
