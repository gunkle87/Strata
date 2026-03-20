# Strata Forge Runtime API Plan

Purpose
- Define the first Strata-native runtime API contract.
- Establish the stable shared boundary that sits directly above backend
  execution.
- Lock the common runtime lifecycle before any shared executable plan format is
  finalized.

Status
- Planning document.
- This file defines API intent and boundary ownership.
- It does not finalize the shared plan contract.
- Current phase-close note:
  - `Forge` now has one real admitted runtime slice for the executable
    `FAST_4STATE` path.
  - That real slice includes load validation, session creation/reset/destroy,
    input application, one deterministic common advancement boundary, and
    output reads for admitted real fast artifacts.
  - It does not imply temporal readiness, broad probe readiness, storage
    inspection readiness, or full multi-backend parity.

Core doctrine
- `structure is the authoring layer`
- `function is the execution layer`

This means:
- `Forge` is not the authoring API,
- `Forge` is not the structural compiler,
- `Forge` is the runtime-facing boundary over execution.

------------------------------------------------------------------------

## 1. Why Forge Exists

Strata is intentionally layered:
- `authoring`
- `Breadboard`
- `Forge`
- backend execution

Without `Forge`, the system would drift into the same failure mode seen in
older engine iterations:
- tools reaching into backend internals,
- backend details leaking into upper layers,
- direct coupling between UI and runtime memory layout,
- brittle bindings tied to structs that should remain private.

`Forge` exists to stop that.

Its job is to provide:
- one stable runtime lifecycle,
- one stable common inspection surface,
- one stable capability-discovery surface,
- one explicit extension path for backend-specific strengths.

------------------------------------------------------------------------

## 2. Scope

`Forge` owns:
- runtime library initialization if needed,
- backend discovery,
- capability discovery,
- runtime session creation and destruction,
- loading backend-targeted executable artifacts,
- reset and lifecycle control,
- input application,
- common simulation advancement,
- common output and state reads,
- common probes,
- metadata queries,
- backend extension discovery and dispatch.

`Forge` does not own:
- authored circuit structure,
- editing semantics,
- recognition policy,
- macro admission policy,
- backend-native scheduling internals,
- benchmark harness behavior,
- shared plan format decisions beyond what the runtime boundary requires.

------------------------------------------------------------------------

## 3. Non-Goals

This phase does not include:
- authoring API design,
- `Breadboard` structural compiler API design,
- final shared plan format design,
- UI workflow design,
- remote service protocol,
- plugin ABI,
- speculative distributed execution,
- forcing `LXS` and `HighZ` into one semantic runtime model.

Important rule
- this document must not silently decide the universal plan contract too early.

------------------------------------------------------------------------

## 4. Core Principles

1. `Forge` is the only runtime interface.
   - No UI, tool, or test should call backend internals directly.

2. Shared first, extension second.
   - Put only coherent cross-backend behavior in the common API.
   - Expose backend-specific strengths through explicit extension families.

3. Opaque runtime ownership.
   - Public consumers must not depend on backend struct layout.

4. Preserve native backend truth.
   - `LXS` stays 4-state and zero-delay.
   - `HighZ` stays 7-state and 3-phase delta.

5. Normalize only for interoperability.
   - Common reads expose portable state where needed.
   - Native richer behavior is exposed through capability-gated extensions.

6. Coarse-grained access over chatty calls.
   - The API must support Python and future tooling without per-bit FFI loops
     becoming the intended usage pattern.

7. Capability discovery is first-class.
   - Clients should be able to ask what a backend supports before using it.

------------------------------------------------------------------------

## 5. Runtime Boundary Position

Bottom-up stack:

1. backend execution
2. `Forge`
3. `Breadboard`
4. authoring
5. tooling

This is intentional.

`Forge` sits directly above execution and below `Breadboard`.

That means:
- `Forge` consumes executable artifacts suitable for a backend runtime,
- `Forge` does not consume raw authored structure,
- `Breadboard` is responsible for translation into runtime-loadable form.

------------------------------------------------------------------------

## 6. Shared Runtime Responsibilities

The common `Forge` API must define these families.

### 6.1 Library and Backend Discovery

Functions
- enumerate registered backends,
- query backend identity,
- query backend version,
- query capability sets,
- query extension families.

Purpose
- let upper layers decide what runtime path is available,
- let tools adapt without hardcoding backend assumptions.

### 6.2 Executable Artifact Loading

Functions
- accept a backend-targeted executable artifact,
- validate backend compatibility,
- create a loadable runtime object,
- reject mismatched artifacts cleanly.

Important constraint
- this defines lifecycle expectations, not the final shared plan format.

### 6.3 Session Lifecycle

Functions
- create runtime session,
- reset runtime session,
- free runtime session,
- query lifecycle state,
- query loaded backend and artifact metadata.

### 6.4 Input Application

Functions
- write inputs by stable index,
- write inputs by stable object identifier,
- submit batched input updates,
- commit input application at well-defined runtime boundaries.

### 6.5 Common Advancement

Functions
- advance one common simulation step,
- advance multiple common steps,
- run until a bounded stop condition where supported by all backends.

Important rule
- the common advancement contract must stay generic enough to map cleanly onto
  both:
  - `LXS` tick-style execution
  - `HighZ` temporal execution

This common layer should not pretend both backends expose identical temporal
controls.

### 6.6 Common Reads

Functions
- read outputs,
- read named signal values through stable IDs,
- read common stateful object values where portable,
- read portable normalized state representation,
- read metadata needed to interpret returned buffers.

### 6.7 Common Probes

Functions
- enumerate probes,
- read probe descriptors,
- read probe values or snapshots,
- read basic runtime counters that are considered part of portable inspection.

### 6.8 Metadata and Mapping

Functions
- enumerate inputs,
- enumerate outputs,
- enumerate named runtime-visible objects,
- map names to stable IDs,
- map stable IDs back to descriptors,
- inspect width, class, and ownership metadata.

------------------------------------------------------------------------

## 7. Shared API Object Families

Recommended public object families:

- `ForgeLibrary`
  - optional library or registry context
- `ForgeBackendInfo`
  - backend identity and capability metadata
- `ForgeArtifact`
  - loaded backend-targeted executable artifact handle
- `ForgeSession`
  - live runtime session handle
- `ForgeDescriptor`
  - metadata object for signals, probes, storage, and runtime-visible entities

Public rule
- these should be opaque at the ABI boundary where possible.

Internal rule
- backend-private structs stay backend-private.

------------------------------------------------------------------------

## 8. Common Lifecycle Contract

The first shared lifecycle should look like this conceptually:

1. discover backend
2. inspect backend capabilities
3. load backend-targeted executable artifact
4. create session
5. apply inputs
6. advance runtime through the common step contract
7. read outputs and probes
8. reset or destroy session

What must be true:
- lifecycle ownership is explicit,
- invalid order is an API error,
- reset semantics are documented,
- backend mismatch is visible immediately,
- caller-owned buffers are preferred for bulk reads.

------------------------------------------------------------------------

## 9. Common State Contract

The common API must expose a portable state view suitable for cross-backend
interoperability.

Recommended normalized common view:
- `0`
- `1`
- `X`
- `Z`

Important rule
- this is an observation contract, not a command to collapse backend-native
  execution truth.

Implications
- `LXS` can report native 4-state directly,
- `HighZ` may report projected portable state through the common API,
- `HighZ` may also expose richer native state through extension APIs.

Common reads should distinguish:
- current committed observable state,
- outputs after the last common advance boundary,
- stable probe snapshots,
- runtime object descriptors required to interpret values.

The common API must not leak:
- half-committed backend-private arrays,
- unstable transient scratch storage,
- implementation-only scheduling buffers.

------------------------------------------------------------------------

## 10. Probe Contract

Probes matter enough in Strata that they must be part of the shared API from the
beginning.

The common probe contract should support:
- probe enumeration,
- stable probe identifiers,
- probe descriptor reads,
- probe value reads,
- probe snapshot grouping where meaningful,
- portable counter reads relevant to runtime inspection.

Probe design must remain consistent with:
- [PROBE_PROTOCOL.md](/docs/reference/PROBE_PROTOCOL.md)

Important rule
- probe support belongs in the runtime boundary, not in benchmark-only tools.

------------------------------------------------------------------------

## 11. Capability Discovery Contract

Capability discovery is mandatory.

Every backend exposed through `Forge` should report:
- backend name,
- backend class,
- native state model,
- common lifecycle support level,
- common read support level,
- probe support level,
- available extension families,
- known execution semantics summary,
- known limits that upper layers must respect.

Examples of capability questions:
- does this backend support temporal sub-step control?
- does this backend support richer-than-common state reads?
- does this backend support bulk advancement?
- does this backend support storage inspection?
- does this backend support multiple clock domains?

Important rule
- capabilities must be explicit machine-readable data, not only documentation.

------------------------------------------------------------------------

## 12. Extension Model

`Forge` should define a clean shared-extension pattern.

Shared API responsibilities:
- discover extension families,
- query whether an extension family is supported,
- expose stable extension entry lookup,
- ensure unsupported calls fail explicitly.

Backend-specific extension families should remain separate.

### 12.1 LXS-Oriented Extensions

Examples
- bulk advancement modes,
- fast profiling controls,
- packed or region execution diagnostics,
- throughput-oriented runtime counters.

### 12.2 HighZ-Oriented Extensions

Examples
- delta-phase stepping,
- propagation-step control,
- temporal breakpoint boundaries,
- richer native state inspection,
- event or wheel diagnostics where applicable.

Rule
- extension features must not be smuggled into the common API just because one
  backend does them well.

------------------------------------------------------------------------

## 13. Error and Diagnostics Model

The API must use explicit result codes and predictable failure behavior.

Recommended error classes:
- invalid argument,
- invalid handle,
- invalid lifecycle order,
- backend unavailable,
- artifact incompatible,
- unsupported capability,
- out of bounds,
- internal failure.

Diagnostics path should support:
- last error code,
- last error message,
- optional structured diagnostic payload later if needed.

Important rule
- failure must be ordinary and inspectable, not silent.

------------------------------------------------------------------------

## 14. Data Movement Rules

The common API should prefer:
- caller-owned buffers,
- bulk fills,
- descriptor-first access,
- stable IDs over repeated string lookups in hot paths.

Avoid as the intended path:
- per-bit queries,
- chatty single-signal calls for large reads,
- runtime name resolution in inner loops.

This matters for:
- Python bindings,
- test harness performance,
- UI state refresh cost,
- probe polling.

------------------------------------------------------------------------

## 15. Relationship To Breadboard

`Breadboard` and `Forge` must remain separate.

`Breadboard` owns:
- structure import,
- normalization,
- recognition,
- macro substitution,
- backend-targeted lowering,
- production of backend-loadable executable artifacts.

`Forge` owns:
- loading those artifacts,
- validating runtime compatibility,
- creating runtime sessions,
- exposing execution control and inspection.

Important rule
- `Forge` should not silently absorb compiler responsibilities.

------------------------------------------------------------------------

## 16. Relationship To Shared Plan Design

This document intentionally stops short of finalizing a universal plan format.

What this document may decide:
- what lifecycle states exist,
- what runtime objects need stable identity,
- what reads and probes must be possible,
- what capability discovery must report,
- what extension structure must exist.

What this document must not decide yet:
- final executable artifact binary layout,
- final shared plan schema,
- how much of that artifact is universal versus backend-targeted,
- whether one artifact family or several artifact families are needed.

Those decisions must follow from:
- the `Forge` runtime boundary defined here,
- the `Breadboard` compiler boundary,
- backend capability and semantic constraints.

------------------------------------------------------------------------

## 17. First Admission Boundary

The first admitted `Forge` slice should be intentionally small but real.

Recommended initial support:
- backend enumeration,
- capability query,
- load backend-targeted artifact handle,
- create session,
- reset session,
- apply input batch,
- advance one common step,
- read outputs in normalized portable form,
- read a minimal probe set,
- destroy session.

This is enough to validate:
- lifecycle shape,
- backend dispatch boundary,
- opaque-handle discipline,
- common read semantics,
- first Python-friendly binding path later.

------------------------------------------------------------------------

## 18. Validation Requirements

Each admitted `Forge` step should have:
- direct C-level tests through the public boundary,
- lifecycle ordering tests,
- invalid-handle tests,
- capability-discovery tests,
- backend mismatch rejection tests,
- bulk read shape tests,
- probe access smoke tests.

Critical validation rule
- tests must use `Forge`, not backend internals.

------------------------------------------------------------------------

## 19. Risks To Avoid

1. Deciding the shared plan format too early.
2. Letting `Forge` become a disguised compiler layer.
3. Letting one backend's strengths distort the common API.
4. Exposing backend-private runtime memory as public ABI.
5. Making Python bindings slow by design through chatty access patterns.
6. Hiding backend differences instead of reporting them through capabilities.
7. Treating common normalized state as if it were the only runtime truth.

------------------------------------------------------------------------

## 20. Recommended Next Documents

After this plan, the next design documents should be:

1. `docs/reference/STRATA_STATE_PROJECTION_POLICY.md`
   - define portable-state observation and backend-targeted projection rules

2. `docs/boundaries/STRATA_BREADBOARD_BOUNDARY_SPEC.md`
   - define what `Breadboard` produces and what `Forge` consumes

3. `docs/reference/STRATA_BACKEND_CAPABILITY_REGISTRY.md`
   - define the machine-readable capability surface

4. `docs/boundaries/STRATA_SHARED_PLAN_DRAFT.md`
   - derive executable artifact contract from the runtime and compiler
     boundaries already defined

------------------------------------------------------------------------

## 21. High-Level Conclusion

`Forge` is the shared runtime boundary for Strata.

Its first job is not to unify all backend internals.
Its first job is to make backend differences safe, explicit, and usable through
one disciplined runtime-facing contract.

If this layer stays strict:
- `Breadboard` can evolve above it,
- `LXS` can stay fast below it,
- `HighZ` can stay temporal below it,
- tooling and UI can integrate without reaching into backend memory layouts,
- and the shared plan contract can be designed later from a stable foundation
  instead of guessed too early.

Current phase-close interpretation:
- `Forge` is now real for one narrow executable lifecycle.
- The current real common runtime surface is limited to admitted fast-path
  artifact load, session lifecycle, input application, deterministic stepping,
  and output reads.
- Placeholder load paths, broader probes, storage inspection, and temporal
  controls remain outside this completed phase.


