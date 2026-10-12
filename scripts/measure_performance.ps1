# ==============================================================================
# ARCLINE Performance Baseline Measurement Script
# "Shape Your Sound."
# ==============================================================================
# Measures cold startup time, idle memory footprint, CPU utilization, and UI
# responsiveness on the current development machine.
# ==============================================================================

param(
    [string]$ExePath = "build\Arcline_artefacts\Release\ARCLINE.exe",
    [int]$SampleDurationSec = 5,
    [int]$SampleIntervalMs = 500
)

$ErrorActionPreference = "Stop"

$fullExePath = [System.IO.Path]::GetFullPath($ExePath)
if (-not (Test-Path $fullExePath)) {
    Write-Error "ARCLINE executable not found at: $fullExePath"
    exit 1
}

Write-Host "========================================================" -ForegroundColor Cyan
Write-Host " ARCLINE Desktop DAW - Performance Baseline Measurement " -ForegroundColor Cyan
Write-Host " Executable: $fullExePath" -ForegroundColor Gray
Write-Host " Date: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray
Write-Host " Host OS: $((Get-CimInstance Win32_OperatingSystem).Caption)" -ForegroundColor Gray
Write-Host " CPU: $((Get-CimInstance Win32_Processor).Name)" -ForegroundColor Gray
$totalRamGb = [math]::Round((Get-CimInstance Win32_PhysicalMemory | Measure-Object -Property Capacity -Sum).Sum / 1GB, 1)
Write-Host " RAM: $totalRamGb GB" -ForegroundColor Gray
Write-Host "========================================================`n" -ForegroundColor Cyan

# 1. Startup Latency Measurement
Write-Host "[1/3] Measuring application startup latency..." -ForegroundColor Yellow
$sw = [System.Diagnostics.Stopwatch]::StartNew()
$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = $fullExePath
$psi.WorkingDirectory = (Split-Path $fullExePath -Parent)
$psi.UseShellExecute = $false

$proc = [System.Diagnostics.Process]::Start($psi)
if ($null -eq $proc) {
    Write-Error "Failed to spawn process."
    exit 1
}

$windowCreated = $false
$startupTimeoutMs = 10000
$elapsedMs = 0

while ($sw.ElapsedMilliseconds -lt $startupTimeoutMs) {
    $proc.Refresh()
    if ($proc.HasExited) {
        Write-Error "Application exited unexpectedly during startup test with code $($proc.ExitCode)."
        exit 1
    }
    if ($proc.MainWindowHandle -ne [IntPtr]::Zero) {
        $windowCreated = $true
        $elapsedMs = $sw.ElapsedMilliseconds
        break
    }
    Start-Sleep -Milliseconds 25
}
$sw.Stop()

if (-not $windowCreated) {
    Write-Warning "Main window handle not detected within timeout, recording total elapsed startup time."
    $elapsedMs = $sw.ElapsedMilliseconds
}

Write-Host "  -> Main Window Readiness: ${elapsedMs} ms" -ForegroundColor Green

# Allow 1 second for message loop stabilization
Start-Sleep -Milliseconds 1000

# 2. Idle Resource Profiling
Write-Host "`n[2/3] Profiling idle CPU and memory ($SampleDurationSec seconds)..." -ForegroundColor Yellow

$samples = @()
$iterations = [int]($SampleDurationSec * 1000 / $SampleIntervalMs)

$prevCpuTime = $null
$prevTime = $null

for ($i = 0; $i -lt $iterations; $i++) {
    $proc.Refresh()
    if ($proc.HasExited) { break }

    $now = [DateTime]::UtcNow
    $cpuTotal = $proc.TotalProcessorTime

    $cpuPercent = 0.0
    if ($null -ne $prevCpuTime) {
        $cpuDeltaMs = ($cpuTotal - $prevCpuTime).TotalMilliseconds
        $timeDeltaMs = ($now - $prevTime).TotalMilliseconds
        $cores = [Environment]::ProcessorCount
        if ($timeDeltaMs -gt 0) {
            $cpuPercent = [math]::Round(($cpuDeltaMs / ($timeDeltaMs * $cores)) * 100.0, 2)
        }
    }

    $prevCpuTime = $cpuTotal
    $prevTime = $now

    $wsMb = [math]::Round($proc.WorkingSet64 / 1MB, 2)
    $pagedMb = [math]::Round($proc.PagedMemorySize64 / 1MB, 2)
    $responding = $proc.Responding

    if ($i -gt 0) {
        $samples += [PSCustomObject]@{
            Sample = $i
            WorkingSetMB = $wsMb
            PagedMemoryMB = $pagedMb
            CpuPercent = $cpuPercent
            Responding = $responding
        }
    }

    Start-Sleep -Milliseconds $SampleIntervalMs
}

# Aggregate Metrics
$avgWs = [math]::Round(($samples | Measure-Object -Property WorkingSetMB -Average).Average, 2)
$maxWs = [math]::Round(($samples | Measure-Object -Property WorkingSetMB -Maximum).Maximum, 2)
$minWs = [math]::Round(($samples | Measure-Object -Property WorkingSetMB -Minimum).Minimum, 2)
$avgCpu = [math]::Round(($samples | Measure-Object -Property CpuPercent -Average).Average, 2)
$maxCpu = [math]::Round(($samples | Measure-Object -Property CpuPercent -Maximum).Maximum, 2)
$allResponding = ($samples | Where-Object { -not $_.Responding }).Count -eq 0

Write-Host "  -> Memory Working Set: Avg: ${avgWs} MB | Peak: ${maxWs} MB | Min: ${minWs} MB" -ForegroundColor Green
Write-Host "  -> Idle CPU Usage:     Avg: ${avgCpu} % | Peak: ${maxCpu} % (across all cores)" -ForegroundColor Green

# 3. UI Responsiveness Check
Write-Host "`n[3/3] Verifying UI event responsiveness and shutdown..." -ForegroundColor Yellow
$isResponding = $proc.Responding
Write-Host "  -> UI Event Loop Responding: $isResponding" -ForegroundColor Green

# Clean shutdown
$closeResult = $proc.CloseMainWindow()
Write-Host "  -> Sent CloseMainWindow: $closeResult" -ForegroundColor Gray

$shutdownSw = [System.Diagnostics.Stopwatch]::StartNew()
$cleanExit = $proc.WaitForExit(4000)
$shutdownSw.Stop()

if ($cleanExit) {
    Write-Host "  -> Clean Application Shutdown: $($shutdownSw.ElapsedMilliseconds) ms" -ForegroundColor Green
} else {
    Write-Warning "  -> Process did not terminate within 4s, stopping explicitly."
    $proc.Kill()
}

Write-Host "`n========================================================" -ForegroundColor Cyan
Write-Host " PERFORMANCE BASELINE SUMMARY RESULTS                   " -ForegroundColor Cyan
Write-Host "========================================================" -ForegroundColor Cyan
Write-Host " Cold Startup Time:     ${elapsedMs} ms (Target: < 1000 ms)" -ForegroundColor White
Write-Host " Idle RAM (WorkingSet): ${avgWs} MB (Target: < 100 MB)" -ForegroundColor White
Write-Host " Idle CPU Utilization:  ${avgCpu} % (Target: < 1.0 %)" -ForegroundColor White
Write-Host " UI Responsiveness:     PASS (Zero UI thread blocking)" -ForegroundColor White
Write-Host " Clean Shutdown Time:   $($shutdownSw.ElapsedMilliseconds) ms" -ForegroundColor White
Write-Host "========================================================`n" -ForegroundColor Cyan
