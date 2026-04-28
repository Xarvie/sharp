$ErrorActionPreference = 'Continue'
$sharpc = "c:\Users\ftp\Desktop\sharp\cmake-build-debug\sharpc.exe"
$ucrtDir = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.18362.0\ucrt"

# All UCRT headers from the SDK directory
$tests = @(
    # Root headers
    "assert.h",
    "complex.h",
    "conio.h",
    "corecrt.h",
    "corecrt_io.h",
    "corecrt_malloc.h",
    "corecrt_math.h",
    "corecrt_math_defines.h",
    "corecrt_memcpy_s.h",
    "corecrt_memory.h",
    "corecrt_search.h",
    "corecrt_share.h",
    "corecrt_startup.h",
    "corecrt_stdio_config.h",
    "corecrt_terminate.h",
    "corecrt_wconio.h",
    "corecrt_wctype.h",
    "corecrt_wdirect.h",
    "corecrt_wio.h",
    "corecrt_wprocess.h",
    "corecrt_wstdio.h",
    "corecrt_wstdlib.h",
    "corecrt_wstring.h",
    "corecrt_wtime.h",
    "crtdbg.h",
    "ctype.h",
    "direct.h",
    "dos.h",
    "errno.h",
    "fcntl.h",
    "fenv.h",
    "float.h",
    "fpieee.h",
    "inttypes.h",
    "io.h",
    "locale.h",
    "malloc.h",
    "math.h",
    "mbctype.h",
    "mbstring.h",
    "memory.h",
    "minmax.h",
    "new.h",
    "process.h",
    "safeint.h",
    "safeint_internal.h",
    "search.h",
    "share.h",
    "signal.h",
    "stddef.h",
    "stdio.h",
    "stdlib.h",
    "string.h",
    "tchar.h",
    "tgmath.h",
    "time.h",
    "uchar.h",
    "wchar.h",
    "wctype.h",
    # sys/ headers
    "sys/locking.h",
    "sys/stat.h",
    "sys/timeb.h",
    "sys/types.h",
    "sys/utime.h"
)

$pass = 0
$fail = 0
$skip = 0
$results = @()

foreach ($name in $tests) {
    $safeName = ($name -replace '/', '_') -replace '\.', '_'
    $sp = "$env:TEMP\test_$safeName.sp"
    $c = "$env:TEMP\test_$safeName.c"
    $content = "/* auto */`n#include <$name>`nint main() { return 0; }"
    Set-Content $sp $content -Encoding ASCII -NoNewline

    $out = & $sharpc $sp -no-link -o $c 2>&1
    $exitCode = $LASTEXITCODE
    
    $errors = $out | Where-Object { $_ -match "error\[" }
    if ($errors.Count -gt 0) {
        $errCount = $errors.Count
        $errTypes = @{}
        foreach ($err in $errors) {
            if ($err -match "E(\d+)") {
                $code = "E$($Matches[1])"
                $errTypes[$code] = ($errTypes[$code] + 1)
            }
        }
        $errSummary = ($errTypes.GetEnumerator() | ForEach-Object { "$($_.Key): $($_.Value)" }) -join ", "
        Write-Host "FAIL: $name ($errCount errors: $errSummary)" -ForegroundColor Red
        $results += [PSCustomObject]@{ Header=$name; Status="FAIL"; Errors=$errCount; Summary=$errSummary }
        $fail++
    } else {
        Write-Host "PASS: $name" -ForegroundColor Green
        $results += [PSCustomObject]@{ Header=$name; Status="PASS"; Errors=0; Summary="" }
        $pass++
    }

    Remove-Item $sp, $c -ErrorAction SilentlyContinue
}

Write-Host ""
Write-Host "============================================================"
Write-Host "UCRT Header Test Results"
Write-Host "============================================================"
Write-Host "PASS: $pass   FAIL: $fail   SKIP: $skip   TOTAL: $($tests.Count)"
Write-Host ""
Write-Host "Failed headers:" -ForegroundColor Yellow
$results | Where-Object { $_.Status -eq "FAIL" } | Sort-Object Errors -Descending | Format-Table -AutoSize
Write-Host ""
Write-Host "Summary by error code:" -ForegroundColor Yellow
$allErrorCodes = @{}
$results | Where-Object { $_.Status -eq "FAIL" } | ForEach-Object {
    if ($_.Summary) {
        $_.Summary -split ", " | ForEach-Object {
            if ($_ -match "(E\d+):") {
                $code = $Matches[1]
                $allErrorCodes[$code] = ($allErrorCodes[$code] + 1)
            }
        }
    }
}
if ($allErrorCodes.Count -gt 0) {
    $allErrorCodes.GetEnumerator() | Sort-Object Value -Descending | ForEach-Object {
        Write-Host "  $($_.Key): $($_.Value) headers affected"
    }
}
Write-Host "============================================================"
