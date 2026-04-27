$ErrorActionPreference = "SilentlyContinue"

$testFiles = @(
    @{header="varargs.h"; sp="c:\Users\ftp\Desktop\sharp\tests\diag_varargs.sp"; c="c:\Users\ftp\Desktop\sharp\tests\diag_varargs.c"},
    @{header="locking.h"; sp="c:\Users\ftp\Desktop\sharp\tests\diag_locking.sp"; c="c:\Users\ftp\Desktop\sharp\tests\diag_locking.c"},
    @{header="stat.h"; sp="c:\Users\ftp\Desktop\sharp\tests\diag_stat.sp"; c="c:\Users\ftp\Desktop\sharp\tests\diag_stat.c"},
    @{header="timeb.h"; sp="c:\Users\ftp\Desktop\sharp\tests\diag_timeb.sp"; c="c:\Users\ftp\Desktop\sharp\tests\diag_timeb.c"},
    @{header="utime.h"; sp="c:\Users\ftp\Desktop\sharp\tests\diag_utime.sp"; c="c:\Users\ftp\Desktop\sharp\tests\diag_utime.c"}
)

foreach ($t in $testFiles) {
    $content = "/* compiler: any */`n#include <$($t.header)>`nint main() { return 0; }`n"
    $content | Out-File -FilePath $t.sp -Encoding ASCII -Force
}

foreach ($t in $testFiles) {
    Write-Host "=== $($t.header) ===" -ForegroundColor Cyan
    $out = & "c:\Users\ftp\Desktop\sharp\cmake-build-debug\sharpc.exe" $t.sp -no-link -o $t.c 2>&1
    if ($LASTEXITCODE -ne 0) {
        $out | Where-Object { $_ -ne "" } | Select-Object -First 3
    } else {
        Write-Host "  OK"
    }
    Write-Host ""
}
