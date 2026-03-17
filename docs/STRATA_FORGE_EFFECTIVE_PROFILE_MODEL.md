# Strata Forge Effective Profile Model

Purpose
- Define the logical shape of the effective policy object that `Forge` should
  use at runtime.
- Turn build profile, product profile, and session policy into one coherent
  enforcement model.
- Prevent profile enforcement from becoming scattered per-call logic.

Status
- Planning and implementation-facing design document.
- This file defines the logical model and usage rules for effective profile
  computation.
- It does not finalize the exact C struct layout or file ownership.

Core doctrine
- one computed effective profile
- one authoritative runtime truth after gating
- no ad hoc policy checks spread across the API

------------------------------------------------------------------------

## 1. Why This Model Exists

Strata already has policy concepts for:
- build gating,
- product gating,
- session narrowing,
- and `Forge` enforcement.

Without one explicit effective-profile model, future code will drift toward:
- repeating the same checks in many call sites,
- inconsistent filtering between discovery and direct calls,
- policy bugs caused by combining layers differently in different functions,
- brittle behavior that is hard to audit.

This model exists to stop that.

------------------------------------------------------------------------

## 2. What The Effective Profile Is

The effective profile is the runtime policy object produced by combining:
- build profile,
- product profile,
- session policy.

It represents the final allowed surface for one runtime context.

That means it should answer:
- which backends are visible,
- which artifacts are admissible,
- which API families are visible,
- which extension families are allowed,
- which descriptor families are visible,
- which probe classes are visible,
- which native input/read paths are allowed,
- which runtime controls are allowed.

Important rule
- call sites should consume the effective profile
- not reinterpret build/product/session layers independently

------------------------------------------------------------------------

## 3. Inputs To Effective Profile Computation

### 3.1 Build Profile

Defines the compiled maximum.

Examples:
- backend compiled in or absent
- native extension code present or absent
- probe support present or absent

### 3.2 Product Profile

Defines the shipped public surface.

Examples:
- backend exposed or hidden
- native observation denied
- privileged probe classes denied
- advanced controls denied

### 3.3 Session Policy

Defines per-session narrowing.

Examples:
- common-only session
- probe-reduced session
- no-advanced-controls session

Important rule
- session policy may only reduce what remains after build+product intersection

------------------------------------------------------------------------

## 4. Computation Rule

Conceptually:

1. Start from build profile
2. Intersect with product profile
3. Narrow with session policy
4. Produce one effective profile object

That object should be treated as:
- authoritative for discovery
- authoritative for lookups
- authoritative for direct-call validation
- authoritative for artifact/session admission

------------------------------------------------------------------------

## 5. Recommended Logical Sections

The effective profile should be organized by section rather than one flat list
of flags.

Recommended sections:
- backend visibility
- artifact admission
- capability-family exposure
- descriptor visibility
- probe visibility
- state exposure
- input submission
- runtime control
- extension families
- diagnostics and privileged inspection

------------------------------------------------------------------------

## 6. Backend Visibility Section

This section should answer, for each backend:
- absent
- compiled but hidden
- visible and selectable

It should also answer:
- whether the backend may be used for artifact loading
- whether the backend may be selected for new sessions
- whether backend-targeted compile/target requests should be treated as legal

Important rule
- backend visibility is not just an enumeration concern
- it must also affect load and session admission

------------------------------------------------------------------------

## 7. Artifact Admission Section

This section should answer:
- which backend targets are accepted
- which extension-family requirements are accepted
- which artifact compatibility classes are accepted
- whether the product accepts only a subset of otherwise compatible artifacts

This section should support explicit rejection when:
- a target backend is hidden
- a required extension is product-denied
- a required descriptor/probe class is not permitted for the product

------------------------------------------------------------------------

## 8. Capability-Family Exposure Section

This section should answer whether the following families are exposed:
- common lifecycle
- common advancement
- common reads
- common probes
- metadata/mapping
- backend-native read families
- backend-native control families
- privileged inspection families

Important rule
- family exposure should be represented directly
- not inferred later from unrelated flags

------------------------------------------------------------------------

## 9. Descriptor Visibility Section

This section should answer visibility for runtime descriptor families such as:
- inputs
- outputs
- common probes
- backend-native probes
- storage objects later
- privileged/native descriptor families later

It should be sufficient to drive:
- count filtering
- enumeration filtering
- ID lookup filtering
- name lookup filtering

------------------------------------------------------------------------

## 10. Probe Visibility Section

This section should answer:
- which probe visibility classes are visible
- which probe classes are readable
- whether probe values are allowed at all
- whether grouped/snapshot probe reads are allowed later

This section should be compatible with the dedicated probe-visibility model
document, not replace it.

------------------------------------------------------------------------

## 11. State Exposure Section

This section should answer:
- whether common normalized reads are allowed
- whether native richer-state reads are allowed
- whether native state submission paths are allowed
- whether state exposure is common-only for this runtime context

Important rule
- execution truth and exposure policy remain separate

That means:
- a backend may execute natively
- while this section exposes only the common public state surface

------------------------------------------------------------------------

## 12. Input Submission Section

This section should answer:
- whether common portable input submission is allowed
- whether batch input submission is allowed
- whether backend-native input classes are allowed
- whether advanced input submission paths are allowed later

This allows future `Forge` calls to distinguish between:
- legal common submission
- product-forbidden native submission

------------------------------------------------------------------------

## 13. Runtime Control Section

This section should answer:
- whether common single-step is allowed
- whether common multi-step is allowed
- whether bounded run helpers are allowed
- whether advanced backend-native stepping is allowed
- whether temporal sub-step control is allowed

Important rule
- common control and backend-native control should not be merged into a single
  yes/no decision

------------------------------------------------------------------------

## 14. Extension Family Section

This section should answer:
- which extension families are visible
- which extension families are callable
- which extension families are denied by policy

This should drive both:
- capability discovery
- direct-call validation for extension entrypoints

------------------------------------------------------------------------

## 15. Diagnostics And Privileged Inspection Section

This section should answer:
- whether basic diagnostics are visible
- whether advanced counters are visible
- whether privileged inspection paths are visible
- whether internal-only diagnostics must remain hidden

This matters because diagnostics are often where product boundaries drift if
they are not modeled explicitly.

------------------------------------------------------------------------

## 16. Effective Profile Consumers

The effective profile should be consumed by these runtime areas.

### 16.1 Discovery

Used for:
- backend enumeration
- capability enumeration
- extension-family discovery

### 16.2 Artifact Loading

Used for:
- target acceptance
- extension dependency acceptance
- profile-based artifact rejection

### 16.3 Descriptor Lookups

Used for:
- count filtering
- index lookup filtering
- ID lookup filtering
- name lookup filtering

### 16.4 Probe Reads

Used for:
- probe enumeration filtering
- probe value permission checks
- privileged probe rejection

### 16.5 Native Paths

Used for:
- native read gating
- native input gating
- advanced control gating
- extension-family call gating

------------------------------------------------------------------------

## 17. Where The Effective Profile Should Live

The effective profile should be treated as runtime-owned policy state inside
`Forge`.

Recommended ownership pattern:
- build profile available at registry/library scope
- product profile selected or installed at library/runtime configuration scope
- session policy applied at session creation or session attach time
- effective profile stored in a runtime-owned object accessible to the public
  boundary implementation

Important rule
- do not make every public API call recompute the policy from scratch
- compute once, store once, consume consistently

------------------------------------------------------------------------

## 18. Recommended Near-Term Implementation Order

### Phase 1

- define logical struct sections
- define default unrestricted skeleton behavior
- compute an effective profile placeholder

### Phase 2

- wire effective profile into backend discovery
- wire effective profile into capability discovery

### Phase 3

- wire effective profile into descriptor and probe filtering

### Phase 4

- wire effective profile into artifact admission
- wire effective profile into direct native-path rejection

------------------------------------------------------------------------

## 19. Required Test Families

When this model lands in code, tests should verify:
- effective profile narrows build/profile/session correctly
- session policy cannot widen product permissions
- hidden backend absent from discovery
- denied extension absent from capability discovery
- hidden descriptors absent from counts and lookups
- denied native input/read/control paths reject explicitly

------------------------------------------------------------------------

## 20. High-Level Conclusion

Strata should not scatter product gating logic across the `Forge` API.

It should compute one effective profile and treat it as the authoritative
runtime policy object for:
- discovery
- descriptors
- probes
- artifact admission
- direct-call validation

That is the cleanest path to predictable feature clipping without forks or
patchy later fixes.
