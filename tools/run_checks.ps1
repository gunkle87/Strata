$ErrorActionPreference = "Continue"

$ROOT_DIR = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$CURRENT_DIR = (Get-Location).Path

if ($CURRENT_DIR -ne $ROOT_DIR) {
    Write-Host "ERROR: run_checks.ps1 must be run from the repository root."
    Write-Host "Expected: $ROOT_DIR"
    Write-Host "Current : $CURRENT_DIR"
    exit 1
}

Write-Host "========================================"
Write-Host "       Strata Build/Test Helper         "
Write-Host "========================================"
Write-Host ""

$failed = $false

# 1. tools/bench_convert
Write-Host "--- 1. tools/bench_convert ---"
Push-Location "tools\bench_convert"

Write-Host "Building bench_convert..."
$makeBuild = & make 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "[FAIL] bench_convert build failed"
    Write-Host $makeBuild
    $failed = $true
} else {
    Write-Host "[PASS] bench_convert build"
    
    Write-Host "Testing bench_convert..."
    $makeTest = & make test 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[FAIL] bench_convert tests failed"
        Write-Host $makeTest
        $failed = $true
    } else {
        Write-Host "[PASS] bench_convert tests"
    }
}
Pop-Location

Write-Host ""
if ($failed) {
    Write-Host "Stopping further checks due to failure."
    exit 1
}

# 2. tests/forge
Write-Host "--- 2. tests/forge ---"
Push-Location "tests\forge"

Write-Host "Building forge skeleton tests..."
$makeBuild2 = & make 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "[FAIL] forge skeleton build failed"
    Write-Host $makeBuild2
    $failed = $true
} else {
    Write-Host "[PASS] forge skeleton build"
    
    Write-Host "Testing forge skeleton..."
    $makeTest2 = & make test 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[FAIL] forge skeleton tests failed"
        Write-Host $makeTest2
        $failed = $true
    } else {
        Write-Host "[PASS] forge skeleton tests"
    }
}
Pop-Location

Write-Host ""

if ($failed) {
    Write-Host "========================================"
    Write-Host "              CHECKS FAILED             "
    Write-Host "========================================"
    exit 1
} else {
    Write-Host "========================================"
    Write-Host "           ALL CHECKS PASSED            "
    Write-Host "========================================"
    exit 0
}
