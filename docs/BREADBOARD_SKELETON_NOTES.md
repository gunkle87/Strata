# Breadboard Skeleton Implementation Notes

This document provides a brief summary of the initial `Breadboard` public boundary and skeleton implementation.

## Purpose

The `Breadboard` boundary has been established as a structural compilation layer without absorbing real execution, UI, or runtime API capabilities. The types and stubs provided here fulfill the requirements laid out in `STRATA_BREADBOARD_BOUNDARY_SPEC.md` while remaining intentionally incomplete in terms of actual compilation logic.

## Added Types

The following core public types define the compiler contract:
- `BreadboardTarget`: Specifies backend-targeted compilation intent (e.g., fast 4-state, temporal).
- `BreadboardTargetMask`: Bitmask representing allowed execution targets following product profile models.
- `BreadboardDescriptorClass`: Enumerates the runtime-visible structural classifications (input, output, probe).
- `BreadboardDescriptor`: Defines the layout of a deterministic identifier describing an exported object's shape, purpose, and stability.
- `BreadboardResult`: Provides standard compilation status and error codes.
- `BreadboardCompileOptions`: Knobs for compilation strictness (provides `allow_placeholders`, `deny_approximation`, `strict_projection`).
- `BreadboardDiagnosticSeverity`: Severity scale for emitted compiler diagnostics.
- `BreadboardDiagnosticCode`: Identifiers for specific compiler structural and semantic checks.
- `BreadboardDiagnostic`: Carries message, code, and severity for diagnostics.
- `BreadboardTargetInfo`: Metadata defining the target backend and capabilities.
- `BreadboardDraftInfo`: Coarse stable summary of a generated draft, preserving the earlier draft-info contract.
- `BreadboardDraftAdmissionInfo`: Metadata surface describing a constructed compilation artifact draft, oriented towards Forge runtime admission considerations and explicit placeholder extension/capability behaviors.
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

The draft also supports:
- `breadboard_artifact_draft_query_info`
- `breadboard_draft_query_admission_info`
- `breadboard_artifact_draft_export_placeholder_size`
- `breadboard_artifact_draft_export_placeholder`

The first is a coarse stable draft summary. The second is the newer admission-oriented surface.
The export helpers provide a temporary placeholder handoff into the current
Forge stub artifact loader for vertical integration tests. They are explicitly
not the final Strata artifact format.

## Draft Admission Metadata
Drafts generated using the placeholder allowance (`allow_placeholders=true`) provide explicit admission-oriented metadata via `breadboard_draft_query_admission_info`. The exposed metadata represents coarse placeholder capabilities:
- **`requires_advanced_controls`**: Set to true when the target implies structural capabilities beyond baseline native limits (e.g. `TEMPORAL` placeholders).
- **`native_only_behavior`**: Coarse placeholder signal that the draft assumes target-native behavior beyond the common baseline.
- **`extension_flags`**: Placeholder bit flags acting as a temporary proxy for future extension-family requirements. These are not yet real backend registry masks.

This allows the Breadboard scaffolding to emit drafts which explicitly answer early admission-style questions without pretending to encode real backend binary structures.

## Temporary Placeholder Export

Breadboard can now export a placeholder draft into a temporary Forge-compatible
stub artifact byte stream. This export path:
- requires a placeholder draft
- derives the placeholder backend ID from the draft target using the current
  temporary handoff contract
- maps FAST_4STATE placeholder drafts to the baseline stub payload
- maps TEMPORAL placeholder drafts to the advanced-controls stub payload

This export path exists only to validate the first Breadboard -> Forge vertical
handoff. It should be treated as scaffolding rather than the long-term artifact
contract.

The current placeholder handoff now carries an explicit coarse admission
manifest, a serialized descriptor block, and a tiny section directory in the
shared placeholder header in addition to the payload class. That keeps the
temporary contract self-describing while still remaining a scaffolding format
rather than the final Strata artifact layout.

## Current Stub Limitations

As this is purely a scaffolding and contract task:
1. **Compilation**: `breadboard_module_compile` does not run any real lowering, recognition, or validation passes.
2. **Policy Gating**: target gating mechanisms via `breadboard_module_set_target_policy` and `breadboard_module_query_target_availability` exist. Compilation is explicitly rejected with `BREADBOARD_DIAG_CODE_TARGET_DENIED_BY_POLICY` if attempting to target denied backends. Note that Forge still remains the final runtime enforcement boundary.
3. **Draft Metadata**: Both `BreadboardDraftInfo` and `BreadboardDraftAdmissionInfo` are placeholder-oriented summaries. They are intentionally coarse and should not be interpreted as real lowering facts.
4. **Placeholders**: The compiler explicitly rejects compilation with `BREADBOARD_ERR_COMPILE_FAILED` recording a diagnostic unless `allow_placeholders=true` is set. If allowed, a draft is returned that deterministically exposes 2 dummy inputs, 2 dummy outputs, and 1 dummy probe descriptor, serving to establish standard runtime discovery.
5. **Diagnostics**: `breadboard_module_get_diagnostic_count` and `breadboard_module_get_diagnostic` now reflect the internally recorded array of diagnostics resulting from module compilation and API usage.
6. **Temporary Artifact Handoff**: The export helpers produce only the current temporary Forge-compatible placeholder bytes. They do not define the final Strata artifact format.
7. **No Structural Import**: There are not yet APIs for actually feeding components, graphs, or netlists into the `BreadboardModule`.

These limitations ensure no real processing occurs and prevents arbitrary runtime behavior from appearing prematurely within Breadboard before the actual compiler paths are built.

## Boundary Adherence

- **No Forge Touching**: No `Forge` APIs or internal data structures were pulled into `Breadboard`. 
- **No Backend Logic**: No loops, session states, `LXS` fast states, or `HighZ` phase logics are maintained or imported into execution kernels.
- **Pure Opaque Separation**: The artifact and module state map solely to opaque pointers outside the boundary.
