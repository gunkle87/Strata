# Strata Forge Profile Enforcement Plan

Purpose
- Define how `Forge` enforces build profiles, product profiles, and session
  policy.
- Translate the product-profile and feature-gating docs into implementation
  ownership for the runtime boundary.
- Prevent product clipping from becoming an inconsistent patchwork of special
  cases.

Status
- Planning and implementation-facing design document.
- This file defines enforcement responsibilities and expected runtime
  behavior.
- It does not finalize the exact C structs, file layout, or configuration
  syntax.

Core doctrine
- `Forge` is the enforcement boundary
- capability discovery must describe post-gating reality
- forbidden access must fail explicitly

------------------------------------------------------------------------

## 1. Why This Plan Exists

Strata now has:
- a product-profile model,
- a feature-gating policy,
- a shared runtime boundary,
- a backend capability model.

That is enough to define *what* Strata wants, but not yet enough to define
*how `Forge` should enforce it*.

This plan exists to answer:
- where gating state lives,
- when it is applied,
- what gets filtered,
- how forbidden access behaves,
- and what future code slices should implement first.

------------------------------------------------------------------------

## 2. Enforcement Scope

This plan applies to:
- backend discovery,
- capability discovery,
- artifact acceptance,
- session creation,
- descriptor enumeration,
- descriptor lookup,
- probe enumeration and reads,
- common read and input paths,
- backend-native extension entrypoints.

This plan does not apply to:
- authored structure editing,
- `Breadboard` internals beyond target availability decisions,
- UI-layer access control,
- security outside the Strata runtime boundary,
- backend-private execution logic.

------------------------------------------------------------------------

## 3. The Three Inputs To Enforcement

`Forge` enforcement is derived from three inputs.

### 3.1 Build Profile

This describes what was compiled into the shipped binary.

Examples:
- whether `LXS` is present,
- whether `HighZ` is present,
- whether probe support is present,
- whether native-state extension code is present.

### 3.2 Product Profile

This describes what the product intends to expose from the compiled set.

Examples:
- whether a backend is visible,
- whether native reads are allowed,
- whether specific probe classes are exposed,
- whether advanced runtime controls are allowed.

### 3.3 Session Policy

This describes safe narrowing applied to a specific session.

Examples:
- selected backend within the product-allowed set,
- common-only observation mode,
- reduced probe surface for one session type.

Important rule
- session policy may narrow only
- it must never widen build or product permissions

------------------------------------------------------------------------

## 4. Recommended Forge Enforcement Layers

`Forge` should enforce profiles at four concrete layers.

### 4.1 Discovery Filtering

Controls:
- backend enumeration
- capability enumeration
- extension-family discovery
- descriptor counts and descriptor listings

Meaning:
- if the product forbids it, it should not appear here

### 4.2 Direct Call Validation

Controls:
- explicit API calls to restricted features,
- lookup by ID,
- lookup by name,
- extension entrypoints,
- privileged probe access,
- native input submission,
- native read paths.

Meaning:
- even if the caller guesses the path, `Forge` rejects it

### 4.3 Artifact Admission

Controls:
- whether an artifact may be loaded at all,
- whether a target backend is allowed,
- whether the artifact requires denied extension families.

Meaning:
- product restrictions must be checked before session execution begins

### 4.4 Session-Time Narrowing

Controls:
- per-session feature reduction inside the product-allowed surface

Meaning:
- sessions can be more restrictive than the product
- never less restrictive

------------------------------------------------------------------------

## 5. Required Forge Policy Objects

Strata should eventually define policy objects in `Forge` with three logical
layers.

### 5.1 Build Capability Set

Represents compiled-in availability.

Should answer:
- which backends exist,
- which extension families exist,
- which probe families exist,
- which advanced read/write paths exist.

### 5.2 Product Exposure Profile

Represents what this shipped product may expose.

Should answer:
- which backends are visible,
- which API families are exposed,
- which native extensions are allowed,
- which probe visibility classes are exposed,
- which artifact targets are accepted.

### 5.3 Session Restriction Profile

Represents safe per-session narrowing.

Should answer:
- whether the session is common-only,
- whether probe access is narrowed further,
- whether advanced controls are disabled for this session type.

Important implementation rule
- `Forge` should derive one effective profile from these layers
- call sites should not manually reason about all three separately

------------------------------------------------------------------------

## 6. Effective Profile Computation

The runtime should compute an effective policy from:
- build profile,
- product profile,
- session policy.

Conceptually:
- build profile defines the maximum possible set
- product profile intersects that set
- session policy narrows it further

The effective profile should be treated as authoritative for:
- discovery,
- descriptor filtering,
- probe filtering,
- direct-call validation,
- artifact/session admission decisions.

------------------------------------------------------------------------

## 7. Discovery Filtering Rules

Discovery must report the effective profile, not raw engine potential.

### 7.1 Backend Discovery

If a backend is:
- not compiled, it must not exist in discovery
- compiled but product-hidden, it must not appear in discovery
- session-denied, it must not appear as selectable for that session

### 7.2 Capability Discovery

Capability flags and extension families must reflect:
- compiled presence,
- product permission,
- session narrowing.

### 7.3 Descriptor Discovery

Descriptor counts and enumerations must be filtered by the effective profile.

This matters for:
- inputs,
- outputs,
- probes,
- storage objects later,
- native descriptor families later.

------------------------------------------------------------------------

## 8. Probe Filtering Rules

Probe filtering should be class-based and enforced in all lookup modes.

Recommended visibility classes:
- common public
- backend public
- backend privileged
- internal only

Required enforcement:
- denied probe classes do not contribute to visible counts
- denied probe classes do not appear in descriptor-by-index results
- denied probe classes do not resolve by ID
- denied probe classes do not resolve by name
- direct probe reads against denied probes fail explicitly

This should be implemented once in `Forge`, not reimplemented per call site.

------------------------------------------------------------------------

## 9. Native-State And Native-Input Enforcement

Native backend semantics may remain fully intact internally while public
surface is reduced.

Examples of policy-limited exposure:
- native richer-state reads denied
- common normalized reads allowed
- native input value submission denied
- common portable input submission allowed

Important rule
- reduced exposure does not imply reduced backend truth

Implementation consequence
- `Forge` should separate:
  - common portable value paths
  - backend-native value paths

Then gate them independently.

------------------------------------------------------------------------

## 10. Artifact Admission Rules

Artifact loading must be filtered before runtime execution begins.

`Forge` should reject an artifact when:
- its backend target is not compiled,
- its backend target is product-denied,
- it requires an extension family the effective profile does not allow,
- it depends on descriptor or probe classes that the product disallows as
  required runtime-visible surfaces.

Important rule
- the product profile should not let artifact load succeed and then rely on a
  later call to discover that the session is unusable

------------------------------------------------------------------------

## 11. Direct-Call Validation Rules

Even after discovery filtering, callers may still:
- guess descriptor IDs,
- guess names,
- call extension entrypoints directly,
- reuse handles obtained under another policy.

Therefore `Forge` must validate at direct-call time as well.

Required behavior:
- validate the handle
- validate the effective profile
- validate the specific requested class or capability
- then either:
  - proceed, or
  - return explicit forbidden/unsupported result

------------------------------------------------------------------------

## 12. Result Behavior

Strata should make forbidden behavior explicit.

Current likely result classes:
- invalid argument
- invalid handle
- out of bounds
- not found
- incompatible artifact
- unsupported

Recommended near-term addition
- a distinct forbidden/access-denied style result code

Why this matters:
- unsupported means the engine/runtime cannot do it
- forbidden means the product profile will not allow it

Those are not the same thing.

If a distinct forbidden code is not added immediately:
- the gap should be documented,
- and a later cleanup slice should add it before the profile system is treated
  as complete.

------------------------------------------------------------------------

## 13. Required Metadata On Runtime-Visible Objects

To support profile filtering cleanly, runtime-visible objects should eventually
carry enough metadata for policy checks.

Examples:
- descriptor class
- visibility class
- backend ownership
- native/common exposure class
- extension-family dependency if relevant

This metadata belongs in runtime-facing descriptor information, not in UI-only
tables.

------------------------------------------------------------------------

## 14. Recommended Implementation Order

`Forge` should implement profile enforcement in this order.

### Phase 1

- define policy object shapes
- add effective-profile computation
- add a temporary forbidden-result policy decision

### Phase 2

- filter backend discovery
- filter capability discovery
- filter extension-family discovery

### Phase 3

- filter descriptor counts and descriptor lookups
- start with probe and native-observation-sensitive surfaces

### Phase 4

- enforce artifact admission rules
- enforce session-level narrowing rules

### Phase 5

- gate backend-native extension entrypoints
- add broader tests for denial behavior

------------------------------------------------------------------------

## 15. Required Test Families

When enforcement starts landing in code, tests should cover:
- compiled-out backend absent from discovery
- product-hidden backend absent from discovery
- denied native extension absent from capability discovery
- denied probe absent from enumeration
- denied probe lookup by ID fails explicitly
- denied probe lookup by name fails explicitly
- denied native read path fails explicitly
- denied native input path fails explicitly
- denied artifact target rejected before session start
- session narrowing hides features still allowed by the product

------------------------------------------------------------------------

## 16. Relationship To Breadboard

`Breadboard` should eventually be aware of profile-target availability, but it
must not become the enforcement boundary for runtime exposure.

That means:
- `Breadboard` may reject denied targets or target classes,
- `Breadboard` may avoid emitting artifacts incompatible with a product,
- but `Forge` still owns final runtime enforcement.

------------------------------------------------------------------------

## 17. What This Plan Avoids

This plan deliberately avoids:
- backend forks for each product line
- engine semantic rewrites to satisfy exposure policy
- UI-only clipping
- per-call ad hoc policy code scattered everywhere
- discovery that leaks more than the product allows

------------------------------------------------------------------------

## 18. High-Level Conclusion

Strata should enforce product clipping through one coherent `Forge` policy
system.

That system should:
- combine build, product, and session policy,
- compute one effective profile,
- filter discovery and descriptors,
- reject forbidden paths explicitly,
- and preserve native backend truth underneath.

That is the clean route to controlled multi-product shipping without forks and
without patchy late-stage restrictions.
