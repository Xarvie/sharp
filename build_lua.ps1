$env:SHARPC = "C:\Users\ftp\Desktop\sharp\build-debug\Debug\sharpc.exe"
$env:SRC = "C:\Users\ftp\Desktop\sharp\test\lua-5.4.6\src"
$env:OUT = "C:\Users\ftp\Desktop\sharp\test\lua-5.4.6\out"

$files = @(
    "lapi.c", "lcode.c", "lctype.c", "ldebug.c", "ldo.c", "ldump.c",
    "lfunc.c", "lgc.c", "llex.c", "lmem.c", "lobject.c", "lopcodes.c",
    "lparser.c", "lstate.c", "lstring.c", "ltable.c", "ltm.c",
    "lundump.c", "lvm.c", "lzio.c", "lauxlib.c", "lbaselib.c",
    "lcorolib.c", "ldblib.c", "liolib.c", "lmathlib.c", "loadlib.c",
    "loslib.c", "lstrlib.c", "ltablib.c", "lutf8lib.c", "linit.c",
    "lua.c"
)

foreach ($f in $files) {
    Write-Host "sharpc: $f"
    $srcPath = Join-Path $env:SRC $f
    $outPath = Join-Path $env:OUT $f
    & $env:SHARPC $srcPath -o $outPath -I $env:SRC 2>&1 | Out-Null
    if (Test-Path $outPath) {
        $size = (Get-Item $outPath).Length
        Write-Host "  -> $outPath ($size bytes)"
    } else {
        Write-Host "  -> FAILED"
    }
}

Write-Host "`nAll done!"
