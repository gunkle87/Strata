# Strata Backend Capability Registry

Purpose
- Define the machine-readable capability model for Strata backends.
- Establish how `Forge` discovers and reports backend strengths, limits, and
  extension surfaces.
- Give `Breadboard` a stable target-description vocabulary for legality,
  projection, and lowering decisions.

Status
- Planning and contract document.
- This file defines capability categories and reporting rules.
- It does not define the final ABI struct layout.

Core doctrine
- `shared common API for universals, extension APIs for strengths`
- `normalize for interoperability, preserve native truth for execution`

------------------------------------------------------------------------

## 1. Why This Registry Exists

Strata is not one engine pretending to be two.

It is:
- one layered system,
- one shared runtime boundary,
- multiple distinct backends.

That only works cleanly if backend differences are:
- explicit,
- queryable,
- stable enough to plan against,
- visible to both runtime and compiler layers.

Without a capability registry, the system drifts into:
- hardcoded backend assumptions,
- hidden special cases,
- accidental API distortion toward one backend,
- compiler decisions based on undocumented folklore.

This registry exists to stop that.

------------------------------------------------------------------------

## 2. Scope

This registry governs:
- backend identity reporting,
- shared common capability reporting,
- backend-native capability reporting,
- extension-family discovery,
- capability-informed `Breadboard` targeting,
- capability-informed `Forge` behavior selection.

This registry does not govern:
- authored structure,
- UI interaction semantics,
- final shared artifact binary layout,
- benchmark policy,
- backend implementation internals beyond what must be surfaced as stable
  capabilities.

------------------------------------------------------------------------

## 3. Core Rules

1. Every backend exposed through `Forge` must report capabilities explicitly.
2. Capability reporting must be machine-readable.
3. Capabilities must describe actual supported behavior, not aspiration.
4. Unsupported features must be reported as unsupported, not implied.
5. Common API support and extension support must be reported separately.
6. Capability reporting must not blur `LXS` and `HighZ` into one semantic
   runtime.
7. `Breadboard` may rely on capabilities for legality and lowering only if they
   are stable and documented here.

------------------------------------------------------------------------

## 4. Registry Roles By Layer

### 4.1 Forge

`Forge` uses the registry to:
- enumerate backends,
- report backend identity,
- report common capability support,
- report extension-family availability,
- reject unsupported operations cleanly.

### 4.2 Breadboard

`Breadboard` uses the registry to:
- select target lowering rules,
- apply projection policy,
- reject unsupported constructs for a chosen target,
- emit target-compatible executable artifacts.

### 4.3 Backends

Backends use the registry to:
- declare what they actually support,
- expose native strengths without leaking internal structs,
- publish extension families explicitly.

------------------------------------------------------------------------

## 5. Capability Families

Capabilities should be grouped by family, not scattered as unrelated flags.

Recommended capability families:
- backend identity
- state model
- lifecycle
- advancement
- observation
- probe support
- storage inspection
- clock and temporal control
- extension families
- compiler-targeting constraints
- diagnostics
- performance-oriented runtime features

------------------------------------------------------------------------

## 6. Backend Identity Capabilities

Every backend must report:
- backend name
- backend class
- backend version
- backend lineage or family label if useful
- native execution summary

Examples
- `LXS`
  - fast backend
  - 4-state
  - zero-delay
- `HighZ`
  - temporal backend
  - 7-state
  - 3-phase delta

Important rule
- identity is descriptive, not decorative.
- it must communicate enough for tools and logs to be useful.

------------------------------------------------------------------------

## 7. State Model Capabilities

Each backend must report:
- native state model class
- portable common read support level
- native richer-state read availability
- initialization-state handling support if exposed
- projection sensitivity relevant to compiler targeting

Examples

### LXS
- native state model: 4-state
- portable common reads: full
- native richer-state reads: none
- initialization-state steady-state support: no

### HighZ
- native state model: 7-state
- portable common reads: projected
- native richer-state reads: supported through extension
- initialization-state handling: backend-defined, capability-gated

------------------------------------------------------------------------

## 8. Lifecycle Capabilities

Each backend must report support for the common runtime lifecycle:
- artifact load support
- session create support
- session reset support
- session destroy support
- lifecycle-state query support

These are baseline capabilities.

If a backend cannot support the common lifecycle, it should not be admitted
through `Forge`.

------------------------------------------------------------------------

## 9. Advancement Capabilities

Each backend must report how it maps to the common advancement model.

Capability examples:
- supports common single-step advance
- supports common multi-step advance
- supports bounded run-until conditions
- supports bulk fast advance
- supports temporal sub-step control
- supports delta-phase stepping
- supports propagation-step stepping

Important rule
- common advancement support must be separated from backend-native temporal
  controls.

Examples

### LXS
- common single-step advance: yes
- common multi-step advance: yes
- bulk fast advance: yes
- temporal sub-step control: no
- delta-phase stepping: no

### HighZ
- common single-step advance: yes
- common multi-step advance: yes
- bulk fast advance: limited or backend-defined
- temporal sub-step control: yes
- delta-phase stepping: yes

------------------------------------------------------------------------

## 10. Observation Capabilities

Each backend must report support for:
- output reads
- portable signal reads
- name-to-ID mapping
- ID-to-descriptor mapping
- stable output descriptor enumeration
- native-state extension reads

Observation should be described in two layers:

### Common Observation
- portable normalized state reads
- output reads
- common descriptor reads

### Native Observation
- richer-state reads
- backend-specific object or lane inspection
- temporal intermediate inspection if intentionally supported

------------------------------------------------------------------------

## 11. Probe Capabilities

Each backend must report:
- common probe enumeration support
- common probe value support
- probe descriptor support
- probe snapshot grouping support if available
- native probe extension support
- runtime counter availability through the common path or extension path

Probe support must stay consistent with:
- [PROBE_PROTOCOL.md](PROBE_PROTOCOL.md)

Examples

### LXS
- common probe enumeration: yes
- common probe values: yes
- native richer probe families: limited or fast-backend specific

### HighZ
- common probe enumeration: yes
- common probe values: yes
- native temporal probe families: likely yes

------------------------------------------------------------------------

## 12. Storage Inspection Capabilities

Each backend must report support for reading:
- registers
- register files
- RAM
- ROM
- other stateful storage classes if admitted later

And for each storage class:
- common read support
- native detailed read support if it differs
- descriptor availability

Important rule
- if `Forge` common reads are expected to support a storage class, the backend
  must report whether it does so fully, partially, or not at all.

------------------------------------------------------------------------

## 13. Clock And Temporal Capabilities

These are especially important for `HighZ`.

Each backend should report:
- single-clock assumption or multi-clock capability
- temporal stepping support
- delta-phase visibility support
- event or wheel visibility support if admitted
- breakpoint or stop-boundary support if admitted

Examples

### LXS
- multi-clock temporal stepping: not a defining feature
- delta visibility: no
- event or wheel visibility: no

### HighZ
- multi-clock capability: expected or targetable
- delta visibility: yes
- temporal stepping: yes
- event or wheel visibility: capability-gated

------------------------------------------------------------------------

## 14. Compiler-Targeting Capabilities

`Breadboard` needs target-facing capability data beyond runtime lifecycle.

Backends should report compiler-relevant constraints such as:
- supported primitive families
- supported macro families
- native state model limits
- projection requirements for unsupported state distinctions
- storage-model constraints relevant to lowering
- temporal feature expectations relevant to compiled output

Important rule
- this is not a dump of backend internals.
- it is the minimum stable contract needed for legal lowering decisions.

------------------------------------------------------------------------

## 15. Diagnostics Capabilities

Each backend should report support for:
- last error message path
- structured diagnostic support if admitted later
- runtime warning reporting
- profiling-family extensions
- attribution-family extensions where relevant

Important rule
- diagnostics are capabilities too.
- if a backend exposes advanced profiling or temporal inspection, that must be
  discoverable instead of assumed.

------------------------------------------------------------------------

## 16. Performance-Oriented Runtime Capabilities

These are mostly relevant to `LXS`, but the registry should make room for them
without distorting the common API.

Examples:
- bulk advancement modes
- throughput-oriented counters
- packed execution diagnostics
- region execution diagnostics
- low-overhead profiling families

Important rule
- these belong in explicit extension families, not in the universal baseline.

------------------------------------------------------------------------

## 17. Extension Family Registry

Every backend must report extension families by stable name or stable enum-like
identity.

Recommended extension-family categories:
- `temporal_control`
- `native_state_read`
- `performance_profile`
- `runtime_diagnostics`
- `storage_detail`

Examples

### LXS likely extension families
- `performance_profile`
- `runtime_diagnostics`

### HighZ likely extension families
- `temporal_control`
- `native_state_read`
- `runtime_diagnostics`

Hard rule
- unsupported extension families must return explicit unsupported responses.

------------------------------------------------------------------------

## 18. Capability Value Shapes

Capabilities should not all be simple booleans.

Useful value shapes include:
- boolean
- enum-like class
- support level
- bounded integer count
- named extension family list
- capability version

Recommended support levels:
- `none`
- `partial`
- `full`

This is especially useful for:
- storage inspection
- probe support
- common read support
- run-control support

------------------------------------------------------------------------

## 19. Minimum Capability Set For Backend Admission

A backend should not be admitted through `Forge` unless it can report, at
minimum:
- backend identity
- native state model class
- common lifecycle support
- common advancement support
- common output read support
- capability query support itself
- extension-family list, even if empty

Recommended additional minimums:
- probe support level
- storage inspection support level
- compiler-targeting constraint summary

------------------------------------------------------------------------

## 20. Known Initial Backend Profiles

These are planning-level profiles, not final runtime declarations.

### 20.1 LXS Planning Profile

Backend class
- fast backend

Expected capability profile
- native state model: 4-state
- common lifecycle: full
- common advancement: full
- bulk fast advancement: full
- temporal sub-step control: none
- native richer-state reads: none
- common observation: full
- probe support: full or near-full
- storage inspection: full or near-full
- likely extension families:
  - `performance_profile`
  - `runtime_diagnostics`

### 20.2 HighZ Planning Profile

Backend class
- temporal backend

Expected capability profile
- native state model: 7-state
- common lifecycle: full
- common advancement: full
- temporal sub-step control: full
- delta-phase stepping: full
- native richer-state reads: full or near-full
- common observation: projected
- probe support: full
- storage inspection: full or near-full
- likely extension families:
  - `temporal_control`
  - `native_state_read`
  - `runtime_diagnostics`

------------------------------------------------------------------------

## 21. Relationship To Forge

`Forge` must use this registry to decide:
- what common operations to expose as available,
- what extension families can be requested,
- what errors to return for unsupported operations,
- what metadata to surface to tooling and UI.

Hard rule
- `Forge` must not hardcode assumptions that bypass the registry.

------------------------------------------------------------------------

## 22. Relationship To Breadboard

`Breadboard` must use this registry to decide:
- whether a target backend can legally accept a construct,
- whether projection is required,
- whether a macro family is legal for a target,
- what artifact metadata must be emitted for load-time validation.

Hard rule
- `Breadboard` must not lower against undocumented backend assumptions.

------------------------------------------------------------------------

## 23. Relationship To Shared Artifact Draft

The later artifact draft must carry enough capability-relevant information to:
- validate target backend compatibility,
- reconstruct descriptor mapping needed by `Forge`,
- preserve compiler-targeting choices visible at load time,
- surface projection or approximation metadata where required.

This registry therefore constrains the artifact draft, but does not define its
layout.

------------------------------------------------------------------------

## 24. Validation Requirements

Capability reporting is only useful if it is testable.

Validation should include:
- backend enumeration tests
- capability query tests
- unsupported capability rejection tests
- extension-family discovery tests
- `Breadboard` target-legality tests driven by capability data
- `Forge` behavior-selection tests driven by capability data

Important rule
- capability claims must be verified against real backend behavior.

------------------------------------------------------------------------

## 25. Risks To Avoid

1. Treating capability reporting as documentation only.
2. Using booleans where support levels are needed.
3. Hardcoding backend assumptions outside the registry.
4. Letting the registry become vague marketing instead of a technical contract.
5. Collapsing `LXS` and `HighZ` differences in the name of convenience.
6. Exposing backend-private implementation details instead of stable
   capabilities.

------------------------------------------------------------------------

## 26. Relationship To Other Design Docs

This document follows:
- [STRATA_FORGE_RUNTIME_API_PLAN.md](STRATA_FORGE_RUNTIME_API_PLAN.md)
- [STRATA_STATE_PROJECTION_POLICY.md](STRATA_STATE_PROJECTION_POLICY.md)
- [STRATA_BREADBOARD_BOUNDARY_SPEC.md](STRATA_BREADBOARD_BOUNDARY_SPEC.md)

This document should guide:
- `STRATA_SHARED_PLAN_DRAFT.md`

------------------------------------------------------------------------

## 27. High-Level Conclusion

The backend capability registry is the translation table between:
- what backends truly are,
- what `Forge` can safely expose,
- and what `Breadboard` can legally target.

If this stays explicit and machine-readable:
- backend purity stays intact,
- compiler decisions stay honest,
- the common API stays stable,
- and the later artifact draft can be built on real constraints instead of
  guesses.

