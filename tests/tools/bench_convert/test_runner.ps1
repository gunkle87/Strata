$ErrorActionPreference = "Continue"

$ROOT_DIR = (Resolve-Path (Join-Path $PSScriptRoot "..\..\..")).Path
$BIN = Join-Path $ROOT_DIR "tools\bench_convert\bench_convert.exe"
$FIXTURES_DIR = Join-Path $ROOT_DIR "tests\tools\bench_convert\fixtures"
$ACTUAL_DIR = Join-Path $FIXTURES_DIR "actual"

if (!(Test-Path $ACTUAL_DIR)) { New-Item -ItemType Directory -Path $ACTUAL_DIR | Out-Null }
Get-ChildItem $ACTUAL_DIR -File -ErrorAction SilentlyContinue | Remove-Item -Force

# CLI Smoke Tests
Write-Host "Running CLI smoke tests..."
$res = &$BIN 2>&1
if ($LASTEXITCODE -eq 0) { throw "Expected failure with no args" }

$res = &$BIN --help 2>&1
if ($LASTEXITCODE -ne 0) { throw "Expected success with --help" }

$res = &$BIN --version 2>&1
if ($LASTEXITCODE -ne 0) { throw "Expected success with --version" }

$stdoutOut = "$ACTUAL_DIR\stdout_cli.bench"
$stdoutRes = &$BIN --input "$FIXTURES_DIR\inputs\blif\buf.blif" --output $stdoutOut --from blif --overwrite replace --stdout 2>&1
if ($LASTEXITCODE -ne 0) { throw "Expected success with --stdout" }
if (($stdoutRes | Out-String) -notmatch "INPUT\(a\)") { throw "Expected BENCH content on stdout" }

$reportOut = "$ACTUAL_DIR\report_cli.bench"
$reportPath = "$ACTUAL_DIR\report_cli.txt"
$null = &$BIN --input "$FIXTURES_DIR\inputs\blif\buf.blif" --output $reportOut --from blif --overwrite replace --report $reportPath 2>&1
if ($LASTEXITCODE -ne 0) { throw "Expected success with --report" }
if (!(Test-Path $reportPath)) { throw "Expected report file to be created" }
if ((Get-Content $reportPath | Out-String) -notmatch "status=success") { throw "Expected success report content" }

# Function to run converter and check against golden
function Check-Golden($testName, $inFile, $goldenFile, $format) {
    $outFile = "$FIXTURES_DIR\actual\$testName.bench"
    $res = &$BIN --input $inFile --output $outFile --from $format --overwrite replace 2>&1
    if ($LASTEXITCODE -ne 0) { 
        throw "Failed to convert $inFile. output: $res"
    }
    $actualBytes = [System.IO.File]::ReadAllBytes($outFile)
    $expectedBytes = [System.IO.File]::ReadAllBytes($goldenFile)
    
    # Strip carriage returns to compare UNIX/DOS line endings safely
    $actualNoCr = @()
    foreach ($b in $actualBytes) { if ($b -ne 13) { $actualNoCr += $b } }
    $expectedNoCr = @()
    foreach ($b in $expectedBytes) { if ($b -ne 13) { $expectedNoCr += $b } }
    
    # Also trim trailing newlines
    while ($actualNoCr.Length -gt 0 -and $actualNoCr[-1] -eq 10) { $actualNoCr = $actualNoCr[0..($actualNoCr.Length-2)] }
    while ($expectedNoCr.Length -gt 0 -and $expectedNoCr[-1] -eq 10) { $expectedNoCr = $expectedNoCr[0..($expectedNoCr.Length-2)] }
    
    $same = $true
    if ($actualNoCr.Length -ne $expectedNoCr.Length) {
        $same = $false
    } else {
        for ($i = 0; $i -lt $actualNoCr.Length; $i++) {
            if ($actualNoCr[$i] -ne $expectedNoCr[$i]) { $same = $false; break; }
        }
    }
    
    if (-not $same) {
        $aStr = [System.Text.Encoding]::UTF8.GetString([byte[]]$actualNoCr)
        $eStr = [System.Text.Encoding]::UTF8.GetString([byte[]]$expectedNoCr)
        throw "Mismatch for $testName! Lengths ($($actualNoCr.Length) vs $($expectedNoCr.Length))`nExpected:`n[$eStr]`nActual:`n[$aStr]"
    }
    Write-Host "  [PASS] $testName"
}

# Function to run converter expecting failure
function Check-Fail($testName, $inFile, $format) {
    $outFile = "$FIXTURES_DIR\actual\fail_$testName.bench"
    $res = &$BIN --input $inFile --output $outFile --from $format --overwrite replace 2> $null
    if ($LASTEXITCODE -eq 0) {
        throw "Expected failure for $inFile but it succeeded"
    }
    if (!(Test-Path $outFile)) {
        Write-Host "  [PASS] $testName (Failed as expected, no output file)"
    } else {
        throw "File $outFile was created despite failure!"
    }
}

Write-Host "Running deterministic golden tests..."
Check-Golden "blif_buf" "$FIXTURES_DIR\inputs\blif\buf.blif" "$FIXTURES_DIR\golden\blif_buf.bench" "blif"
Check-Golden "blif_or" "$FIXTURES_DIR\inputs\blif\or.blif" "$FIXTURES_DIR\golden\blif_or.bench" "blif"
Check-Golden "verilog_alias_output" "$FIXTURES_DIR\inputs\verilog\alias_output.v" "$FIXTURES_DIR\golden\verilog_alias_output.bench" "verilog"
Check-Golden "verilog_and" "$FIXTURES_DIR\inputs\verilog\and_gate.v" "$FIXTURES_DIR\golden\verilog_and.bench" "verilog"
Check-Golden "verilog_vector" "$FIXTURES_DIR\inputs\verilog\vector_or.v" "$FIXTURES_DIR\golden\verilog_vector.bench" "verilog"

Write-Host "Running malformed and unsupported tests..."
Check-Fail "blif_const_fail" "$FIXTURES_DIR\inputs\blif\const_fail.blif" "blif"
Check-Fail "blif_gate_fail" "$FIXTURES_DIR\inputs\blif\gate_fail.blif" "blif"
Check-Fail "blif_missing_end_fail" "$FIXTURES_DIR\inputs\blif\missing_end_fail.blif" "blif"
Check-Fail "blif_decl_conflict_fail" "$FIXTURES_DIR\inputs\blif\decl_conflict_fail.blif" "blif"
Check-Fail "blif_multi_model_fail" "$FIXTURES_DIR\inputs\blif\multi_model_fail.blif" "blif"

Check-Fail "verilog_behavior_fail" "$FIXTURES_DIR\inputs\verilog\behavior_fail.v" "verilog"
Check-Fail "verilog_decl_conflict_fail" "$FIXTURES_DIR\inputs\verilog\decl_conflict_fail.v" "verilog"

Write-Host ""
Write-Host "All tests passed successfully!"
