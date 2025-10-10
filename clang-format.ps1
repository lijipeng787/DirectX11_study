# Optional parameter: subdirectory to format (relative or absolute). If omitted, whole tree.
param(
    [Parameter(Position=0)]
    [string]$SubDir
)

# Resolve target root path
if ([string]::IsNullOrWhiteSpace($SubDir)) {
    $TargetRoot = Get-Location
    Write-Host "[Info] No subdirectory specified. Formatting entire directory: $TargetRoot" -ForegroundColor Cyan
} else {
    if (-not (Test-Path -LiteralPath $SubDir)) {
        Write-Error "Specified subdirectory does not exist: $SubDir"
        exit 1
    }
    $Resolved = Resolve-Path -LiteralPath $SubDir | Select-Object -First 1 -ExpandProperty Path
    if (-not (Test-Path -LiteralPath $Resolved -PathType Container)) {
        Write-Error "Specified path is not a directory: $Resolved"
        exit 1
    }
    $TargetRoot = $Resolved
    Write-Host "[Info] Formatting only subdirectory: $TargetRoot" -ForegroundColor Cyan
}

# Gather files
Write-Host "[Info] Collecting source files (*.cpp, *.h, *.hpp) ..." -ForegroundColor Cyan
$FilesToFormat = Get-ChildItem -Path $TargetRoot -Recurse -Include *.cpp,*.h,*.hpp -File 2>$null

if (-not $FilesToFormat -or $FilesToFormat.Count -eq 0) {
    Write-Warning "No matching source files found under: $TargetRoot"
    exit 0
}

# Calculate total and init counter
$TotalCount = $FilesToFormat.Count
$Counter = 0

# Iterate & format
$Stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
$Errors = @()

$FilesToFormat | ForEach-Object {
    $Counter++
    $PercentComplete = ($Counter / $TotalCount) * 100

    Write-Progress -Activity "Clang-Format Formatting Source Files" `
                   -Status "Formatting file ($Counter / $TotalCount): $($_.FullName)" `
                   -PercentComplete $PercentComplete

    try {
        clang-format -i $_.FullName 2>$null
        if ($LASTEXITCODE -ne 0) {
            $Errors += $_.FullName
        }
    } catch {
        $Errors += $_.FullName
    }
}

$Stopwatch.Stop()
Write-Progress -Activity "Clang-Format Formatting Source Files" -Completed $true

Write-Host "[Info] Completed formatting $TotalCount file(s) in $([Math]::Round($Stopwatch.Elapsed.TotalSeconds,2))s" -ForegroundColor Green
if ($Errors.Count -gt 0) {
    Write-Warning "clang-format reported issues on $($Errors.Count) file(s):"
    $Errors | ForEach-Object { Write-Host "  $_" -ForegroundColor Yellow }
    exit 2
}
exit 0