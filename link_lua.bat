@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
set "INCDIR=test\lua-5.4.6\src"
set "OUTDIR=build-debug\lua-obj"
set "LIBS=advapi32.lib kernel32.lib msvcrt.lib vcruntime.lib oldnames.lib legacy_stdio_definitions.lib"
mkdir %OUTDIR% 2>nul

echo === Recompiling all .pp.c with /MD ===
for %%F in (lapi lauxlib lbaselib lcode lcorolib lctype ldblib ldebug ldo ldump lfunc lgc linit liolib llex lmathlib lmem loadlib lobject lopcodes loslib lparser lstate lstring lstrlib ltable ltablib ltm lua luac lundump lutf8lib lvm lzio) do (
  clang-cl /nologo /W0 /c /O2 /MD /I%INCDIR% %INCDIR%\%%F.pp.c /Fo%OUTDIR%\%%F.obj 2>nul
  if errorlevel 1 (echo   %%F.pp.c FAILED) else (echo   %%F.pp.c OK)
)

echo.
echo === Linking lua.exe ===
link /nologo /OUT:build-debug\lua.exe %LIBS% ^
  %OUTDIR%\lapi.obj %OUTDIR%\lauxlib.obj %OUTDIR%\lbaselib.obj %OUTDIR%\lcode.obj ^
  %OUTDIR%\lcorolib.obj %OUTDIR%\lctype.obj %OUTDIR%\ldblib.obj %OUTDIR%\ldebug.obj ^
  %OUTDIR%\ldo.obj %OUTDIR%\ldump.obj %OUTDIR%\lfunc.obj %OUTDIR%\lgc.obj ^
  %OUTDIR%\linit.obj %OUTDIR%\liolib.obj %OUTDIR%\llex.obj %OUTDIR%\lmathlib.obj ^
  %OUTDIR%\lmem.obj %OUTDIR%\loadlib.obj %OUTDIR%\lobject.obj %OUTDIR%\lopcodes.obj ^
  %OUTDIR%\loslib.obj %OUTDIR%\lparser.obj %OUTDIR%\lstate.obj %OUTDIR%\lstring.obj ^
  %OUTDIR%\lstrlib.obj %OUTDIR%\ltable.obj %OUTDIR%\ltablib.obj %OUTDIR%\ltm.obj ^
  %OUTDIR%\lundump.obj %OUTDIR%\lutf8lib.obj %OUTDIR%\lvm.obj %OUTDIR%\lzio.obj ^
  %OUTDIR%\lua.obj 2>&1
if errorlevel 1 (echo lua.exe link FAILED) else (echo lua.exe link OK)

echo.
echo === Linking luac.exe ===
link /nologo /OUT:build-debug\luac.exe %LIBS% ^
  %OUTDIR%\lapi.obj %OUTDIR%\lauxlib.obj %OUTDIR%\lbaselib.obj %OUTDIR%\lcode.obj ^
  %OUTDIR%\lcorolib.obj %OUTDIR%\lctype.obj %OUTDIR%\ldblib.obj %OUTDIR%\ldebug.obj ^
  %OUTDIR%\ldo.obj %OUTDIR%\ldump.obj %OUTDIR%\lfunc.obj %OUTDIR%\lgc.obj ^
  %OUTDIR%\linit.obj %OUTDIR%\liolib.obj %OUTDIR%\llex.obj %OUTDIR%\lmathlib.obj ^
  %OUTDIR%\lmem.obj %OUTDIR%\loadlib.obj %OUTDIR%\lobject.obj %OUTDIR%\lopcodes.obj ^
  %OUTDIR%\loslib.obj %OUTDIR%\lparser.obj %OUTDIR%\lstate.obj %OUTDIR%\lstring.obj ^
  %OUTDIR%\lstrlib.obj %OUTDIR%\ltable.obj %OUTDIR%\ltablib.obj %OUTDIR%\ltm.obj ^
  %OUTDIR%\lundump.obj %OUTDIR%\lutf8lib.obj %OUTDIR%\lvm.obj %OUTDIR%\lzio.obj ^
  %OUTDIR%\luac.obj 2>&1
if errorlevel 1 (echo luac.exe link FAILED) else (echo luac.exe link OK)

echo.
if exist build-debug\lua.exe (echo lua.exe: OK) else (echo lua.exe: NOT FOUND)
if exist build-debug\luac.exe (echo luac.exe: OK) else (echo luac.exe: NOT FOUND)
