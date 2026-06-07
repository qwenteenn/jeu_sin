@echo off
setlocal
cd /d "%~dp0"

echo.
echo === PyramidDungeon - installation et compilation Windows x64 ===
echo.

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0bootstrap_windows_build.ps1"
if errorlevel 1 goto bootstrap_error

set "PATH=%ProgramFiles%\CMake\bin;%ProgramFiles%\Git\cmd;%PATH%"

where cmake >nul 2>nul
if errorlevel 1 (
    echo [ERREUR] CMake est installe mais n'est pas encore visible.
    echo Redemarre Windows puis relance ce script.
    goto error
)

where git >nul 2>nul
if errorlevel 1 (
    echo [ERREUR] Git est installe mais n'est pas encore visible.
    echo Redemarre Windows puis relance ce script.
    goto error
)

cmake --help | findstr /C:"Visual Studio 18 2026" >nul
if errorlevel 1 (
    echo [ERREUR] Cette version de CMake ne connait pas Visual Studio 2026.
    echo Mets CMake a jour avec : winget upgrade Kitware.CMake
    goto error
)

echo.
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

:bootstrap_error
echo.
echo [ERREUR] L'installation des outils de compilation a echoue.
echo Consulte le message affiche plus haut, puis relance le script.
goto error_pause

:error
echo.
echo [ERREUR] Build echoue.

:error_pause
pause
exit /b 1
