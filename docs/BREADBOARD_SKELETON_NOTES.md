# Breadboard Skeleton Implementation Notes

This document provides a brief summary of the initial `Breadboard` public boundary and skeleton implementation.

## Purpose

The `Breadboard` boundary has been established as a structural compilation layer without absorbing real execution, UI, or runtime API capabilities. The types and stubs provided here fulfill the requirements laid out in `STRATA_BREADBOARD_BOUNDARY_SPEC.md` while remaining intentionally incomplete in terms of actual compilation logic.

## Added Types

The following core public types define the compiler contract:
- `BreadboardTarget`: Specifies backend-targeted compilation intent (e.g., fast 4-state, temporal).
- `BreadboardDescriptorClass`: Enumerates the runtime-visible structural classifications (input, output, probe).
- `BreadboardDescriptor`: Defines the layout of a deterministic identifier describing an exported object's shape, purpose, and stability.
- `BreadboardResult`: Provides standard compilation status and error codes.
- `BreadboardCompileOptions`: Knobs for compilation strictness (provides `allow_placeholders`, `deny_approximation`, `strict_projection`).
- `BreadboardDiagnosticSeverity`: Severity scale for emitted compiler diagnostics.
- `BreadboardDiagnosticCode`: Identifiers for specific compiler structural and semantic checks.
- `BreadboardDiagnostic`: Carries message, code, and severity for diagnostics.
- `BreadboardTargetInfo`: Metadata defining the target backend and capabilities.
- `BreadboardDraftInfo`: Metadata describing an artifact draft containing its target, size structure, and generation properties.
- `BreadboardModule` (Opaque handle): Ingests structure and tracks compiler state, stores recorded diagnostics.
- `BreadboardArtifactDraft` (Opaque handle): The executable handoff artifact intended for Forge load validation.

## Query API
The `BreadboardArtifactDraft` now supports a set of query functionalities to expose structured descriptors:
- `breadboard_draft_input_descriptor_count`, `breadboard_draft_input_descriptor_at`
- `breadboard_draft_input_descriptor_by_id`, `breadboard_draft_input_descriptor_by_name`
- `breadboard_draft_output_descriptor_count`, `breadboard_draft_output_descriptor_at`
- `breadboard_draft_output_descriptor_by_id`, `breadboard_draft_output_descriptor_by_name`
- `breadboard_draft_probe_descriptor_count`, `breadboard_draft_probe_descriptor_at`
- `breadboard_draft_probe_descriptor_by_id`, `breadboard_draft_probe_descriptor_by_name`

These queries are deterministic and operate entirely draft-side, keeping lookup mapping explicit without relying on runtime session mechanisms. Lookup failures explicitly yield `BREADBOARD_ERR_NOT_FOUND`.

## Current Stub Limitations

As this is purely a scaffolding and contract task:
1. **Compilation**: `breadboard_module_compile` does not run any real lowering, recognition, or validation passes.
2. **Placeholders**: The compiler explicitly rejects compilation with `BREADBOARD_ERR_COMPILE_FAILED` recording a diagnostic unless `allow_placeholders=true` is set. If allowed, an artifact is returned that deterministically exposes 2 dummy inputs, 2 dummy outputs, and 1 dummy probe descriptor, serving to establish standard runtime discovery.
3. **Diagnostics**: `breadboard_module_get_diagnostic_count` and `breadboard_module_get_diagnostic` now reflect the internally recorded array of diagnostics resulting from module compilation and API usage.
4. **No Structural Import**: There are not yet APIs for actually feeding components, graphs, or netlists into the `BreadboardModule`.

These limitations ensure no real processing occurs and prevents arbitrary runtime behavior from appearing prematurely within Breadboard before the actual compiler paths are built.

## Boundary Adherence

- **No Forge Touching**: No `Forge` APIs or internal data structures were pulled into `Breadboard`. 
- **No Backend Logic**: No loops, session states, `LXS` fast states, or `HighZ` phase logics are maintained or imported into execution kernels.
- **Pure Opaque Separation**: The artifact and module state map solely to opaque pointers outside the boundary.
