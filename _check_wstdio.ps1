$ucrtDir = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt"
$sharpc = "c:\Users\ftp\Desktop\sharp\cmake-build-debug\sharpc.exe"

# Test corecrt_wstdio.h directly
$name = "corecrt_wstdio.h"
$sp = "$env:TEMP\pp_wstdio.sp"
$c = "$env:TEMP\pp_wstdio.c"
$content = "/* auto */`n#include <$name>`nint main() { return 0; }"
Set-Content $sp $content -Encoding ASCII -NoNewline

Write-Host "=== Testing corecrt_wstdio.h ===" -ForegroundColor Cyan
$out = & $sharpc $sp -no-link -o $c 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "Errors:" -ForegroundColor Red
    $out | Where-Object { $_ -match "error\[E\d+\]" } | Select-Object -First 5 | ForEach-Object { Write-Host "  $_" }
} else {
    Write-Host "Success! Lines around the error area:" -ForegroundColor Green
    if (Test-Path $c) {
        $lines = Get-Content $c
        Write-Host "Total lines: $($lines.Count)"
    }
}

Remove-Item $sp, $c -ErrorAction SilentlyContinue
