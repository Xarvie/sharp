$ErrorActionPreference = "SilentlyContinue"

$ucrtInclude = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt"
$pass = 0
$fail = 0
$skip = 0

if (-not (Test-Path $ucrtInclude)) {
    Write-Host "UCRT include not found at $ucrtInclude"
    Write-Host "Searching..."
    $found = Get-ChildItem "C:\Program Files (x86)\Windows Kits\10\Include" -Recurse -Directory -Filter "ucrt" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($found) {
        $ucrtInclude = $found.FullName
        Write-Host "Found: $ucrtInclude"
    } else {
        Write-Host "UCRT not found"
        exit
    }
}

Write-Host "UCRT Include: $ucrtInclude" -ForegroundColor Cyan
Write-Host ""

$headers = Get-ChildItem $ucrtInclude -File -Filter "*.h" | Sort-Object Name

foreach ($f in $headers) {
    $name = $f.Name
    $safeName = $name -replace '\.', '_'
    $sp = "c:\Users\ftp\Desktop\sharp\tests\ucrt_$safeName.sp"
    $c  = "c:\Users\ftp\Desktop\sharp\tests\ucrt_$safeName.c"
    $exe = "c:\Users\ftp\Desktop\sharp\tests\ucrt_$safeName.exe"

    $spContent = "/* auto */`n#include <$name>`nint main() { return 0; }"
    Set-Content $sp $spContent -Encoding ASCII -NoNewline

    # Step 1: Compile .sp -> .c
    $out1 = & "c:\Users\ftp\Desktop\sharp\cmake-build-debug\sharpc.exe" $sp -no-link -o $c 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FAIL sharpc: $name"
        $fail++
        continue
    }

    # Step 2: Compile .c -> .exe with clang
    $out2 = & clang -std=c11 -w -I"$ucrtInclude" -o $exe $c 2>&1
    if ($LASTEXITCODE -ne 0) {
        $err = $out2 | Where-Object { $_ -match "error:" } | Select-Object -First 1
        if ($err -match "No such file or directory") {
            Write-Host "SKIP missing: $name"
            $skip++
        } else {
            Write-Host "FAIL clang:  $name"
            $fail++
        }
        continue
    }

    Write-Host "PASS:      $name"
    $pass++
}

Write-Host ""
Write-Host "=============================="
Write-Host "PASS: $pass   FAIL: $fail   SKIP: $skip"
Write-Host "=============================="

# Cleanup
Get-ChildItem "c:\Users\ftp\Desktop\sharp\tests\ucrt_*.sp" -ErrorAction SilentlyContinue | Remove-Item -Force
Get-ChildItem "c:\Users\ftp\Desktop\sharp\tests\ucrt_*.c" -ErrorAction SilentlyContinue | Remove-Item -Force
Get-ChildItem "c:\Users\ftp\Desktop\sharp\tests\ucrt_*.exe" -ErrorAction SilentlyContinue | Remove-Item -Force
