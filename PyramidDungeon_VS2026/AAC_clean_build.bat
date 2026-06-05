@echo off
cd /d "%~dp0"
if exist build rmdir /s /q build
if exist build-vs2026 rmdir /s /q build-vs2026
if exist build-ninja rmdir /s /q build-ninja
echo Dossiers de build supprimes.
pause
