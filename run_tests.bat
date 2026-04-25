@echo off
echo === SP Compiler Regression Tests (PowerShell version) ===
echo.
powershell -ExecutionPolicy Bypass -File "%~dp0run_tests.ps1"
exit /b %ERRORLEVEL%
