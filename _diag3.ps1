$ErrorActionPreference = 'Continue'

$tests = @(
    @{hdr='sec_api/crtdbg_s.h'; name='crtdbg'},
    @{hdr='sec_api/mbstring_s.h'; name='mbstring'},
    @{hdr='sec_api/search_s.h'; name='search'},
    @{hdr='sec_api/stralign_s.h'; name='stralign'},
    @{hdr='varargs.h'; name='varargs'},
    @{hdr='winapi/winbase.h'; name='winbase'},
    @{hdr='winapi/windef.h'; name='windef'},
    @{hdr='winapi/windows.h'; name='windows'},
    @{hdr='winapi/winnt.h'; name='winnt'},
    @{hdr='winapi/winuser.h'; name='winuser'}
)

foreach ($t in $tests) {
    $sp = "c:\Users\ftp\Desktop\sharp\tests\d_$($t.name).sp"
    $c = "c:\Users\ftp\Desktop\sharp\tests\d_$($t.name).c"
    $content = "/* auto */`n#include <$($t.hdr)>`nint main() { return 0; }"
    Set-Content $sp $content -Encoding ASCII -NoNewline

    Write-Host "`n=== $($t.hdr) ===" -ForegroundColor Cyan
    $out = & "c:\Users\ftp\Desktop\sharp\cmake-build-debug\sharpc.exe" $sp -no-link -o $c 2>&1
    if ($LASTEXITCODE -ne 0) {
        $out | Where-Object { $_ -ne "" -and $_ -notmatch "At line|CategoryInfo|FullyQualified" } | Select-Object -First 3
    }
}
