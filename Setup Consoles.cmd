@echo off
rem Builds SkiFree for every console from your ski32.exe, with Docker: see tools\setup-consoles.ps1.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\setup-consoles.ps1" %*
if not defined RECOMP_LAUNCHER pause
