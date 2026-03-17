# Strata Product Profile Model

Purpose
- Define how Strata ships multiple product lines from one codebase.
- Separate engine implementation truth from product exposure policy.
- Prevent UI-only restrictions from becoming the real security boundary.

Status
- Planning and policy document.
- This file defines the profile model and enforcement layers.
- It does not define the final C structs or build-system syntax.

Core doctrine
- one codebase
- one `Forge` runtime boundary
- multiple product profiles
- no backend forks

------------------------------------------------------------------------

## 1. Why Product Profiles Exist

Strata will serve more than one product line.

Those product lines will not all want:
- the same backend set,
- the same probe visibility,
- the same native-state exposure,
- the same control surface,
- or the same shipping footprint.

If those differences are enforced only in UI code, they are not real
restrictions.

Any caller that can reach unrestricted `Forge` APIs can:
- bypass the UI,
- rebuild its own client,
- discover more than the product intended to expose,
- and recover backend-native power the product never meant to ship.

Product profiles exist to stop that.

------------------------------------------------------------------------

## 2. What A Product Profile Is

A product profile is the policy object that defines what a shipped product may
expose through `Forge`.

It answers questions such as:
- which backends are available,
- which common API families are available,
- which extension families are available,
- which probes are visible,
- which state classes may be submitted or read,
- which runtime controls are permitted,
- which descriptor families are visible to discovery.

Important rule
- a product profile governs what clients can actually do
- not just what the default UI happens to render

------------------------------------------------------------------------

## 3. Three Gating Layers

Strata should distinguish three different kinds of gating.

### 3.1 Build Profile

Defines what is compiled into the shipped binary at all.

Examples
- `LXS` compiled in
- `HighZ` compiled in
- probes compiled in
- native-state extension support compiled in

Use build gating for:
- footprint reduction,
- licensing boundaries,
- attack-surface reduction,
- products that must never ship a capability.

### 3.2 Product Profile

Defines what the shipped product exposes from the compiled set.

Examples
- both backends present, but only one exposed
- `HighZ` present, but native richer-state reads disabled
- probes present internally, but only approved public classes visible

Use product gating for:
- SKU definition,
- product-line differentiation,
- supportable public surface control,
- stable customer-facing restrictions.

### 3.3 Session Policy

Defines what is enabled for one specific runtime session.

Examples
- selected backend,
- optional feature toggles already allowed by the product,
- safe per-session control narrowing.

Use session gating for:
- runtime configuration,
- safe narrowing of already-permitted features,
- temporary restriction within a product profile.

Important rule
- session policy may narrow a product profile
- it must not widen it

------------------------------------------------------------------------

## 4. Core Rules

1. No product restriction may depend on UI discipline alone.
2. `Forge` is the enforcement boundary for product exposure.
3. Backends remain semantically truthful internally.
4. Product profiles control exposure, not backend identity.
5. Capability discovery must report what is actually available after gating.
6. Hidden capabilities must not remain discoverable through alternate lookups.
7. A denied capability must fail explicitly, not degrade silently.
8. Session policy may only remove access, never add access.

------------------------------------------------------------------------

## 5. Engine Truth Versus Product Exposure

The backend must not be rewritten into a different engine just to satisfy a
product line.

Examples
- `LXS` remains 4-state and zero-delay
- `HighZ` remains 7-state and 3-phase delta

That does not mean every product must expose those engines fully.

Instead:
- backend truth stays native
- product exposure is clipped at `Forge`
- tooling and UI sit on top of the clipped `Forge` surface

This is the correct separation.

------------------------------------------------------------------------

## 6. Mandatory Exposure Categories

Each product profile should declare policy for these categories.

### 6.1 Backend Presence

Examples
- backend absent
- backend present but not exposed
- backend exposed

### 6.2 Input Submission Surface

Examples
- common portable input values only
- backend-native input values allowed
- batch input submission allowed or denied

### 6.3 Read Surface

Examples
- common normalized reads only
- descriptor reads only
- native richer-state reads allowed or denied

### 6.4 Probe Surface

Examples
- probe family absent
- common probes only
- restricted probe classes only
- native probe extensions allowed or denied

### 6.5 Runtime Control Surface

Examples
- common single-step only
- multi-step allowed
- temporal sub-step controls denied
- advanced backend-native control denied

### 6.6 Artifact/Target Surface

Examples
- may load only `LXS` artifacts
- may load only `HighZ` artifacts
- may target both in `Breadboard`

------------------------------------------------------------------------

## 7. Capability Discovery Must Reflect The Profile

`Forge` must never advertise capabilities that the current build profile,
product profile, and session policy do not actually allow.

That means:
- hidden probe classes must not appear in enumeration,
- denied extension families must not appear as available,
- denied native-state reads must not appear as supported,
- denied backend selection options must not be listed as usable.

Important rule
- capability discovery is authoritative after gating
- it is not a raw dump of internal engine potential

------------------------------------------------------------------------

## 8. Required Enforcement Behavior

When a feature is denied, enforcement must happen at more than one layer.

### 8.1 Discovery Layer

The feature should disappear from normal enumeration where appropriate.

### 8.2 Direct Call Layer

If the caller still reaches a forbidden path by name, ID, or guessed call
shape, `Forge` must reject it explicitly.

Recommended failure mode
- explicit forbidden or unsupported result
- never silent downgrade

### 8.3 Session Layer

A valid session must not automatically gain access to product-forbidden
features.

------------------------------------------------------------------------

## 9. Probe Visibility Classes

Probe access should not be treated as one flat yes/no switch.

Strata should support visibility classes such as:
- common public
- backend public
- backend privileged
- internal only

Meaning
- common public
  - safe to expose across products through common inspection
- backend public
  - backend-specific but safe for products that intentionally expose it
- backend privileged
  - advanced inspection not appropriate for all products
- internal only
  - never exposed outside internal/testing builds

Product profiles must filter probes by class.

Important rule
- filtered probes must not appear in counts, lookup tables, or descriptors

------------------------------------------------------------------------

## 10. Native-State Exposure Rules

State exposure should be treated separately from backend execution.

A product may:
- run a backend natively,
- but expose only the common portable state model

This is especially important for richer-state backends.

General rule
- native execution does not imply native exposure

That lets Strata support products that:
- use a richer engine internally,
- while exposing only a reduced common observation/input surface.

------------------------------------------------------------------------

## 11. Recommended Product Profile Families

This document does not freeze final product names.

But Strata should be designed to support profile families such as:
- full platform
- fast-backend only
- temporal-backend only
- backend with common-surface-only exposure
- internal diagnostics-heavy build
- restricted public product build

These are model families, not final SKUs.

------------------------------------------------------------------------

## 12. What Must Be Impossible

The system must make these failure modes impossible or explicitly rejected:
- product-restricted features becoming available through a custom UI
- hidden probes still being discoverable by ID or name
- native-state APIs staying callable after being declared unavailable
- session-level configuration widening product restrictions
- build profiles silently diverging from capability discovery

------------------------------------------------------------------------

## 13. Relationship To Other Strata Docs

This model constrains:
- `STRATA_FORGE_RUNTIME_API_PLAN.md`
- `STRATA_BACKEND_CAPABILITY_REGISTRY.md`
- `STRATA_SHARED_PLAN_DRAFT.md`

This model does not replace them.

Instead:
- capability registry defines what a backend can do
- product profiles define what a product may expose
- `Forge` enforces the final visible contract

------------------------------------------------------------------------

## 14. Near-Term Design Consequences

Strata should add, or plan for, the following:
- build-profile definitions
- product-profile definitions
- session-policy narrowing
- capability reporting after gating
- explicit forbidden-result handling
- probe visibility metadata
- state-exposure policy metadata

These should be designed before the runtime surface expands too far.

------------------------------------------------------------------------

## 15. High-Level Conclusion

Strata should not solve product differentiation with forks or UI-only
restrictions.

It should solve it with:
- one shared codebase,
- one true backend set,
- one enforced runtime boundary,
- explicit product profiles,
- truthful capability discovery after gating.

That is the clean path that preserves architecture integrity while still
allowing sharply different products to ship from the same system.
