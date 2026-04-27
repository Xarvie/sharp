$ucrtDir = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt"
$sharpc = "c:\Users\ftp\Desktop\sharp\cmake-build-debug\sharpc.exe"

# Test with preprocess-only to see the actual preprocessed output
$name = "complex.h"
$sp = "$env:TEMP\pp_complex.sp"
$content = "/* auto */`n#include <$name>`nint main() { return 0; }"
Set-Content $sp $content -Encoding ASCII -NoNewline

# Try to get preprocessed output
try {
    & $sharpc $sp -preprocess -o "$env:TEMP\pp_complex.c" 2>$null
    if (Test-Path "$env:TEMP\pp_complex.c") {
        $lines = Get-Content "$env:TEMP\pp_complex.c"
        Write-Host "Total lines: $($lines.Count)"
        $lines[620..635] | ForEach-Object { Write-Host $_ }
    } else {
        Write-Host "No preprocessed output generated"
    }
} catch {
    Write-Host "Error: $_"
}

Remove-Item $sp, "$env:TEMP\pp_complex.c" -ErrorAction SilentlyContinue
