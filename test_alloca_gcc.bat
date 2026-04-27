@echo off
C:\msys64\mingw64\bin\gcc.exe tests\test_alloca.c -o tests\test_alloca.exe 2>gcc_stderr.txt 1>gcc_stdout.txt
echo EXITCODE=%errorlevel%
type gcc_stdout.txt
type gcc_stderr.txt
