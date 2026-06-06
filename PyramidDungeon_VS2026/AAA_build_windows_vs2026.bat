@echo off
setlocal enabledelayedexpansion

cd /d "%~dp0"

echo.
echo === PyramidDungeon - build Visual Studio 2026 x64 ===
echo.

where cmake >nul 2>nul
if errorlevel 1 (
    echo [ERREUR] CMake est introuvable dans le PATH.
    echo Lance ce script depuis Developer PowerShell / Native Tools, ou installe CMake.
    pause
    exit /b 1
)

where git >nul 2>nul
if errorlevel 1 (
    echo [ERREUR] Git est introuvable. CMake en a besoin pour telecharger raylib.
    pause
    exit /b 1
)

cmake --help | findstr /C:"Visual Studio 18 2026" >nul
if errorlevel 1 (
    echo [ERREUR] Ton CMake ne connait pas le generateur "Visual Studio 18 2026".
    echo Solution 1 : utilise build_windows_ninja_msvc.bat depuis un terminal developpeur VS 2026.
    echo Solution 2 : installe / utilise le CMake fourni avec VS 2026 ou un CMake recent.
    pause
    exit /b 1
)

if exist build-vs2026 (
    echo Nettoyage de build-vs2026...
    rmdir /s /q build-vs2026
)

echo Configuration CMake...
cmake --preset vs2026-release
if errorlevel 1 goto error

echo.
echo Compilation Release...
cmake --build --preset build-vs2026-release
if errorlevel 1 goto error

echo.
echo [OK] Build termine.
echo EXE : build-vs2026\Release\PyramidDungeon.exe
echo.
pause
exit /b 0

:error
echo.
echo [ERREUR] Build echoue. Supprime le dossier build-vs2026 si besoin, puis relance.
pause
exit /b 1
