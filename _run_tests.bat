@echo off
set PASS=0
set FAIL=0

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64 >nul 2>&1

for %%f in (c:\Users\ftp\Desktop\sharp\tests\test_*.sp) do (
    set "name=%%~nf"
    set "spfile=%%f"
    set "cfile=c:\Users\ftp\Desktop\sharp\tests\!name!.c"
    set "exefile=c:\Users\ftp\Desktop\sharp\tests\!name!.exe"

    "c:\Users\ftp\Desktop\sharp\cmake-build-debug\sharpc.exe" "!spfile!" -no-link -o "!cfile!" >nul 2>&1
    if errorlevel 1 (
        echo FAIL: sharpc !name!
        set /a FAIL+=1
        goto :next
    )

    "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\bin\Hostx64\x64\cl.exe" /nologo /O2 /Fe:"!exefile!" "!cfile!" >nul 2>&1
    if errorlevel 1 (
        echo FAIL: cl !name!
        set /a FAIL+=1
        goto :next
    )

    "!exefile!" >nul 2>&1
    if errorlevel 1 (
        echo FAIL: run !name!
        set /a FAIL+=1
    ) else (
        echo PASS: !name!
        set /a PASS+=1
    )

    :next
)

echo.
echo ==============================
echo PASS: %PASS%   FAIL: %FAIL%
echo ==============================

if %FAIL%==0 (
    echo ALL TESTS PASSED
)

endlocal
