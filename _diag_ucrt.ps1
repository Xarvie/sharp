$ErrorActionPreference = 'Continue'

$ucrtInclude = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt"
$failHeaders = @("complex.h", "corecrt.h", "corecrt_malloc.h", "corecrt_math.h", "corecrt_memory.h", "corecrt_wstdio.h", "corecrt_wstring.h", "crtdbg.h", "fpieee.h", "mbstring.h", "new.h", "safeint.h", "search.h", "stdalign.h", "stdnoreturn.h", "tgmath.h", "uchar.h")

foreach ($h in $failHeaders[0..6]) {
    $name = $h -replace '\.', '_'
    $sp = "c:\Users\ftp\Desktop\sharp\tests\u_$name.sp"
    $c = "c:\Users\ftp\Desktop\sharp\tests\u_$name.c"
    $content = "/* auto */`n#include <$h>`nint main() { return 0; }"
    Set-Content $sp $content -Encoding ASCII -NoNewline

    Write-Host "`n=== $h ===" -ForegroundColor Cyan
    $out = & "c:\Users\ftp\Desktop\sharp\cmake-build-debug\sharpc.exe" $sp -no-link -o $c 2>&1
    if ($LASTEXITCODE -ne 0) {
        $out | Where-Object { $_ -match "error:|Error" } | Select-Object -First 2
    }
}
