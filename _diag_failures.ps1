$ErrorActionPreference = "SilentlyContinue"

$failHeaders = @(
    "varargs.h", "locking.h", "stat.h", "timeb.h", "utime.h",
    "basetyps.h", "guiddef.h", "windef.h", "winnt.h"
)

foreach ($h in $failHeaders) {
    $name = $h -replace '\.', '_'
    $sp = "c:\Users\ftp\Desktop\sharp\tests\test_diag_$name.sp"
    $c = "c:\Users\ftp\Desktop\sharp\tests\test_diag_$name.c"
    $content = "/* compiler: any */
#include <$h>
int main() { return 0; }
"
    $content | Out-File -FilePath $sp -Encoding ASCII -Force
    Write-Host "=== $h ===" -ForegroundColor Cyan
    $out = & "c:\Users\ftp\Desktop\sharp\cmake-build-debug\sharpc.exe" $sp -no-link -o $c 2>&1
    $out | Where-Object { $_ -match "error:|Error" } | Select-Object -First 5
    Write-Host ""
}

# Cleanup
Get-ChildItem "c:\Users\ftp\Desktop\sharp\tests\test_diag_*.sp" -ErrorAction SilentlyContinue | Remove-Item -Force
Get-ChildItem "c:\Users\ftp\Desktop\sharp\tests\test_diag_*.c" -ErrorAction SilentlyContinue | Remove-Item -Force
