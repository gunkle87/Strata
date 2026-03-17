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

function Invoke-Cleanup {
    param(
        [string]$RelativePath,
        [string]$Label
    )

    Push-Location $RelativePath
    $cleanOutput = & make clean 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[WARN] $Label clean failed"
        Write-Host $cleanOutput
    }
    Pop-Location
}

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
Invoke-Cleanup "tools\bench_convert" "bench_convert"

$actualDir = Join-Path $ROOT_DIR "tests\tools\bench_convert\fixtures\actual"
Get-ChildItem -Path $actualDir -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -ne ".gitignore" } |
    Remove-Item -Force

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
Invoke-Cleanup "tests\forge" "forge"

if ($failed) {
    Write-Host "Stopping further checks due to failure."
    exit 1
}

# 3. tests/breadboard
Write-Host "--- 3. tests/breadboard ---"
Push-Location "tests\breadboard"

Write-Host "Building breadboard skeleton tests..."
$makeBuild3 = & make 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "[FAIL] breadboard skeleton build failed"
    Write-Host $makeBuild3
    $failed = $true
} else {
    Write-Host "[PASS] breadboard skeleton build"
    
    Write-Host "Testing breadboard skeleton..."
    $makeTest3 = & make test 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[FAIL] breadboard skeleton tests failed"
        Write-Host $makeTest3
        $failed = $true
    } else {
        Write-Host "[PASS] breadboard skeleton tests"
    }
}
Pop-Location

Write-Host ""
Invoke-Cleanup "tests\breadboard" "breadboard"

if ($failed) {
    Write-Host "Stopping further checks due to failure."
    exit 1
}

# 4. tests/integration
Write-Host "--- 4. tests/integration ---"
Push-Location "tests\integration"

Write-Host "Building integration tests..."
$makeBuild4 = & make 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "[FAIL] integration tests build failed"
    Write-Host $makeBuild4
    $failed = $true
} else {
    Write-Host "[PASS] integration tests build"
    
    Write-Host "Testing integration..."
    $makeTest4 = & make test 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[FAIL] integration tests failed"
        Write-Host $makeTest4
        $failed = $true
    } else {
        Write-Host "[PASS] integration tests"
    }
}
Pop-Location

Write-Host ""
Invoke-Cleanup "tests\integration" "integration"

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
