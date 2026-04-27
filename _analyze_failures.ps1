$ErrorActionPreference = 'Continue'
$ucrtDir = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt"
$sharpc = "c:\Users\ftp\Desktop\sharp\cmake-build-debug\sharpc.exe"

$tests = @(
    "complex.h", "corecrt.h", "crtdbg.h", "fpieee.h",
    "mbctype.h", "mbstring.h", "new.h", "safeint.h",
    "safeint_internal.h", "search.h", "stdalign.h",
    "stdnoreturn.h", "tgmath.h", "uchar.h"
)

foreach ($name in $tests) {
    $sp = "$env:TEMP\diag_$($name -replace '\.', '_').sp"
    $c  = "$env:TEMP\diag_$($name -replace '\.', '_').c"
    $content = "/* auto */`n#include <$name>`nint main() { return 0; }"
    Set-Content $sp $content -Encoding ASCII -NoNewline

    Write-Host "`n=== $name ===" -ForegroundColor Cyan
    $out = & $sharpc $sp -no-link -o $c 2>&1
    $errors = $out | Where-Object { $_ -match "error\[E\d+\]" }
    if ($errors) {
        $errors | Select-Object -First 3 | ForEach-Object { Write-Host "  $_" }
    } else {
        Write-Host "  (unknown error)"
    }

    Remove-Item $sp, $c -ErrorAction SilentlyContinue
}
