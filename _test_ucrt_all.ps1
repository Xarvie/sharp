$ErrorActionPreference = "SilentlyContinue"

$ucrtDir = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt"
$sharpc = "c:\Users\ftp\Desktop\sharp\cmake-build-debug\sharpc.exe"

# Only test user-includable headers (exclude internal corecrt_* headers)
$pass = 0
$fail = 0
$failList = @()

$headers = Get-ChildItem $ucrtDir -File -Filter "*.h" |
    Where-Object { $_.Name -notmatch "^corecrt_" } |
    Sort-Object Name

foreach ($f in $headers) {
    $name = $f.Name
    $sp = "$env:TEMP\ucrt_$($name -replace '\.', '_').sp"
    $c  = "$env:TEMP\ucrt_$($name -replace '\.', '_').c"

    $content = "/* auto */`n#include <$name>`nint main() { return 0; }"
    Set-Content $sp $content -Encoding ASCII -NoNewline

    $out = & $sharpc $sp -no-link -o $c 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Host "PASS: $name" -ForegroundColor Green
        $pass++
    } else {
        $err = $out | Where-Object { $_ -match "error\[E\d+\]" } | Select-Object -First 1
        Write-Host "FAIL: $name - $err" -ForegroundColor Red
        $fail++
        $failList += @{name=$name; error=$err}
    }

    Remove-Item $sp, $c -ErrorAction SilentlyContinue
}

Write-Host ""
Write-Host "==============================" -ForegroundColor Cyan
Write-Host "PASS: $pass   FAIL: $fail   TOTAL: $($headers.Count)" -ForegroundColor Cyan
Write-Host "==============================" -ForegroundColor Cyan

if ($failList.Count -gt 0) {
    Write-Host "`nFailed headers:" -ForegroundColor Yellow
    $failList | ForEach-Object { Write-Host "  $($_.name) -> $($_.error)" }
}
