$ErrorActionPreference = "SilentlyContinue"

$includeDir = "C:\Users\ftp\Desktop\sharp\cmake-build-debug\third_party\tcc\include"
$pass = 0
$fail = 0

$headers = Get-ChildItem $includeDir -Recurse -File -Filter "*.h" | Where-Object { $_.Name -ne "tcclib.h" } | Sort-Object Name

foreach ($f in $headers) {
    $name = $f.Name
    $headerPath = $f.FullName
    $relPath = $f.FullName.Replace($includeDir + "\", "").Replace("\", "/")

    # Create a test .sp file that just includes this header
    $safeName = $name -replace '\.', '_'
    if ($f.Directory.Name -ne "include") {
        $safeName = "$($f.Directory.Name)_$safeName"
    }
    $sp = "c:\Users\ftp\Desktop\sharp\tests\th_$safeName.sp"
    $c  = "c:\Users\ftp\Desktop\sharp\tests\th_$safeName.c"
    $exe = "c:\Users\ftp\Desktop\sharp\tests\th_$safeName.exe"

    $spContent = "/* auto */`n#include <$relPath>`nint main() { return 0; }"
    Set-Content $sp $spContent -Encoding ASCII -NoNewline

    # Step 1: Compile .sp -> .c
    $out1 = & "c:\Users\ftp\Desktop\sharp\cmake-build-debug\sharpc.exe" $sp -no-link -o $c 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FAIL sharpc: $relPath"
        $fail++
        continue
    }

    # Step 2: Compile .c -> .exe with clang
    $out2 = & clang -std=c11 -w -I"$includeDir" -o $exe $c 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FAIL clang:  $relPath"
        $fail++
        continue
    }

    Write-Host "PASS:      $relPath"
    $pass++
}

Write-Host ""
Write-Host "=============================="
Write-Host "PASS: $pass   FAIL: $fail"
Write-Host "=============================="

# Cleanup
Get-ChildItem "c:\Users\ftp\Desktop\sharp\tests\th_*.sp" -ErrorAction SilentlyContinue | Remove-Item -Force
Get-ChildItem "c:\Users\ftp\Desktop\sharp\tests\th_*.c" -ErrorAction SilentlyContinue | Remove-Item -Force
Get-ChildItem "c:\Users\ftp\Desktop\sharp\tests\th_*.exe" -ErrorAction SilentlyContinue | Remove-Item -Force
