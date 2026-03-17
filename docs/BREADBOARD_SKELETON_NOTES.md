# Breadboard Skeleton Implementation Notes

This document provides a brief summary of the initial `Breadboard` public boundary and skeleton implementation.

## Purpose

The `Breadboard` boundary has been established as a structural compilation layer without absorbing real execution, UI, or runtime API capabilities. The types and stubs provided here fulfill the requirements laid out in `STRATA_BREADBOARD_BOUNDARY_SPEC.md` while remaining intentionally incomplete in terms of actual compilation logic.

## Added Types

The following core public types define the compiler contract:
- `BreadboardTarget`: Specifies backend-targeted compilation intent (e.g., fast 4-state, temporal).
- `BreadboardResult`: Provides standard compilation status and error codes.
- `BreadboardCompileOptions`: Knobs for compilation strictness (currently provides `allow_placeholders`).
- `BreadboardDiagnostic`: Carries message and severity for structural and semantic checks.
- `BreadboardModule` (Opaque handle): Ingests structure and tracks compiler state.
- `BreadboardArtifactDraft` (Opaque handle): The executable handoff artifact intended for Forge load validation.

## Current Stub Limitations

As this is purely a scaffolding and contract task:
1. **Compilation**: `breadboard_module_compile` does not run any real lowering, recognition, or validation passes.
2. **Placeholders**: The compiler explicitly rejects compilation with `BREADBOARD_ERR_UNSUPPORTED` unless `allow_placeholders=true` is set via compile options. If set, an empty artifact draft is returned to explicitly plumb the happy path.
3. **Diagnostics**: `breadboard_module_get_diagnostic_count` aggressively returns 0, and `breadboard_module_get_diagnostic` returns `BREADBOARD_ERR_UNSUPPORTED` for any requested index as no structural faults are currently emitted.
4. **No Structural Import**: There are not yet APIs for actually feeding components, graphs, or netlists into the `BreadboardModule`.

These limitations ensure no real processing occurs and prevents arbitrary runtime behavior from appearing prematurely within Breadboard before the actual compiler paths are built.

## Boundary Adherence

- **No Forge Touching**: No `Forge` APIs or internal data structures were pulled into `Breadboard`. 
- **No Backend Logic**: No loops, session states, `LXS` fast states, or `HighZ` phase logics are maintained or imported into execution kernels.
- **Pure Opaque Separation**: The artifact and module state map solely to opaque pointers outside the boundary.
