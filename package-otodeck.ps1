$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$sourceExe = Join-Path $projectRoot "Builds\VisualStudio2022\x64\Release\App\OtoDeck.exe"
$targetExe = Join-Path $projectRoot "OtoDeck DJ.exe"
$recordingsDir = Join-Path $projectRoot "Recordings"

if (-not (Test-Path $sourceExe)) {
    throw "Release executable not found: $sourceExe"
}

if (-not (Test-Path $recordingsDir)) {
    New-Item -ItemType Directory -Path $recordingsDir | Out-Null
}

Copy-Item -LiteralPath $sourceExe -Destination $targetExe -Force
Write-Host "Packaged app updated:" $targetExe
