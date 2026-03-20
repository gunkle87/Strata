# Strata Breadboard Boundary Spec

Purpose
- Define the ownership boundary for `Breadboard`, the Strata structural
  compiler layer.
- Specify what `Breadboard` is allowed to consume, what it is allowed to
  produce, and what it must never absorb from adjacent layers.
- Establish the compiler-side contract that will later constrain the shared
  artifact draft.

Status
- Planning and boundary document.
- This file defines responsibilities, invariants, and output obligations.
- It does not finalize the shared artifact binary layout.
- Current phase-close note:
  - `Breadboard` now has one real executable lowering path for the admitted
    `FAST_4STATE` subset.
  - That path remains intentionally narrow and uses the current temporary
    shared envelope rather than claiming the final Strata artifact design.
  - Everything outside that admitted subset remains placeholder-oriented,
    explicitly rejected, or deferred.

Core doctrine
- `structure is the authoring layer`
- `function is the execution layer`

This means:
- `Breadboard` is the translation layer between structure and function,
- `Breadboard` is not the authoring model,
- `Breadboard` is not the runtime API,
- `Breadboard` is not backend execution.

------------------------------------------------------------------------

## 1. Why Breadboard Exists

Strata needs a place where authored structure becomes executable function
without forcing:
- UI semantics into the runtime,
- recognition into backend hot paths,
- backend-specific semantic lowering into authoring,
- runtime API concerns into structural translation.

That place is `Breadboard`.

`Breadboard` exists to:
- ingest structure,
- normalize it,
- validate it,
- recognize higher-order patterns,
- apply backend-targeted legality and projection rules,
- emit executable artifacts suitable for runtime loading.

Without `Breadboard`, the system drifts into one of two bad patterns:
- backends becoming accidental compilers,
- or authoring becoming accidentally backend-specific.

------------------------------------------------------------------------

## 2. Scope

`Breadboard` owns:
- structural import,
- structural normalization,
- structural validation,
- name and identity stabilization for emitted artifacts,
- recognition,
- macro semantic substitution,
- backend-targeted legality checks,
- backend-targeted state projection,
- executable artifact emission,
- compiler diagnostics about translation and approximation.

`Breadboard` does not own:
- UI editing operations,
- runtime session lifecycle,
- direct execution control,
- backend-native scheduling,
- backend-private state storage,
- benchmark orchestration,
- tooling that is only format conversion and not structural compilation.

------------------------------------------------------------------------

## 3. Inputs To Breadboard

`Breadboard` may consume:

### 3.1 Authored Structure

Examples
- circuits from the future authoring layer,
- UI-authored component and connection graphs,
- structurally complete design snapshots.

### 3.2 Imported Structural Forms

Examples
- `.bench`
- BLIF-derived structural forms
- other admitted structural netlist formats

Important rule
- pure format conversion alone is not `Breadboard`.
- standalone converters remain tooling-layer assets.

### 3.3 Compiler Configuration

Examples
- target backend selection,
- strictness flags,
- lowering policy,
- projection policy switches that are explicitly admitted,
- naming and canonicalization controls.

### 3.4 Library And Semantic Catalog Inputs

Examples
- primitive catalog,
- macro semantic definitions,
- legality tables,
- backend capability-informed lowering rules.

------------------------------------------------------------------------

## 4. Outputs From Breadboard

`Breadboard` may produce:

### 4.1 Executable Artifact Families

These are runtime-loadable artifacts suitable for `Forge`.

They must contain enough information for:
- backend compatibility validation,
- session creation,
- input and output identity mapping,
- runtime-visible descriptor mapping,
- probe descriptor mapping where required,
- backend-targeted executable content.

Important rule
- this spec defines required output meaning, not final file or memory layout.

### 4.2 Compiler Diagnostics

Examples
- structural validation failures,
- unsupported construct failures,
- recognition reports,
- lowering reports,
- projection and approximation reports,
- target-backend compatibility reports.

### 4.3 Metadata Required For Runtime Inspection

Examples
- stable IDs,
- names,
- widths,
- classes,
- ownership relationships relevant to `Forge` reads.

------------------------------------------------------------------------

## 5. Breadboard Responsibilities

### 5.1 Structural Import

`Breadboard` may accept:
- imported netlist-like structure,
- normalized intermediate structure,
- authored structure snapshots.

It must convert them into one internal structural model suitable for:
- validation,
- recognition,
- lowering,
- artifact emission.

### 5.2 Structural Normalization

`Breadboard` must normalize structure enough to make:
- validation deterministic,
- recognition deterministic,
- backend-targeted lowering deterministic.

Examples
- canonical ordering,
- stable identity assignment,
- normalization of equivalent structural forms,
- flattening only where explicitly part of the compiler policy.

### 5.3 Structural Validation

`Breadboard` must validate:
- structural completeness,
- connectivity legality,
- primitive signature legality,
- macro legality,
- backend-targeted legality when a target backend is selected.

Validation failures must be explicit.
No silent approximation by default.

### 5.4 Recognition

Recognition belongs here.

Recognition responsibilities:
- motif detection,
- report-only census where appropriate,
- legality checks for replacement candidates,
- explicit substitution decisions,
- emission of better executable function where admitted.

Hard rule
- recognition must not drift into backend runtime hot loops.

### 5.5 Macro Semantic Substitution

`Breadboard` owns:
- macro semantic identity,
- substitution legality,
- replacement meaning,
- port mapping,
- object-level lowering into backend-targeted executable function.

Hard rule
- macro semantic definition belongs above execution.
- macro execution kernels belong beside or inside the backends.

### 5.6 Backend-Targeted Projection

`Breadboard` owns:
- state projection for unsupported semantics,
- target-specific legality gating,
- approximation reporting,
- initialization-state handling required before steady-state execution.

This must follow:
- [STRATA_STATE_PROJECTION_POLICY.md](/docs/reference/STRATA_STATE_PROJECTION_POLICY.md)

### 5.7 Executable Artifact Emission

`Breadboard` must emit artifacts that `Forge` can load and validate.

That means emitted artifacts must carry enough meaning to support:
- backend identity checks,
- capability-aware load validation,
- stable I/O descriptor mapping,
- probe descriptor mapping where applicable,
- runtime-visible object mapping needed by the common API.

------------------------------------------------------------------------

## 6. What Breadboard Must Not Absorb

`Breadboard` must not absorb:
- live session control,
- runtime reset logic,
- input commit semantics,
- per-step advancement logic,
- backend scheduling loops,
- backend probe storage reads,
- UI interaction logic,
- benchmark harness behavior,
- generic tooling-only file conversion that does not require structural
  compilation.

Hard rule
- if a change makes `Breadboard` look like a runtime or a UI, the boundary is
  being violated.

------------------------------------------------------------------------

## 7. Relationship To Authoring

The authoring layer owns:
- user edits,
- structural graph manipulation,
- hierarchy editing,
- connection intent,
- design-time object identity.

`Breadboard` consumes a structural snapshot or equivalent structural input.

Important rule
- `Breadboard` should not reach back into authoring for live mutable state.

That means:
- no direct editor callback ownership,
- no dependence on UI layout concerns,
- no accidental coupling to scene graph or visual selection state.

------------------------------------------------------------------------

## 8. Relationship To Forge

`Forge` and `Breadboard` are adjacent but distinct.

`Breadboard` produces:
- backend-targeted executable artifacts,
- compiler diagnostics,
- metadata required for runtime visibility.

`Forge` consumes:
- those executable artifacts,
- their compatibility metadata,
- the runtime-visible descriptor mapping needed for common reads.

Important rule
- `Forge` validates and loads.
- `Breadboard` translates and emits.

Neither should silently absorb the other.

------------------------------------------------------------------------

## 9. Relationship To Backends

Backends own:
- native runtime truth,
- scheduling,
- execution kernels,
- state transitions,
- backend-private diagnostics.

`Breadboard` may target backends explicitly, but it must not become a backend.

That means:
- no backend runtime loops inside `Breadboard`,
- no backend-private state arrays,
- no hot-path execution fallback logic,
- no direct session mutation as part of compilation.

------------------------------------------------------------------------

## 10. Artifact Requirements Without Freezing Layout

This boundary spec intentionally avoids choosing final artifact structure too
early.

What `Breadboard` outputs must guarantee:
- target backend identity is explicit,
- target compatibility can be validated,
- runtime-visible object descriptors can be reconstructed,
- stable IDs and mappings are available,
- any projection or approximation relevant to runtime interpretation can be
  discovered,
- emitted function is executable by the target backend without authoring-layer
  context.

What this document does not decide:
- exact binary format,
- exact in-memory layout,
- one shared universal artifact versus a family of related artifact types,
- whether serialization and in-memory representation are identical.

Those belong to the later shared artifact draft.

------------------------------------------------------------------------

## 11. Targeting Model

`Breadboard` must compile with explicit target awareness.

That means the compiler knows:
- target backend,
- target capability class,
- target state-model limits,
- target legal primitive and macro families,
- target approximation rules.

This allows:
- backend-pure execution,
- explicit legality failures,
- explicit projection reporting,
- no hidden runtime reinterpretation.

Hard rule
- one structure input may lower differently for different backends.
- that is expected and must be explicit.

------------------------------------------------------------------------

## 12. Descriptor And Identity Requirements

Artifacts emitted by `Breadboard` must preserve enough stable identity to make
`Forge` useful.

At minimum, emitted runtime-visible metadata should support:
- input enumeration,
- output enumeration,
- stable input and output IDs,
- stable name lookup where names are present,
- width information,
- class information,
- probe descriptor mapping where probes are part of the emitted artifact,
- storage-object descriptor mapping where common reads require it.

Hard rule
- runtime-visible IDs must be deterministic for a deterministic compiler pass.

------------------------------------------------------------------------

## 13. Diagnostic Requirements

`Breadboard` diagnostics must be good enough to explain:
- why a structural input is invalid,
- why a target backend rejects a construct,
- when recognition was report-only versus admitted,
- when projection or approximation occurred,
- when compilation succeeded with caveats,
- what runtime-affecting choices were made during lowering.

Important rule
- approximation must never be silent.

------------------------------------------------------------------------

## 14. Tooling Boundary Rule

Not every tool that touches structure is `Breadboard`.

Standalone tooling remains separate when its job is primarily:
- format conversion,
- corpus preparation,
- report generation,
- benchmark orchestration,
- fixture production.

Examples
- a `.blif` to `.bench` converter belongs in tooling unless it becomes a real
  structural compiler stage with backend-targeted lowering responsibilities.

This rule exists to prevent `Breadboard` from becoming a junk drawer.

------------------------------------------------------------------------

## 15. Validation Requirements

`Breadboard` must eventually be validated through:
- structural import tests,
- normalization determinism tests,
- validation failure tests,
- recognition report-only tests,
- recognition replacement tests,
- target-backend legality tests,
- projection policy tests,
- artifact emission tests,
- `Forge` load compatibility smoke tests.

Important rule
- validation should prove the boundary works, not only that one backend happens
  to run a case.

------------------------------------------------------------------------

## 16. Risks To Avoid

1. Letting `Breadboard` become a disguised runtime.
2. Letting standalone tooling drift into structural compilation without naming
   that transition.
3. Freezing the artifact layout too early for convenience.
4. Hiding backend-targeted approximation inside vague compiler passes.
5. Making emitted artifacts depend on authoring-layer runtime state.
6. Letting recognition become inseparable from one backend's internal kernels.
7. Emitting artifacts that `Forge` cannot validate cleanly.

------------------------------------------------------------------------

## 17. Relationship To Other Design Docs

This document follows:
- [STRATA_FORGE_RUNTIME_API_PLAN.md](/docs/boundaries/STRATA_FORGE_RUNTIME_API_PLAN.md)
- [STRATA_STATE_PROJECTION_POLICY.md](/docs/reference/STRATA_STATE_PROJECTION_POLICY.md)

This document should guide:
- `docs/reference/STRATA_BACKEND_CAPABILITY_REGISTRY.md`
- `docs/boundaries/STRATA_SHARED_PLAN_DRAFT.md`

Key sequencing rule
- the shared artifact draft must be derived from:
  - what `Breadboard` must guarantee,
  - what `Forge` must be able to consume,
  - what the backends can actually execute.

------------------------------------------------------------------------

## 18. High-Level Conclusion

`Breadboard` is the structural compiler layer in Strata.

Its job is to turn structure into backend-targeted executable function with:
- deterministic normalization,
- explicit validation,
- explicit recognition,
- explicit projection,
- and runtime-loadable output.

If this boundary stays clean:
- authoring can stay structural,
- `Forge` can stay runtime-facing,
- `LXS` can stay fast,
- `HighZ` can stay temporal,
- and the later artifact draft can be designed from real boundary obligations
  instead of guesswork.

Current phase-close interpretation:
- `Breadboard` is now real for one admitted executable path only.
- That real path covers legality checks, deterministic lowering, descriptor
  truth, and executable artifact emission for the first `FAST_4STATE` slice.
- This document must not be read as evidence of broad compiler completeness or
  temporal-backend readiness.


