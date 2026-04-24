@echo off
setlocal enabledelayedexpansion

set SHARPC=cmake-build-debug\sharpc.exe
set FAIL=0
set PASS=0

echo === Cleaning generated .c and .exe in tests/ ===
del /Q tests\*.c 2>nul
del /Q tests\*.exe 2>nul

echo === Running all .sp tests in tests/ directory ===
echo.

for %%f in (tests\*.sp) do (
    set NAME=%%f
    set EXE=tests\%%~nf.exe
    echo --- Testing %%f ---
    %SHARPC% "%%f" -o "!EXE!" 2>nul
    if errorlevel 1 (
        echo FAIL  %%f  compilation failed
        set /a FAIL+=1
    ) else (
        echo PASS  %%f  compiled OK
        set /a PASS+=1
    )
)

echo.
echo ==============================
echo PASS: !PASS!   FAIL: !FAIL!
echo ==============================

if !FAIL!==0 (
    echo ALL TESTS PASSED
) else (
    echo SOME TESTS FAILED
)

echo === Cleaning generated .c and .exe in tests/ ===
del /Q tests\*.c 2>nul
del /Q tests\*.exe 2>nul

endlocal
exit /b %FAIL%
