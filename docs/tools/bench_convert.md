# `bench_convert` (Strata Tool)

`bench_convert` is a standalone utility within the Strata architecture that converts supported source netlist dialects into canonical generic `.bench` format.

## Architectural Context
This tool operates strictly at the **tooling layer**. It has no dependencies on the Breadboard builder, Forge compiler, or any backend execution engines. Its primary role is to act as a frontend normalization phase, allowing users to cleanly translate external files (like Verilog and BLIF) into the stable `.bench` format utilized by downward layers.

## Supported Input Formats
- **BLIF (`.blif`)**: Restricted to a single `.model` comprising solely of `.inputs`, `.outputs`, and `.names` combinational logic. Sophisticated logic (like constants, sub-circuits, mixed ON/OFF sets, latches) are not supported.
- **Verilog (`.v`)**: Flat structural continuous models. The reader handles primitive gate instantiations and basic assign aliases (e.g., `assign y = ~x;`). Vectors are flattened automatically (e.g., `wire [1:0] a;` to `a[1]`, `a[0]`). Behavioral code (`always`, `initial`, constructs involving delays) will throw parsing errors.

`bench_convert` enforces a strict subset: it is *not* a logic synthesizer, elaboration engine, or full front-end parser for arbitrary HDLs.

## Command Line Interface

```bash
bench_convert --input <path> --output <path> [options]
```

### Required Arguments
- `--input <path>`: Source file to securely parse.
- `--output <path>`: Destination path for the generated `.bench` file.

### Recommended Arguments
- `--from <format>`: Can be `blif` or `verilog`. By default, the tool infers the source format from the input filename extension.
- `--overwrite <policy>`: Sets the overwrite policy for the output destination (`error`, `replace`). Defaults to `error`.
- `--stdout`: Prints the canonical `.bench` output to standard output after a successful write.
- `--report <path>`: Writes a simple deterministic conversion report with input, output, format, and object counts.
- `--strict`: Reserved for future warning-policy expansion. Current behavior is strict by default, so the flag is accepted but does not change behavior yet.

## Current Report Contents
The current `--report` output is a simple key-value text file containing:
- success status
- input path
- output path
- input format
- strict mode state
- input count
- output count
- gate count
- warning count

## Exit Codes
The tool is designed for clean integration into deterministic pipelines.
- **`0`**: Success. Outputs are generated completely.
- **`1`**: A controlled parser or CLI error occurred. Clear source line-aware diagnostics will be emitted to `stderr`. No partial output files are left behind.
- **`>1`**: Unhandled fatal error.

## Test Strategy
- Repo-local fixture inputs and golden outputs live under:
  - `tests/tools/bench_convert/fixtures`
- The test runner does not depend on `C:\DEV\LXS` at runtime.
- Generated test outputs are written only under:
  - `tests/tools/bench_convert/fixtures/actual`
