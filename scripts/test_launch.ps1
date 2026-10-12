$exePath = "build\Arcline_artefacts\Release\ARCLINE.exe"
if (!(Test-Path $exePath)) {
    $exePath = "build\bin\ARCLINE.exe"
}
if (!(Test-Path $exePath)) {
    Write-Error "ARCLINE executable not found at expected path: $exePath"
    exit 1
}

Write-Host "[LAUNCHING ARCLINE]: $exePath"
$proc = Start-Process -FilePath $exePath -PassThru
Start-Sleep -Milliseconds 2000

$logPath = Join-Path $env:APPDATA "Arcline\arcline_debug.log"
if (Test-Path $logPath) {
    Write-Host "[LOG CONTENT FOUND at $logPath]:"
    Get-Content $logPath
} else {
    Write-Host "[LOG NOT FOUND at $logPath]"
}

if (!$proc.HasExited) {
    Write-Host "[PROCESS RUNNING HEALTHILY with PID $($proc.Id)]"
    Stop-Process -Id $proc.Id -Force
    Write-Host "[PROCESS TERMINATED CLEANLY]"
} else {
    Write-Host "[PROCESS EXITED with code $($proc.ExitCode)]"
}
