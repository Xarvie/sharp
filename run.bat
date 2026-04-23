@echo off
rem run.bat — One-click compile and run a Sharp program
rem Usage: run.bat [file.sp]
rem   Default: demo.sp

set "SRC=%~1"
if "%SRC%"=="" set "SRC=demo.sp"

if not exist "%SRC%" (
    echo Error: %SRC% not found
    exit /b 1
)

rem Determine output exe name
for %%F in ("%SRC%") do set "EXE=%%~nF.exe"

rem Build sharpc if needed
if not exist "cmake-build-debug\sharpc.exe" (
    echo Building sharpc...
    cmake -S . -B cmake-build-debug >nul 2>&1
    cmake --build cmake-build-debug --target sharpc >nul 2>&1
)

rem Compile and link via sharpc + libtcc
cmake-build-debug\sharpc.exe "%SRC%" -o "%EXE%"
if %ERRORLEVEL% neq 0 (
    echo Compilation failed
    exit /b 1
)

echo.
echo === Running %EXE% ===
echo.
"%EXE%"
set "RC=%ERRORLEVEL%"

echo.
echo === Exit code: %RC% ===

rem Cleanup the executable (no .c file is produced — cgen writes to temp)
rem del "%EXE%" 2>nul

pause
exit /b %RC%
