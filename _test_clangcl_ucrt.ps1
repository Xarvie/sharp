$ucrt = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt"
$msvc = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\include"
$shared = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\shared"
$um = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um"

Write-Host "=== Testing clang-cl with full UCRT+MSVC headers ===" -ForegroundColor Cyan
$pass = 0; $fail = 0

$headers = Get-ChildItem $ucrt -File -Filter "*.h" | Sort-Object Name

foreach ($f in $headers) {
    $h = $f.Name
    $tmp = "$env:TEMP\test_ucrt_$h"
    $code = "#include <$h>`nint main() { return 0; }"
    Set-Content "$tmp.c" $code -Encoding ASCII -NoNewline
    
    $out = clang-cl /nologo /w /I"$ucrt" /I"$msvc" /I"$shared" /I"$um" /c "$tmp.c" 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Host "PASS: $h"
        $pass++
    } else {
        $err = $out | Where-Object { $_ -match "error" } | Select-Object -First 1
        if ($err -match "not supported|unsupported") {
            Write-Host "SKIP: $h"
        } else {
            Write-Host "FAIL: $h"
            $fail++
        }
    }
}

Write-Host "`n=============================="
Write-Host "PASS: $pass   FAIL: $fail"
Write-Host "=============================="

Remove-Item "$env:TEMP\test_ucrt_*.c","$env:TEMP\test_ucrt_*.obj" -ErrorAction SilentlyContinue
