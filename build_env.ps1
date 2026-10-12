$env:PATH = "E:\SonaraDaw\toolchain\bin;" + $env:PATH
Write-Host "--- Toolchain Environment Verification ---"
Write-Host "CMake:"
cmake --version
Write-Host "Ninja:"
ninja --version
Write-Host "G++:"
g++ --version
