@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64 >nul 2>&1
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\bin\Hostx64\x64\cl.exe" /nologo /O2 /Fe:c:\Users\ftp\Desktop\sharp\tests\%1.exe c:\Users\ftp\Desktop\sharp\tests\%1.c
echo EXIT_CODE:%ERRORLEVEL%
