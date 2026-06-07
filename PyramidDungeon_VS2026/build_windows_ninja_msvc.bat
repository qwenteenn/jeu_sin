@echo off
setlocal enabledelayedexpansion

cd /d "%~dp0"

echo.
echo === PyramidDungeon - build Ninja + MSVC ===
echo Lance ce script depuis "Developer PowerShell for VS 2026" ou "x64 Native Tools Command Prompt for VS 2026".
echo.

where cmake >nul 2>nul
if errorlevel 1 (
    echo [ERREUR] CMake est introuvable.
    pause
    exit /b 1
)

where git >nul 2>nul
if errorlevel 1 (
    echo [ERREUR] Git est introuvable. CMake en a besoin pour telecharger raylib.
    pause
    exit /b 1
)

where cl >nul 2>nul
if errorlevel 1 (
    echo [ERREUR] cl.exe est introuvable.
    echo Ouvre un terminal developpeur Visual Studio 2026, pas un cmd normal sorti du neant.
    pause
    exit /b 1
)

where ninja >nul 2>nul
if errorlevel 1 (
    echo [ERREUR] Ninja est introuvable. Installe Ninja ou utilise build_windows_vs2026.bat.
    pause
    exit /b 1
)

if exist build-ninja (
    echo Nettoyage de build-ninja...
    rmdir /s /q build-ninja
)

echo Configuration CMake...
cmake --preset ninja-msvc-release
if errorlevel 1 goto error

echo.
echo Compilation Release...
cmake --build --preset build-ninja-msvc-release
if errorlevel 1 goto error

echo.
echo [OK] Build termine.
echo EXE : build-ninja\PyramidDungeon.exe
echo.
pause
exit /b 0

:error
echo.
echo [ERREUR] Build echoue.
pause
exit /b 1
