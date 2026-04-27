$ErrorActionPreference = 'Continue'
$ucrtDir = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt"
$sharpc = "c:\Users\ftp\Desktop\sharp\cmake-build-debug\sharpc.exe"

# Test representative failures with full error output
$tests = @(
    @{hdr='complex.h'; desc='C11 complex numbers'},
    @{hdr='corecrt_math.h'; desc='Internal math declarations'},
    @{hdr='corecrt_wstdio.h'; desc='Wide char stdio'},
    @{hdr='crtdbg.h'; desc='CRT debug'},
    @{hdr='search.h'; desc='BSD search functions'},
    @{hdr='stdalign.h'; desc='C11 alignas/alignof'},
    @{hdr='stdnoreturn.h'; desc='C11 noreturn'},
    @{hdr='tgmath.h'; desc='Type-generic math'},
    @{hdr='uchar.h'; desc='C11 Unicode chars'},
    @{hdr='mbstring.h'; desc='Multi-byte string'},
    @{hdr='fpieee.h'; desc='IEEE FP exceptions'},
    @{hdr='new.h'; desc='C++ new/delete'}
)

foreach ($t in $tests) {
    $name = $t.hdr
    $sp = "$env:TEMP\diag_$($name -replace '\.', '_').sp"
    $c  = "$env:TEMP\diag_$($name -replace '\.', '_').c"
    $content = "/* auto */`n#include <$name>`nint main() { return 0; }"
    Set-Content $sp $content -Encoding ASCII -NoNewline

    Write-Host "`n=== $name ($($t.desc)) ===" -ForegroundColor Cyan
    $out = & $sharpc $sp -no-link -o $c 2>&1
    $errors = $out | Where-Object { $_ -match "error\[E\d+\]" }
    if ($errors) {
        $errors | Select-Object -First 3 | ForEach-Object { Write-Host "  $_" }
    } else {
        Write-Host "  (no parse error, may be clang issue)"
    }

    Remove-Item $sp, $c -ErrorAction SilentlyContinue
}
