$ErrorActionPreference = "SilentlyContinue"

$includeDir = "C:\Users\ftp\Desktop\sharp\cmake-build-debug\third_party\tcc\include"
$pass = 0
$fail = 0
$skip = 0

# Get all .h files recursively
$headers = Get-ChildItem $includeDir -Recurse -File -Filter "*.h" | Where-Object { $_.Directory.Name -ne "sec_api" -and $_.Directory.Name -ne "sys" -and $_.Directory.Name -ne "tcc" -and $_.Directory.Name -ne "winapi" } | Sort-Object Name

# Also include some subdirectory ones separately
$subHeaders = Get-ChildItem $includeDir -Recurse -File -Filter "*.h" | Where-Object { $_.Directory.Name -eq "sys" -or $_.Directory.Name -eq "winapi" } | Sort-Object Name

$allHeaders = @($headers) + @($subHeaders) | Sort-Object Name -Unique

foreach ($f in $allHeaders) {
    $name = $f.Name
    $headerPath = $f.FullName

    # Create a test .sp file that just includes this header
    $sp = "c:\Users\ftp\Desktop\sharp\tests\test_tcc_$($name -replace '\.','_').sp"
    $c = "c:\Users\ftp\Desktop\sharp\tests\test_tcc_$($name -replace '\.','_').c"
    $exe = "c:\Users\ftp\Desktop\sharp\tests\test_tcc_$($name -replace '\.','_').exe"

    # Write a minimal test file
    $spContent = "/* compiler: any */
/* Auto-generated test for tcc include: $name */

#include <$name>

int main() {
    return 0;
}
"
    $spContent | Out-File -FilePath $sp -Encoding ASCII -Force

    # Step 1: Compile .sp -> .c
    $out1 = & "c:\Users\ftp\Desktop\sharp\cmake-build-debug\sharpc.exe" $sp -no-link -o $c 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FAIL sharpc: $name"
        $fail++
        continue
    }

    # Step 2: Compile .c -> .exe with clang
    $out2 = & clang -std=c11 -w -I"$includeDir" -o $exe $c 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FAIL clang:  $name"
        $fail++
        continue
    }

    Write-Host "PASS:      $name"
    $pass++
}

Write-Host ""
Write-Host "=============================="
Write-Host "PASS: $pass   FAIL: $fail   SKIP: $skip"
Write-Host "=============================="

# Cleanup generated test files
Get-ChildItem "c:\Users\ftp\Desktop\sharp\tests\test_tcc_*.sp" -ErrorAction SilentlyContinue | Remove-Item -Force
Get-ChildItem "c:\Users\ftp\Desktop\sharp\tests\test_tcc_*.c" -ErrorAction SilentlyContinue | Remove-Item -Force
Get-ChildItem "c:\Users\ftp\Desktop\sharp\tests\test_tcc_*.exe" -ErrorAction SilentlyContinue | Remove-Item -Force
