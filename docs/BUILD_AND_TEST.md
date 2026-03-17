# Strata Build and Test Guide

This document describes how to reliably build and test the current components of the Strata repository. 

As of the current phase, Strata consists of bounded implementation slices, including the `bench_convert` tool, the `Breadboard` skeleton, and the `Forge` skeleton. 

> **Important**: `Forge` and `Breadboard` are currently skeleton API boundaries. They are **not** full backend runtimes or compilers yet. The tests validate their public boundaries, error paths, and capability reporting, but do not execute genuine circuit simulation.

---

## 1. Quick Start: The Helper Script

For daily development and verification, you can run all local checks using the included PowerShell helper script.

```bash
# Must be executed from the repository root
powershell -ExecutionPolicy Bypass -File tools\run_checks.ps1
```

**What it does:**
- Validates it is being run from the repository root.
- Navigates to `tools/bench_convert` to build and test the benchmarking tool.
- Navigates to `tests/breadboard` to build and test the Breadboard compiler skeleton.
- Navigates to `tests/forge` to build and test the Forge API skeleton.
- Navigates to `tests/integration` to build and test the Breadboard-to-Forge handoff.
- Stops immediately if any build or test fails, printing clear pass/fail status.
- Does not reach into external repositories and does not mutate tracked source files.
- Cleans generated `.exe` outputs after each lane.
- Bench converter tests temporarily regenerate files under `tests/tools/bench_convert/fixtures/actual/`; the helper removes those generated files after validation and preserves the ignored placeholder in that directory.

---

## 2. Manual Commands

If you need to work within a specific component, you can use the localized Makefiles.

### tools/bench_convert

This tool parses subsets of Verilog/BLIF into generic `.bench` format. 

**Location:** `tools/bench_convert`

**Commands:**
- **Build:** `make`
- **Test:** `make test`  
  *(Note: Tests invoke `tests/tools/bench_convert/test_runner.ps1` to validate fixtures and ensure unsupported input files fail cleanly.)*
- **Clean:** `make clean`  
  *(Removes `.o` files and the generated `.exe`)*

**Expected Pass Output:**
A successful test run prints output from the PowerShell test runner, indicating passes for deterministic golden tests and malformed input failure tests, ending with:
```
All tests passed successfully!
```

### tests/breadboard

The `Breadboard` tests compile the structural compiler's skeleton API to verify its diagnostics and boundary.

**Location:** `tests/breadboard`

**Commands:**
- **Build:** `make`  
  *(Builds Breadboard test executables.)*
- **Test:** `make test`  
  *(Runs Breadboard tests.)*
- **Clean:** `make clean`  
  *(Removes all generated `.exe` files.)*

**Expected Pass Output:**
A successful test run outputs:
```
--- Running Breadboard skeleton tests ---
[Test outputs]
--- All Breadboard skeleton tests passed ---
```

### tests/forge

The `Forge` tests compile the `Forge` API capabilities as a standalone application to verify boundary opacity and deterministic failure paths.

**Location:** `tests/forge`

**Commands:**
- **Build:** `make`  
  *(Builds the current Forge test executables.)*
- **Test:** `make test`  
  *(Runs the current Forge executables sequentially.)*
- **Clean:** `make clean`  
  *(Removes all generated `.exe` files.)*

**Expected Pass Output:**
A successful test run outputs:
```
--- Running Forge skeleton tests ---
[Test outputs for capabilities, backend info, invalid handles, and artifacts]
--- All Forge skeleton tests passed ---
```

### tests/integration

The integration tests validate the handoff between the Breadboard skeleton and the Forge runtime API.

> **Note**: The current Breadboard -> Forge handoff is a temporary placeholder contract used to validate integration plumbing. It is **not** the final Strata artifact format.

**Location:** `tests/integration`

**Commands:**
- **Build:** `make`  
  *(Builds integration test executables.)*
- **Test:** `make test`  
  *(Runs integration tests.)*
- **Clean:** `make clean`  
  *(Removes all generated `.exe` files.)*

**Expected Pass Output:**
A successful test run outputs:
```
--- Running integration tests ---
[Test outputs for handoff and validation]
--- All integration tests passed ---
```

---

## 3. Limitations and Missing Pieces

- **No Global Build System:** There is currently no unified cross-platform build system (like CMake) across the entire codebase. Makefiles are distributed locally.
- **Stub Status:** The `Forge` session execution and artifact parsing are currently implemented as explicit stubs reflecting "not implemented" failure paths or stubbed successful parses.
- **Windows Only Helper:** `run_checks.ps1` requires PowerShell and Windows conventions. A Unix-equivalent generic wrapper has not been introduced yet.
