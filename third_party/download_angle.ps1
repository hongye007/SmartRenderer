# PowerShell script to download ANGLE prebuilt libraries for Windows
# This script downloads ANGLE binaries from Google's Chrome infrastructure

$ErrorActionPreference = "Stop"

Write-Host "=== Downloading ANGLE Libraries ===" -ForegroundColor Green

# ANGLE version and download URL
# Using Chrome's prebuilt ANGLE binaries
$angleVersion = "chromium/6099"
$baseUrl = "https://chromium.googlesource.com/angle/angle"

# Create angle directory
$angleDir = Join-Path $PSScriptRoot "angle"
if (Test-Path $angleDir) {
    Write-Host "ANGLE directory already exists. Removing..." -ForegroundColor Yellow
    Remove-Item -Path $angleDir -Recurse -Force
}

New-Item -ItemType Directory -Path $angleDir | Out-Null
Write-Host "Created directory: $angleDir" -ForegroundColor Cyan

# For simplicity, we'll download ANGLE prebuilt libraries from a more accessible source
# Alternative: Use NuGet package or build from source

Write-Host "`nAttempting to download ANGLE via NuGet package manager..." -ForegroundColor Cyan

# Create a temporary directory for NuGet
$nugetDir = Join-Path $PSScriptRoot "nuget_temp"
New-Item -ItemType Directory -Path $nugetDir -Force | Out-Null

# Download NuGet.exe if not present
$nugetExe = Join-Path $nugetDir "nuget.exe"
if (-not (Test-Path $nugetExe)) {
    Write-Host "Downloading NuGet.exe..." -ForegroundColor Cyan
    $nugetUrl = "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe"
    Invoke-WebRequest -Uri $nugetUrl -OutFile $nugetExe
    Write-Host "NuGet.exe downloaded successfully" -ForegroundColor Green
}

# Try to install ANGLE via NuGet (Google.ANGLE package)
Write-Host "Installing ANGLE package via NuGet..." -ForegroundColor Cyan
Push-Location $nugetDir
& $nugetExe install ANGLE -OutputDirectory . -NonInteractive

# Check if package was installed
$anglePackage = Get-ChildItem -Path $nugetDir -Filter "ANGLE.*" -Directory | Select-Object -First 1

if ($anglePackage) {
    Write-Host "ANGLE package found: $($anglePackage.FullName)" -ForegroundColor Green
    
    # Copy files to angle directory
    $libDir = Join-Path $angleDir "lib"
    $includeDir = Join-Path $angleDir "include"
    New-Item -ItemType Directory -Path $libDir -Force | Out-Null
    New-Item -ItemType Directory -Path $includeDir -Force | Out-Null
    
    # Copy libraries
    $packageLibs = Get-ChildItem -Path $anglePackage.FullName -Recurse -Filter "*.lib"
    foreach ($lib in $packageLibs) {
        Copy-Item -Path $lib.FullName -Destination $libDir -Force
        Write-Host "Copied: $($lib.Name)" -ForegroundColor Cyan
    }
    
    $packageDlls = Get-ChildItem -Path $anglePackage.FullName -Recurse -Filter "*.dll"
    foreach ($dll in $packageDlls) {
        Copy-Item -Path $dll.FullName -Destination $libDir -Force
        Write-Host "Copied: $($dll.Name)" -ForegroundColor Cyan
    }
    
    # Copy headers
    $packageHeaders = Get-ChildItem -Path $anglePackage.FullName -Recurse -Include "*.h"
    foreach ($header in $packageHeaders) {
        $relativePath = $header.FullName.Substring($anglePackage.FullName.Length + 1)
        $destPath = Join-Path $includeDir $relativePath
        $destDir = Split-Path -Parent $destPath
        New-Item -ItemType Directory -Path $destDir -Force | Out-Null
        Copy-Item -Path $header.FullName -Destination $destPath -Force
    }
    
    Write-Host "`nANGLE installed successfully!" -ForegroundColor Green
} else {
    Write-Host "Warning: Could not install ANGLE via NuGet" -ForegroundColor Yellow
    Write-Host "Please download ANGLE manually from: https://github.com/google/angle" -ForegroundColor Yellow
}

Pop-Location

# Clean up NuGet temp directory
Write-Host "`nCleaning up..." -ForegroundColor Cyan
Remove-Item -Path $nugetDir -Recurse -Force

Write-Host "`n=== ANGLE Download Complete ===" -ForegroundColor Green
Write-Host "ANGLE libraries location: $angleDir" -ForegroundColor Cyan
Write-Host "`nNote: If automatic download failed, please:" -ForegroundColor Yellow
Write-Host "  1. Download ANGLE from: https://github.com/google/angle/releases" -ForegroundColor Yellow
Write-Host "  2. Extract to: $angleDir" -ForegroundColor Yellow
Write-Host "  3. Ensure lib/ and include/ directories exist" -ForegroundColor Yellow
