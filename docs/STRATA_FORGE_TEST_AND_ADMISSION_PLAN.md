# Strata Forge Test And Admission Plan

Purpose
- Define the test strategy and admission bar for the first `Forge`
  implementation slices.
- Turn the `Forge` architecture docs into concrete validation requirements.
- Give Strata a repeatable way to decide when a `Forge` step is ready to keep.

Status
- Planning document
- Applies first to the initial `Forge` skeleton and then to early follow-on
  runtime-boundary phases

Core rule
- `Forge` is admitted by public-boundary behavior, not by internal effort

------------------------------------------------------------------------

## 1. Why This Plan Exists

`Forge` is the shared runtime boundary for Strata.

That means it must be held to a higher bar than ordinary scaffolding:
- stable public shape,
- explicit lifecycle behavior,
- capability-driven behavior,
- clean rejection paths,
- no hidden backend coupling.

If we do not define admission early, `Forge` risks becoming:
- a pile of optimistic stubs,
- a disguised backend,
- or an unstable public surface that changes every task.

This plan exists to prevent that.

------------------------------------------------------------------------

## 2. Scope

This plan governs:
- `Forge` public headers
- `Forge` source-level lifecycle behavior
- backend enumeration and capability reporting
- artifact/session handle behavior
- public error handling
- early smoke tests
- admission criteria for each bounded `Forge` phase

This plan does not govern:
- backend execution correctness
- `Breadboard` compilation correctness
- shared artifact content correctness beyond load validation shape
- UI behavior

------------------------------------------------------------------------

## 3. Core Admission Principles

1. Public behavior is the truth that matters.
   - `Forge` is judged through its public boundary.

2. Opaque-handle discipline is mandatory.
   - Public tests must not depend on private struct layout.

3. Stub behavior is allowed early.
   - But stub behavior must be explicit, deterministic, and testable.

4. Unsupported paths must fail cleanly.
   - “Not implemented” is acceptable early.
   - silent success is not.

5. Capability reporting must be real enough to drive decisions.
   - Even placeholder backend records must be structured and queryable.

6. No backend coupling is allowed for admission.
   - A passing `Forge` phase must not depend on direct backend runtime code.

------------------------------------------------------------------------

## 4. Phase Structure For Forge

Recommended early `Forge` phases:

1. Public type and result skeleton
2. Backend registry and capability skeleton
3. Artifact handle and validation skeleton
4. Session lifecycle skeleton
5. Common read skeleton
6. Probe skeleton
7. Extension discovery skeleton

Each phase should be admitted separately.

------------------------------------------------------------------------

## 5. Test Families

Every `Forge` phase should declare which of these test families it touches.

### 5.1 Public Header Shape Tests

Purpose
- verify the public boundary is coherent and compilable

Examples
- include-order sanity
- typedef visibility
- opaque-handle compilation checks
- enum/result-code availability

### 5.2 Backend Registry Tests

Purpose
- verify backend enumeration and identity behavior

Examples
- enumerate all registered backends
- query `LXS` placeholder record
- query `HighZ` placeholder record
- query backend count
- invalid backend index rejection

### 5.3 Capability Query Tests

Purpose
- verify capability reporting is structured and stable

Examples
- query native state-model class
- query lifecycle support level
- query extension-family presence
- query unsupported capability path cleanly

### 5.4 Artifact Handle Tests

Purpose
- verify load/unload boundary shape and rejection paths

Examples
- invalid artifact input rejection
- unsupported artifact rejection
- null-handle rejection
- clean free of invalid or empty artifact handles if allowed

### 5.5 Session Lifecycle Tests

Purpose
- verify session creation/reset/free behavior

Examples
- create session with invalid artifact fails
- reset invalid session fails
- free null or invalid session follows documented behavior
- lifecycle order violations return explicit errors

### 5.6 Error And Diagnostic Tests

Purpose
- verify error behavior is inspectable

Examples
- invalid argument returns explicit result code
- invalid handle returns explicit result code
- last-error or diagnostic string is available where documented

### 5.7 Boundary Isolation Tests

Purpose
- verify `Forge` is not secretly reaching into disallowed layers

Examples
- build/link tests that do not require backend runtime objects
- source audit checks for forbidden backend includes
- source audit checks for forbidden `Breadboard` or UI dependencies

------------------------------------------------------------------------

## 6. Required Tests For The Initial Forge Skeleton

The first `Forge` implementation slice should not be admitted without:

### Required header tests
- public headers compile in isolation
- public headers compile together
- opaque handle types are visible but incomplete externally

### Required registry tests
- backend enumeration returns at least:
  - `LXS`
  - `HighZ`
- backend identity query works
- invalid backend index is rejected cleanly

### Required capability tests
- capability query returns structured data for each placeholder backend
- `LXS` and `HighZ` differ where expected
- extension-family presence can be queried

### Required lifecycle tests
- artifact load stubs fail or succeed in the documented stub manner
- session create/reset/free stubs return explicit result codes
- invalid-handle paths are tested

### Required diagnostics tests
- invalid args produce explicit result codes
- unsupported operations produce explicit result codes

------------------------------------------------------------------------

## 7. Explicit Non-Admission Conditions

The initial `Forge` skeleton must not be admitted if:
- public headers expose backend-private struct layout
- backend enumeration is hardcoded but not queryable through the public API
- capability reporting is absent or vague
- artifact/session paths silently succeed without meaningful behavior
- invalid-handle paths are not tested
- direct backend execution code is introduced
- `Breadboard` code is introduced
- converter or tooling code becomes a runtime dependency

------------------------------------------------------------------------

## 8. Public Result-Code Expectations

Early `Forge` phases should define and test explicit result families such as:
- success
- invalid argument
- invalid handle
- invalid lifecycle order
- unsupported capability
- incompatible artifact
- internal failure
- not implemented

Important rule
- `not implemented` is acceptable in early phases
- only if it is deliberate, explicit, and tested

------------------------------------------------------------------------

## 9. Placeholder Backend Admission Rule

Placeholder backend support is acceptable for early `Forge`.

But the placeholder records must still be:
- distinct
- queryable
- capability-shaped
- useful enough for `Breadboard` and tooling planning

That means:
- `LXS` and `HighZ` should not collapse into one generic placeholder backend
- even before real backend integration begins

------------------------------------------------------------------------

## 10. Build And Test Expectations

Every `Forge` phase should report:
- exact build command
- exact test command
- whether tests were run from a clean state
- whether the tests exercise only the public boundary

Preferred outcome
- one simple command for build
- one simple command for test

If that does not exist yet:
- the phase should document the exact commands needed

------------------------------------------------------------------------

## 11. Audit Expectations

After each non-trivial `Forge` implementation phase, the audit should check:
- scope compliance
- no hidden backend coupling
- no `Breadboard` or UI leakage
- public-boundary opacity
- test coverage for the required families
- rule/style compliance

This is especially important in the first few phases because boundary drift is
most likely early.

------------------------------------------------------------------------

## 12. Evidence Required In Completion Reports

Each `Forge` implementation report should include:
- files added
- files changed
- test files added
- build command run
- test command run
- current stub limitations
- unimplemented functions or capability gaps
- whether the phase is considered admission-ready or still provisional

------------------------------------------------------------------------

## 13. Recommended Early Admission Checkpoints

### Checkpoint 01
- public types
- result codes
- opaque handles

### Checkpoint 02
- backend registry
- backend identity
- capability queries

### Checkpoint 03
- artifact handle skeleton
- artifact rejection paths

### Checkpoint 04
- session lifecycle skeleton
- lifecycle order enforcement

### Checkpoint 05
- common read skeleton
- descriptor access skeleton

Only admit a checkpoint when its required test family passes.

------------------------------------------------------------------------

## 14. Risks To Avoid

1. Treating placeholder code as automatically admissible.
2. Letting `Forge` grow without tests because “real runtime logic comes later.”
3. Hiding unimplemented behavior behind fake success returns.
4. Letting backend distinctions disappear in the placeholder phase.
5. Testing implementation internals instead of the public API.
6. Skipping rejection-path tests.

------------------------------------------------------------------------

## 15. Relationship To Other Docs

This plan follows:
- [STRATA_FORGE_RUNTIME_API_PLAN.md](/c:/DEV/Strata/docs/STRATA_FORGE_RUNTIME_API_PLAN.md)
- [STRATA_BACKEND_CAPABILITY_REGISTRY.md](/c:/DEV/Strata/docs/STRATA_BACKEND_CAPABILITY_REGISTRY.md)
- [STRATA_MULTI_AGENT_TRIAL_PROTOCOL.md](/c:/DEV/Strata/docs/STRATA_MULTI_AGENT_TRIAL_PROTOCOL.md)

This plan should be used when writing:
- implementation directives
- audit directives
- checkpoint notes for `Forge`

------------------------------------------------------------------------

## 16. High-Level Conclusion

The first `Forge` phases do not need to execute real simulation yet.

But they do need to prove that the runtime boundary is:
- explicit,
- testable,
- capability-shaped,
- opaque,
- and honest about what is still stubbed.

That is the admission bar for a usable Strata runtime boundary.
