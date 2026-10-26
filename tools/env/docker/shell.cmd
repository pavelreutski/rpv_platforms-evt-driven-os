@echo off
rem Runs shell.ps1 even if the PowerShell execution policy blocks scripts.
rem Usage: tools\env\docker\shell.cmd [--rebuild] <x86_64|microblaze|avr|all> [command...]
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0shell.ps1" %*
