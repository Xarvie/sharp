$ErrorActionPreference = "SilentlyContinue"

$pass = 0
$fail = 0

$tests = Get-ChildItem "c:\Users\ftp\Desktop\sharp\tests\test_*.sp" | Sort-Object Name

foreach ($f in $tests) {
    $name = $f.BaseName
    $sp = $f.FullName
    $c = "c:\Users\ftp\Desktop\sharp\tests\$name.c"
    $exe = "c:\Users\ftp\Desktop\sharp\tests\$name.exe"

    # Step 1: Compile .sp -> .c
    $out1 = & "c:\Users\ftp\Desktop\sharp\cmake-build-debug\sharpc.exe" $sp -no-link -o $c 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FAIL sharpc: $name"
        $fail++
        continue
    }

    # Step 2: Compile .c -> .exe with clang
    $out2 = & clang -std=c11 -w -o $exe $c 2>&1
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
Write-Host "PASS: $pass   FAIL: $fail"
Write-Host "=============================="
