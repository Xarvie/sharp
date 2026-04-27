@echo off
C:\msys64\mingw64\bin\gcc.exe tests\test_designated_init.c -o tests\test_designated_init.exe
if %errorlevel% neq 0 (
    echo FAILED with errorlevel %errorlevel%
) else (
    echo SUCCESS
    tests\test_designated_init.exe
)
