[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new()
$OutputEncoding = [System.Text.UTF8Encoding]::new()

function Find-Executable {
    param(
        [Parameter(Mandatory)]
        [string]$Name,
        [string[]]$Candidates = @()
    )

    $command = Get-Command $Name -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    foreach ($candidate in $Candidates) {
        if ($candidate -and (Test-Path -LiteralPath $candidate)) {
            return $candidate
        }
    }

    return $null
}

function Install-WingetPackage {
    param(
        [Parameter(Mandatory)]
        [string]$Id,
        [string]$Override
    )

    Write-Host ""
    Write-Host "[INSTALLATION] $Id" -ForegroundColor Cyan

    $arguments = @(
        "install",
        "--id", $Id,
        "--exact",
        "--source", "winget",
        "--accept-package-agreements",
        "--accept-source-agreements",
        "--disable-interactivity"
    )

    if ($Override) {
        $arguments += @("--override", $Override)
    } else {
        $arguments += "--silent"
    }

    & winget @arguments
    if ($LASTEXITCODE -ne 0) {
        throw "L'installation de $Id a echoue (code $LASTEXITCODE)."
    }
}

$winget = Find-Executable -Name "winget.exe"
if (-not $winget) {
    throw "winget est introuvable. Installe 'App Installer' depuis le Microsoft Store, puis relance le compilateur."
}

$cmakeCandidates = @(
    (Join-Path $env:ProgramFiles "CMake\bin\cmake.exe"),
    (Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe")
)
$cmake = Find-Executable -Name "cmake.exe" -Candidates $cmakeCandidates
if (-not $cmake) {
    Install-WingetPackage -Id "Kitware.CMake"
    $cmake = Find-Executable -Name "cmake.exe" -Candidates $cmakeCandidates
}

$gitCandidates = @(
    (Join-Path $env:ProgramFiles "Git\cmd\git.exe")
)
$git = Find-Executable -Name "git.exe" -Candidates $gitCandidates
if (-not $git) {
    Install-WingetPackage -Id "Git.Git"
    $git = Find-Executable -Name "git.exe" -Candidates $gitCandidates
}

$vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
$vsPath = $null
if (Test-Path -LiteralPath $vswhere) {
    $vsPath = & $vswhere -latest -products * -version "[18.0,19.0)" `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
}

if (-not $vsPath) {
    $existingVs18 = $null
    if (Test-Path -LiteralPath $vswhere) {
        $existingVs18 = & $vswhere -latest -products * -version "[18.0,19.0)" -property installationPath
    }

    if ($existingVs18) {
        $setup = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\setup.exe"
        if (-not (Test-Path -LiteralPath $setup)) {
            throw "L'installateur Visual Studio est introuvable."
        }

        Write-Host ""
        Write-Host "[INSTALLATION] Outils C++ Visual Studio 2026" -ForegroundColor Cyan
        & $setup modify --installPath $existingVs18 `
            --add Microsoft.VisualStudio.Workload.VCTools `
            --includeRecommended --passive --norestart
        if ($LASTEXITCODE -ne 0) {
            throw "L'ajout des outils C++ Visual Studio a echoue (code $LASTEXITCODE)."
        }
    } else {
        Install-WingetPackage -Id "Microsoft.VisualStudio.BuildTools" `
            -Override "--wait --passive --norestart --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
    }

    if (Test-Path -LiteralPath $vswhere) {
        $vsPath = & $vswhere -latest -products * -version "[18.0,19.0)" `
            -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    }
}

if (-not $cmake) {
    throw "CMake reste introuvable apres l'installation. Redemarre Windows puis relance le compilateur."
}
if (-not $git) {
    throw "Git reste introuvable apres l'installation. Redemarre Windows puis relance le compilateur."
}
if (-not $vsPath) {
    throw "Visual Studio Build Tools 2026 avec les outils C++ reste introuvable. Redemarre Windows puis relance le compilateur."
}

Write-Host ""
Write-Host "[OK] Environnement de compilation pret." -ForegroundColor Green
Write-Host "CMake : $cmake"
Write-Host "Git   : $git"
Write-Host "MSVC  : $vsPath"
