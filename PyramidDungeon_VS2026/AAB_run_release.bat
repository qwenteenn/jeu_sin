@echo off
cd /d "%~dp0"
if exist "build-vs2026\Release\PyramidDungeon.exe" (
    start "" "build-vs2026\Release\PyramidDungeon.exe"
    exit /b 0
)
if exist "build-ninja\PyramidDungeon.exe" (
    start "" "build-ninja\PyramidDungeon.exe"
    exit /b 0
)
echo Aucun exe trouve. Compile d'abord le projet.
pause
