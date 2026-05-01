@echo off
set "INCDIR=test\lua-5.4.6\src"
set "OUTDIR=build-debug\lua-obj"
mkdir %OUTDIR% 2>nul

for %%F in (lapi lauxlib lbaselib lcode lcorolib lctype ldblib ldebug ldo ldump lfunc lgc linit liolib llex lmathlib lmem loadlib lobject lopcodes loslib lparser lstate lstring lstrlib ltable ltablib ltm lua luac lundump lutf8lib lvm lzio) do (
  echo Compiling %%F.pp.c...
  clang-cl /nologo /W3 /c /Od /Zi /I%INCDIR% %INCDIR%\%%F.pp.c /Fo%OUTDIR%\%%F.pp.obj > %OUTDIR%\%%F.log 2>&1
  findstr /i "error:" %OUTDIR%\%%F.log > nul
  if errorlevel 1 (
    echo   OK
  ) else (
    echo   FAILED - see %OUTDIR%\%%F.log
    type %OUTDIR%\%%F.log | findstr /i "error:"
  )
)
echo Done.
