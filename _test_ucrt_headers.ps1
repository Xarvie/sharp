$ErrorActionPreference = 'Continue'
$sharpc = "c:\Users\ftp\Desktop\sharp\cmake-build-debug\sharpc.exe"

$tests = @(
    "complex.h", "corecrt.h", "crtdbg.h", "fpieee.h",
    "mbctype.h", "mbstring.h", "new.h", "safeint.h",
    "search.h", "stdalign.h", "stdnoreturn.h", "tgmath.h", "uchar.h"
)

$pass = 0
$fail = 0

foreach ($name in $tests) {
    $safeName = $name -replace '\.', '_'
    $sp = "$env:TEMP\test_$safeName.sp"
    $c = "$env:TEMP\test_$safeName.c"
    $content = "/* auto */`n#include <$name>`nint main() { return 0; }"
    Set-Content $sp $content -Encoding ASCII -NoNewline

    $out = & $sharpc $sp -no-link -o $c 2>&1
    $exitCode = $LASTEXITCODE
    
    $errors = $out | Where-Object { $_ -match "error\[" }
    if ($errors.Count -gt 0) {
        $errCount = $errors.Count
        Write-Host "FAIL: $name ($errCount errors)" -ForegroundColor Red
        $errors | Select-Object -First 3 | ForEach-Object { Write-Host "  $_" }
        $fail++
    } else {
        Write-Host "PASS: $name" -ForegroundColor Green
        $pass++
    }

    Remove-Item $sp, $c -ErrorAction SilentlyContinue
}

Write-Host ""
Write-Host "=============================="
Write-Host "PASS: $pass   FAIL: $fail"
Write-Host "=============================="
