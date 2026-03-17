# Strata State Projection Policy

Purpose
- Define how Strata handles state-model differences across backends.
- Preserve native backend execution truth without losing cross-backend
  interoperability.
- Establish the policy boundary between:
  - authored intent,
  - structural compilation,
  - shared runtime observation,
  - backend-native execution.

Status
- Planning and policy document.
- This file defines required behavior and ownership rules.
- It does not define final binary artifact layout or final in-memory encoding.

Core doctrine
- `structure is the authoring layer`
- `function is the execution layer`
- `normalize for interoperability, preserve native truth for execution`

------------------------------------------------------------------------

## 1. Why This Policy Exists

Strata is intentionally multi-backend.

That means different backends may preserve different native runtime truths:
- `LXS`
  - 4-state
  - zero-delay
  - performance-first
- `HighZ`
  - 7-state
  - 3-phase delta
  - temporal
  - granularity-first

Without a projection policy, the system will drift into one of two bad states:
- forcing one backend to carry the other backend's runtime truth,
- or pretending the state-model difference does not matter.

Neither is acceptable.

This policy exists so that:
- each backend can stay pure,
- `Breadboard` knows what must be lowered or projected,
- `Forge` knows what the portable observation contract is,
- upper layers can compare results without demanding identical internals.

------------------------------------------------------------------------

## 2. Scope

This policy governs:
- common observable state exposed by `Forge`,
- backend-native state preservation,
- translation-time projection of unsupported states,
- reset and initialization handling for non-steady-state values,
- explicit approximation rules when targeting a backend with a smaller state
  model.

This policy does not govern:
- authored structure editing semantics,
- exact backend internal bit encoding,
- exact runtime artifact schema,
- electrical modeling beyond what Strata intentionally supports,
- arbitrary future state classes not yet admitted.

------------------------------------------------------------------------

## 3. Core Rules

1. Native backend truth must be preserved.
   - `LXS` remains natively 4-state.
   - `HighZ` remains natively 7-state.

2. Shared observation may normalize state for interoperability.
   - Common API reads may expose a portable normalized view.

3. Unsupported state distinctions must be handled before steady-state execution
   when targeting a backend that cannot represent them natively.

4. Projection is explicit semantic approximation.
   - It must never be hidden.

5. `Forge` exposes portable truth and capability-gated native truth.
   - It does not redefine backend semantics.

6. `Breadboard` owns backend-targeted projection during lowering.
   - Backends should not accumulate ad hoc projection logic in hot paths.

------------------------------------------------------------------------

## 4. State Ownership By Layer

### 4.1 Authoring Layer

Owns:
- user-authored structure,
- explicit component and connection intent,
- design-time properties,
- pre-runtime initialization intent if such metadata exists.

Does not own:
- backend-native runtime state semantics,
- runtime state projection,
- execution-time resolution behavior.

### 4.2 Breadboard

Owns:
- backend-targeted state legality checks,
- projection of unsupported state distinctions,
- initialization-state handling before runtime begins,
- explicit declaration of semantic approximation during lowering.

Does not own:
- backend-private runtime storage,
- live runtime resolution loops,
- public runtime read APIs.

### 4.3 Forge

Owns:
- shared portable observation contract,
- capability-gated richer state access,
- stable read semantics for callers.

Does not own:
- deciding backend-native truth,
- compiler lowering,
- hidden re-interpretation of runtime semantics.

### 4.4 Backends

Own:
- native execution truth,
- native state storage,
- native scheduling and resolution behavior,
- backend-private intermediate state.

Do not own:
- cross-backend semantic negotiation,
- authoring-level meaning,
- tool-facing state normalization policy.

------------------------------------------------------------------------

## 5. Portable Common Observation Contract

The shared common state view exposed through `Forge` should be the portable
four-state set:

- `0`
- `1`
- `X`
- `Z`

Reason
- this is the cleanest common denominator across the currently intended
  backends,
- `LXS` already maps naturally to it,
- `HighZ` can project to it without forcing `LXS` to become richer-state.

Important rule
- this is a read contract, not a command to flatten all execution semantics.

Implications
- `LXS` common reads can expose native state directly,
- `HighZ` common reads may expose projected state,
- `HighZ` may expose richer native state through an extension path discovered
  through `Forge`.

------------------------------------------------------------------------

## 6. Backend-Native Truth

### 6.1 LXS

`LXS` native runtime truth remains:
- `0`
- `1`
- `X`
- `Z`

Additional rule
- `LXS` must not absorb richer runtime state semantics if doing so weakens its
  fast-backend identity.

### 6.2 HighZ

`HighZ` native runtime truth remains its richer-state model.

Current planning assumption
- 7-state native truth

Important rule
- the exact richer-state vocabulary must remain documented as `HighZ` native
  truth,
- but it is not required that the common API expose all of it directly.

------------------------------------------------------------------------

## 7. Unsupported State Handling

Unsupported state handling must be explicit and backend-targeted.

There are three relevant categories:
- initialization-state values,
- richer strength or quality distinctions,
- backend-specific native states with no common equivalent.

### 7.1 Initialization-State Values

Example
- `UNINIT`

Policy
- `UNINIT` should not survive into steady-state `LXS` execution.
- It should be resolved during:
  - initialization,
  - reset preparation,
  - or first-resolution setup before normal advancement begins.

For richer-state backends
- `UNINIT` may remain meaningful during controlled initialization paths only if
  that is part of the backend's intentional model.
- It should still not drift into ordinary steady-state execution unless that
  behavior is explicitly part of the backend contract.

### 7.2 Strength-Distinction Values

Examples
- `WEAK_0`
- `WEAK_1`

Policy
- preserve them in backends that support them natively,
- project them explicitly when targeting a backend that does not.

### 7.3 Backend-Specific Native Values

Policy
- preserve natively where supported,
- expose through capability-gated extension reads where needed,
- project only when a portable common view is required.

------------------------------------------------------------------------

## 8. Projection Timing Rule

Projection should happen during structural compilation whenever possible.

That means:
- `Breadboard` knows the target backend,
- `Breadboard` applies target legality checks,
- `Breadboard` projects unsupported states into a form the target backend can
  execute,
- backends receive runtime-ready executable intent.

This is preferred over:
- runtime hot-path projection,
- scattered backend-local fallback logic,
- silent interpretation at read time.

Reason
- it keeps the runtime cleaner,
- it keeps the approximation policy visible,
- it avoids contaminating the fast backend with extra semantic baggage.

------------------------------------------------------------------------

## 9. Projection Policy For Current Known Cases

### 9.1 `UNINIT`

Policy
- must be resolved before steady-state `LXS` execution.

Allowed resolution points
- compile-time initialization lowering,
- load-time initialization setup,
- reset-time state preparation,
- first-resolution setup before ordinary advancement begins.

Disallowed behavior
- carrying `UNINIT` through ordinary `LXS` runtime steps as if it were native
  steady-state truth.

### 9.2 `WEAK_0` and `WEAK_1`

Policy for `HighZ`
- preserve natively.

Policy for `LXS`
- project explicitly during lowering.

Possible projected outcomes
- `WEAK_0 -> 0`
- `WEAK_1 -> 1`
- conflict cases may lower to `X`
- disconnected or semantically absent cases may lower to `Z`

Important caution
- these are policy outcomes, not universal truths.
- the exact projection rule must be documented by lowering family or object
  class when ambiguity exists.

### 9.3 Conflict-Or-Contention Cases

Policy
- if a richer-state distinction collapses into uncertainty in the target
  backend, the projected portable outcome should be `X`.

Reason
- `X` is the safest portable way to communicate lost certainty.

### 9.4 No-Drive / High-Impedance Cases

Policy
- if the projected meaning is genuinely no-drive or disconnected observation,
  the portable outcome should be `Z`.

------------------------------------------------------------------------

## 10. Portable Observation Versus Native Observation

Strata should support two observation levels:

### 10.1 Portable Observation

Exposed through the common `Forge` state reads.

Purpose
- UI compatibility,
- tool compatibility,
- cross-backend comparison,
- common scripting behavior.

Guarantee
- returns only the portable normalized state set.

### 10.2 Native Observation

Exposed through capability-gated backend extensions.

Purpose
- richer debugging,
- temporal introspection,
- backend-specific analysis,
- native truth preservation.

Guarantee
- returns backend-native state descriptors and values only when supported.

Hard rule
- portable observation must not pretend to be native observation.

------------------------------------------------------------------------

## 11. API Read Semantics

`Forge` common reads should expose:
- current stable observable state after the last common advancement boundary,
- current outputs in portable normalized form,
- probe snapshots in portable normalized form where probes are part of common
  reads,
- metadata needed to interpret widths and classes.

`Forge` common reads must not expose:
- half-committed backend-private arrays,
- runtime scratch storage,
- temporary resolution lanes,
- implementation-only transient data.

If upper layers need richer introspection:
- that belongs in backend-native extension reads,
- not in leakage through the common API.

------------------------------------------------------------------------

## 12. Compiler Reporting Requirement

Whenever `Breadboard` projects unsupported state semantics for a target backend,
that projection must be reportable.

At minimum, Strata should be able to report:
- target backend,
- projection families applied,
- approximation categories encountered,
- whether any stronger semantic collapse occurred.

This reporting may surface through:
- compiler diagnostics,
- artifact metadata,
- or runtime-loadable metadata visible through `Forge`.

Hard rule
- semantic loss must be visible somewhere.

------------------------------------------------------------------------

## 13. Validation Requirements

Projection policy is not admitted by prose alone.

Validation must eventually include:
- unit tests for projection families,
- load-time tests for initialization handling,
- common API read tests proving normalized outputs are stable,
- backend-native read tests proving richer state is still accessible where
  intended,
- cross-backend comparison tests for portable-state agreement where semantics
  overlap.

Important rule
- these tests should exercise policy through `Breadboard` and `Forge`,
  not by reaching directly into backend internals.

------------------------------------------------------------------------

## 14. Risks To Avoid

1. Silently flattening richer states without recording that fact.
2. Forcing `LXS` to carry richer-state runtime logic and losing speed.
3. Exposing `HighZ` native truth only through the common 4-state view and
   losing its main value.
4. Letting runtime backends accumulate ad hoc projection logic in hot paths.
5. Confusing initialization semantics with ordinary runtime semantics.
6. Treating projected results as if they were semantically identical to native
   richer-state execution.

------------------------------------------------------------------------

## 15. Relationship To Other Design Docs

This policy follows:
- [STRATA_FORGE_RUNTIME_API_PLAN.md](/c:/DEV/Strata/docs/STRATA_FORGE_RUNTIME_API_PLAN.md)

This policy should guide:
- `STRATA_BREADBOARD_BOUNDARY_SPEC.md`
- `STRATA_BACKEND_CAPABILITY_REGISTRY.md`
- `STRATA_SHARED_PLAN_DRAFT.md`

Key dependency rule
- the shared plan draft must honor this projection policy,
- not quietly override it through artifact-format convenience.

------------------------------------------------------------------------

## 16. High-Level Conclusion

Strata does not unify backends by forcing them into one runtime truth.

Strata unifies them by:
- preserving native execution truth where it belongs,
- projecting unsupported semantics during lowering where needed,
- exposing a stable portable observation contract through `Forge`,
- and exposing richer truth through explicit backend capabilities.

That is the only clean way to keep:
- `LXS` fast,
- `HighZ` temporal,
- `Forge` stable,
- and `Breadboard` honest about semantic approximation.
