# Strata Shared Artifact Draft

Purpose
- Define the first shared artifact contract for Strata.
- Specify what `Breadboard` must emit and what `Forge` must be able to load.
- Derive artifact obligations from the already-defined runtime boundary,
  projection policy, compiler boundary, and backend capability model.

Status
- Draft design document.
- This file defines required artifact meaning and minimum content.
- It does not finalize the exact binary encoding, serialization format, or
  in-memory representation.

Core doctrine
- `structure is the authoring layer`
- `function is the execution layer`
- `shared common API for universals, extension APIs for strengths`

------------------------------------------------------------------------

## 1. Why This Draft Exists

Strata now has:
- a shared runtime API boundary through `Forge`,
- a state projection policy,
- a `Breadboard` boundary,
- a backend capability registry.

The next step is to define what compiled output must look like at the contract
level so that:
- `Breadboard` can emit something runtime-loadable,
- `Forge` can validate and load it,
- backends can execute it without authoring-layer knowledge,
- the system can preserve backend purity without fragmenting the ecosystem.

This artifact is the handoff object between:
- structural compilation,
- runtime loading,
- backend execution.

------------------------------------------------------------------------

## 2. What This Draft Is And Is Not

This draft is:
- a meaning contract,
- a boundary contract,
- a minimum-content contract.

This draft is not:
- a final file format,
- a final ABI struct definition,
- a promise that one physical representation must serve all cases,
- a license to push backend execution details into the shared runtime API.

Important rule
- this draft follows from the earlier docs.
- it does not override them for convenience.

------------------------------------------------------------------------

## 3. Artifact Position In The Stack

Bottom-up:

1. backend execution
2. `Forge`
3. shared artifact
4. `Breadboard`
5. authoring
6. tooling

That means:
- authoring does not feed the runtime directly,
- `Forge` does not consume raw authored structure,
- `Breadboard` emits artifact form,
- the artifact is the runtime-facing product of structural compilation.

------------------------------------------------------------------------

## 4. Artifact Responsibilities

The shared artifact must carry enough information to support:
- backend compatibility validation,
- session creation,
- input application by stable runtime-visible identity,
- output reads by stable runtime-visible identity,
- common descriptor mapping for `Forge`,
- probe descriptor mapping where applicable,
- storage descriptor mapping where applicable,
- target-specific executable content,
- projection and approximation visibility where required.

If an artifact cannot support those responsibilities, it is not sufficient for
Strata runtime loading.

------------------------------------------------------------------------

## 5. Core Artifact Rules

1. Every artifact must be target-aware.
   - The target backend or backend class must be explicit.

2. Every artifact must be load-validated.
   - `Forge` must be able to reject incompatible artifacts before execution.

3. Every artifact must preserve stable runtime-visible identity.
   - Inputs, outputs, and other runtime-visible entities must be recoverable by
     stable IDs and descriptors.

4. Every artifact must be deterministic for deterministic compiler inputs.

5. Every artifact must carry enough metadata to explain meaningful projection or
   approximation applied during lowering.

6. The artifact must not require authoring-layer context at runtime.

7. The artifact contract must support more than one backend without forcing the
   backends into one execution model.

------------------------------------------------------------------------

## 6. Artifact Families

At the contract level, Strata should think in terms of one shared artifact
family with backend-targeted instances.

That means:
- one shared concept,
- one shared set of minimum required fields and guarantees,
- backend-specific executable payload sections where needed.

This is better than:
- one totally unrelated artifact per backend,
- or one fake universal artifact that erases backend truth.

So the right current mental model is:
- shared artifact envelope,
- backend-targeted executable payload.

------------------------------------------------------------------------

## 7. Minimum Artifact Sections

Every artifact should conceptually contain these sections.

### 7.1 Artifact Header

Required meaning:
- artifact format version,
- emitting compiler identity or version,
- target backend identity or backend class,
- compatibility class,
- integrity or validity marker.

Purpose:
- let `Forge` reject obviously incompatible or stale artifacts,
- support future format evolution without ambiguity.

### 7.2 Target Compatibility Section

Required meaning:
- target backend name or class,
- required capability class,
- state-model assumptions,
- runtime feature assumptions,
- any required extension-family dependencies.

Purpose:
- allow `Forge` to validate that the selected backend can legally load the
  artifact.

### 7.3 Runtime Descriptor Section

Required meaning:
- input descriptors,
- output descriptors,
- stable runtime-visible IDs,
- width information,
- class information,
- name mapping where names exist,
- ownership or grouping metadata relevant to common reads.

Purpose:
- give `Forge` enough information to expose common runtime reads without
  authoring context.

### 7.4 Probe Descriptor Section

Required meaning:
- probe IDs,
- probe classes,
- widths where needed,
- stable names or labels where present,
- grouping metadata where probe snapshots require it.

Purpose:
- allow common probe enumeration and reading through `Forge`.

### 7.5 Storage Descriptor Section

Required meaning:
- descriptors for registers,
- RAM,
- ROM,
- register files,
- other admitted stateful object classes later if needed.

Purpose:
- support common storage inspection where capabilities allow it.

### 7.6 Executable Payload Section

Required meaning:
- target-backend executable content,
- sufficient runtime data for the target backend to create a session and
  execute.

Important rule
- this section may differ sharply by backend.
- that is acceptable and expected.

### 7.7 Projection And Approximation Metadata

Required meaning:
- whether projection was applied,
- what projection families were applied,
- what approximation categories occurred,
- whether semantic collapse beyond exact preservation happened.

Purpose:
- make semantic loss visible,
- keep runtime and tooling honest.

------------------------------------------------------------------------

## 8. Stable Runtime-Visible Identity

The artifact must preserve enough identity to make `Forge` useful.

At minimum it must support:
- stable input IDs,
- stable output IDs,
- stable runtime-visible object IDs for any object the common API can read,
- descriptor reconstruction without authoring-layer lookup,
- deterministic name mapping for deterministic compiler runs.

Important rule
- stable IDs do not need to equal authoring-layer IDs internally,
- but they must be deterministic and interpretable through artifact metadata.

------------------------------------------------------------------------

## 9. Relationship To Common API Reads

The artifact must contain enough information for `Forge` to support:
- common output reads,
- common normalized state reads,
- descriptor-driven buffer sizing,
- name-to-ID lookup where names are part of the runtime contract,
- probe enumeration and probe reads,
- storage inspection where the backend reports support.

The artifact must not require:
- direct backend struct access from callers,
- authoring-layer graph reconstruction at runtime,
- hidden compiler side tables living outside the artifact.

------------------------------------------------------------------------

## 10. Relationship To State Projection Policy

This artifact draft must honor:
- [STRATA_STATE_PROJECTION_POLICY.md](/c:/DEV/Strata/docs/STRATA_STATE_PROJECTION_POLICY.md)

That means:
- backend-targeted projection happens before or during artifact emission,
- not as silent runtime reinterpretation,
- and meaningful approximation must be visible in artifact metadata.

Examples:
- if `UNINIT` is resolved before steady-state `LXS` execution, the artifact
  should reflect the post-resolution executable intent rather than carry
  `UNINIT` as if it were native `LXS` runtime truth.
- if `WEAK` distinctions are flattened for `LXS`, that fact should be
  discoverable through projection metadata.

------------------------------------------------------------------------

## 11. Relationship To Capability Registry

This artifact draft must honor:
- [STRATA_BACKEND_CAPABILITY_REGISTRY.md](/c:/DEV/Strata/docs/STRATA_BACKEND_CAPABILITY_REGISTRY.md)

That means artifact compatibility must be expressible in terms of:
- backend identity,
- backend class,
- required capabilities,
- required extension families if any,
- compiler-targeting assumptions relevant to loading.

Important rule
- artifacts must not rely on undocumented backend behavior.

------------------------------------------------------------------------

## 12. Relationship To Breadboard

`Breadboard` owns:
- structural import,
- normalization,
- validation,
- recognition,
- macro substitution,
- target legality checks,
- projection,
- artifact emission.

So `Breadboard` is responsible for ensuring the artifact:
- is deterministic,
- is target-valid,
- carries runtime-visible descriptor mapping,
- includes projection metadata where needed,
- contains executable content suitable for the selected backend.

------------------------------------------------------------------------

## 13. Relationship To Forge

`Forge` owns:
- backend discovery,
- capability discovery,
- artifact load validation,
- session creation,
- runtime reads,
- probe reads,
- capability-gated extension entry.

So `Forge` must be able to:
- inspect artifact header data,
- inspect target compatibility data,
- reconstruct runtime-visible descriptors,
- expose common reads from artifact-provided mapping,
- reject incompatible artifacts before session creation.

Hard rule
- `Forge` should not need compiler internals to understand artifact validity.

------------------------------------------------------------------------

## 14. Relationship To Backends

Backends should receive from `Forge` a validated, target-appropriate executable
artifact representation.

The artifact must be sufficient for:
- session initialization,
- native runtime allocation,
- native execution setup,
- runtime-visible object mapping needed for common reads.

The artifact should not require the backend to:
- re-run recognition,
- re-run structural normalization,
- reinterpret authoring semantics,
- invent missing descriptor mapping after load.

------------------------------------------------------------------------

## 15. Shared Envelope Versus Backend Payload

The clean current model is:

### Shared Envelope

Carries:
- artifact identity,
- target compatibility,
- runtime descriptor mapping,
- probe and storage descriptors,
- projection metadata,
- enough common structure for `Forge` to reason about loading and reads.

### Backend Payload

Carries:
- the backend-native executable content,
- any backend-private execution data required to initialize runtime execution,
- optional backend-specific metadata that can still be validated through common
  envelope rules.

This split preserves:
- one ecosystem,
- backend purity,
- explicit interoperability.

------------------------------------------------------------------------

## 16. Serialization And In-Memory Form

This draft intentionally does not require that:
- serialized artifact format,
- in-memory loaded artifact form,
- and backend-private executable unpacking format

all be identical.

Allowed future designs:
- one serialized format plus one internal loaded representation,
- direct in-memory artifact objects for non-file paths,
- backend-specific unpacking after `Forge` validation.

The invariant is not identical representation.
The invariant is preserved contract meaning.

------------------------------------------------------------------------

## 17. Determinism Requirements

For deterministic input structure, deterministic compiler settings, and
deterministic target selection, artifact emission should be deterministic.

That includes:
- stable descriptor ordering,
- stable ID assignment,
- stable compatibility declarations,
- stable projection metadata,
- stable payload ordering where applicable.

Reason:
- diffability,
- reproducibility,
- testability,
- benchmark provenance,
- artifact caching later if desired.

------------------------------------------------------------------------

## 18. Diagnostic Requirements

Artifact creation must be accompanied by diagnostics sufficient to explain:
- whether compilation succeeded,
- whether recognition was report-only or replacing,
- whether target projection occurred,
- whether approximation occurred,
- whether target capability constraints shaped lowering,
- whether emission succeeded with warnings.

Some of this may live outside the artifact as compiler diagnostics.

But meaningful runtime-relevant semantic loss must also be discoverable from the
artifact itself or metadata loaded with it.

------------------------------------------------------------------------

## 19. Minimum Load Validation Rules

Before a session is created, `Forge` should be able to validate:
- artifact format version is supported,
- target backend identity or class is compatible,
- required capability class is present,
- required extension families are present if declared,
- descriptor sections are structurally valid,
- payload section is present and well-formed enough for backend handoff.

If any of these fail:
- load must fail cleanly,
- no runtime session should be created,
- the reason must be inspectable through diagnostics.

------------------------------------------------------------------------

## 20. Minimum First-Cut Artifact Scope

The first admitted shared artifact slice should stay intentionally narrow.

Recommended first-cut support:
- single target backend declaration,
- stable input descriptor table,
- stable output descriptor table,
- minimal runtime-visible descriptor table,
- minimal probe descriptor table,
- executable payload sufficient for one backend smoke path,
- projection metadata presence even if empty.

This is enough to validate:
- `Breadboard` to `Forge` handoff,
- load validation,
- session creation,
- common reads,
- projection metadata plumbing.

------------------------------------------------------------------------

## 21. What The Draft Must Not Decide Too Early

This draft must not silently lock:
- final binary field ordering,
- packed versus unpacked storage format,
- compression strategy,
- whether all backends serialize identical payload internals,
- final extension-metadata encoding,
- debug-only artifact content.

Those are later implementation decisions.

The purpose here is to define what must be expressible, not how every byte is
laid out.

------------------------------------------------------------------------

## 22. Validation Requirements

This artifact draft will need eventual validation through:
- deterministic artifact emission tests,
- load validation tests,
- descriptor reconstruction tests,
- target compatibility rejection tests,
- probe descriptor tests,
- storage descriptor tests,
- projection metadata visibility tests,
- `Forge` session-creation smoke tests.

Important rule
- the artifact is only real once both compiler emission and runtime loading can
  agree on it through the public boundaries.

------------------------------------------------------------------------

## 23. Risks To Avoid

1. Designing the artifact around one backend's private runtime layout.
2. Making the artifact too thin for `Forge` to expose useful common reads.
3. Making the artifact too fat by embedding authoring-layer baggage.
4. Hiding projection or approximation inside opaque payload-only behavior.
5. Treating backend-targeted payload differences as a failure of the model.
6. Freezing byte layout before the contract meaning is stable.

------------------------------------------------------------------------

## 24. Relationship To Other Design Docs

This draft follows:
- [STRATA_FORGE_RUNTIME_API_PLAN.md](/c:/DEV/Strata/docs/STRATA_FORGE_RUNTIME_API_PLAN.md)
- [STRATA_STATE_PROJECTION_POLICY.md](/c:/DEV/Strata/docs/STRATA_STATE_PROJECTION_POLICY.md)
- [STRATA_BREADBOARD_BOUNDARY_SPEC.md](/c:/DEV/Strata/docs/STRATA_BREADBOARD_BOUNDARY_SPEC.md)
- [STRATA_BACKEND_CAPABILITY_REGISTRY.md](/c:/DEV/Strata/docs/STRATA_BACKEND_CAPABILITY_REGISTRY.md)

This draft should guide later implementation of:
- artifact headers,
- load validators,
- compiler emission structures,
- runtime descriptor tables,
- backend payload adapters.

------------------------------------------------------------------------

## 25. High-Level Conclusion

The shared artifact is not a universal engine truth object.

It is the disciplined handoff object between:
- structural compilation,
- shared runtime loading,
- backend-native execution.

If this draft stays honest:
- `Breadboard` can emit target-aware executable intent,
- `Forge` can load and validate it safely,
- `LXS` can stay fast,
- `HighZ` can stay temporal,
- and Strata can remain one layered system without collapsing its engines into
  one compromised core.
