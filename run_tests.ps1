# SP Compiler Regression Test Script
# Reads /* compiler: msvc/gcc/any */ annotation from each .sp file
# Generates C code via sharpc (-no-link), compiles with the appropriate compiler, and runs the .exe

$ErrorActionPreference = "SilentlyContinue"

# Set TCC include path for the preprocessor
$env:SHARP_TCC_DIR = "$PSScriptRoot\third_party\tcc"

$SHARPC = "cmake-build-debug\sharpc.exe"
$TESTS_DIR = "tests"
$GCC = "C:\msys64\mingw64\bin\gcc.exe"
$CLANG = "C:\msys64\clang64\bin\clang.exe"

# MSVC: find cl.exe
$VS_ROOT = "C:\Program Files\Microsoft Visual Studio\2022\Community"
$VCVARS = "$VS_ROOT\VC\Auxiliary\Build\vcvarsall.bat"
$CL_EXE = ""

# Find cl.exe
$msvcPaths = @(
    "$VS_ROOT\VC\Tools\MSVC\*\bin\Hostx64\x64\cl.exe"
)
foreach ($pattern in $msvcPaths) {
    $found = Get-Item $pattern -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($found) {
        $CL_EXE = $found.FullName
        break
    }
}

$PASS = 0
$FAIL = 0

Write-Host "=== SP Compiler Regression Tests ===" -ForegroundColor Cyan
Write-Host "  GCC:   $GCC"
Write-Host "  Clang: $CLANG"
Write-Host "  MSVC:  $(if ($CL_EXE) { $CL_EXE } else { 'not found' })"
Write-Host ""

# Clean up generated test files only
Remove-Item "$TESTS_DIR\*.c" -ErrorAction SilentlyContinue
Remove-Item "$TESTS_DIR\*.exe" -ErrorAction SilentlyContinue

Write-Host "=== Running tests ===" -ForegroundColor Cyan
Write-Host ""

$spFiles = Get-ChildItem "$TESTS_DIR\test_*.sp" | Sort-Object Name

foreach ($spFile in $spFiles) {
    $name = $spFile.Name
    $baseName = $spFile.BaseName
    $cFile = "$TESTS_DIR\$baseName.c"
    $exeFile = "$TESTS_DIR\$baseName.exe"

    # Read compiler annotation
    $content = Get-Content $spFile.FullName -Raw
    $compilerTag = "any"
    if ($content -match '/\*\s*compiler:\s*(\w+)\s*\*/') {
        $compilerTag = $matches[1].ToLower()
    }

    # Select compiler for building the .exe
    $cCompiler = ""
    $compilerLabel = ""
    switch ($compilerTag) {
        "msvc" {
            if ($CL_EXE -and (Test-Path $CL_EXE)) {
                $cCompiler = "msvc"
                $compilerLabel = "msvc"
            } else {
                $cCompiler = "clang"
                $compilerLabel = "clang(fallback)"
            }
        }
        "gcc" {
            if (Test-Path $GCC) {
                $cCompiler = "gcc"
                $compilerLabel = "gcc"
            } else {
                $cCompiler = "clang"
                $compilerLabel = "clang(fallback)"
            }
        }
        default {
            # any: prefer gcc, fallback to clang
            if (Test-Path $GCC) {
                $cCompiler = "gcc"
                $compilerLabel = "gcc"
            } elseif ($CL_EXE -and (Test-Path $CL_EXE)) {
                $cCompiler = "msvc"
                $compilerLabel = "msvc"
            } else {
                $cCompiler = "clang"
                $compilerLabel = "clang"
            }
        }
    }

    Write-Host "--- $name [compiler:$compilerTag -> $compilerLabel] ---"

    # Step 1: sharpc -no-link generates .c file
    & $SHARPC $spFile.FullName -no-link -o $cFile 2>$null
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  FAIL  sharpc compilation failed" -ForegroundColor Red
        $FAIL++
        continue
    }

    if (-not (Test-Path $cFile)) {
        Write-Host "  FAIL  .c file not generated" -ForegroundColor Red
        $FAIL++
        continue
    }

    # Step 2: compile .c to .exe with selected compiler
    $compileOk = $false
    switch ($cCompiler) {
        "gcc" {
            & $GCC -std=c11 -Wall -o $exeFile $cFile 2>$null
            $compileOk = ($LASTEXITCODE -eq 0)
        }
        "clang" {
            & $CLANG -std=c11 -Wall -o $exeFile $cFile 2>$null
            $compileOk = ($LASTEXITCODE -eq 0)
        }
        "msvc" {
            # Try direct compilation first (cl.exe may work if env is set), then vcvarsall
            & $CL_EXE /nologo /O2 /Fe:$exeFile $cFile 2>$null
            if ($LASTEXITCODE -eq 0) {
                $compileOk = $true
            } else {
                # Use vcvarsall.bat to set up MSVC environment, then call cl
                $tempBat = "$env:TEMP\vs_build_$PID.bat"
                Set-Content $tempBat @"
call "$VCVARS" amd64 >nul 2>nul
"$CL_EXE" /nologo /O2 /Fe:$exeFile $cFile 2>nul
echo COMPILER_EXIT:%ERRORLEVEL%
"@
                $result = & cmd /c $tempBat 2>$null
                Remove-Item $tempBat -ErrorAction SilentlyContinue
                $compilerOutput = $result -join "`n"
                if ($compilerOutput -match 'COMPILER_EXIT:(\d+)') {
                    $compileOk = ($matches[1] -eq "0")
                }
            }
        }
    }

    if (-not $compileOk) {
        # Try clang as last resort fallback
        & $CLANG -std=c11 -Wall -o $exeFile $cFile 2>$null
        $compileOk = ($LASTEXITCODE -eq 0)
        if ($compileOk) {
            $compilerLabel = "clang(fallback)"
        }
    }

    if (-not $compileOk) {
        Write-Host "  FAIL  C->exe compilation failed ($compilerLabel)" -ForegroundColor Red
        $FAIL++
        continue
    }

    # Step 3: run the .exe and check exit code
    if (Test-Path $exeFile) {
        & $exeFile 2>$null
        $runCode = $LASTEXITCODE
        Write-Host "  PASS  compiled OK (exit=$runCode) [$compilerLabel]" -ForegroundColor Green
        $PASS++
    } else {
        Write-Host "  FAIL  .exe not produced" -ForegroundColor Red
        $FAIL++
    }
}

Write-Host ""
Write-Host "==============================" -ForegroundColor Cyan
Write-Host "PASS: $PASS   FAIL: $FAIL" -ForegroundColor Cyan
Write-Host "TOTAL: $($PASS + $FAIL)" -ForegroundColor Cyan
Write-Host "==============================" -ForegroundColor Cyan

if ($FAIL -eq 0) {
    Write-Host "ALL TESTS PASSED" -ForegroundColor Green
} else {
    Write-Host "SOME TESTS FAILED" -ForegroundColor Red
}

# Clean up generated test files only
Remove-Item "$TESTS_DIR\*.c" -ErrorAction SilentlyContinue
Remove-Item "$TESTS_DIR\*.exe" -ErrorAction SilentlyContinue

exit $FAIL
