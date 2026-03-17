# Strata Feature Gating Policy

Purpose
- Define how Strata capabilities are gated at build, product, and session
  levels.
- Prevent accidental overexposure of backend-native power through `Forge`.
- Turn product restrictions into enforceable runtime policy instead of UI
  convention.

Status
- Planning and policy document.
- This file defines gating categories and enforcement behavior.
- It does not finalize macro names, config-file syntax, or API struct layout.

Core doctrine
- compile only what should ship
- expose only what the product allows
- advertise only what the caller may actually use

------------------------------------------------------------------------

## 1. Why Gating Must Be Explicit

Strata is intended to support:
- multiple backends,
- multiple products,
- multiple exposure levels,
- and future backend-native extensions.

Without explicit gating policy, the repo will drift toward one of these bad
states:
- everything ships everywhere,
- UI restrictions are mistaken for real restrictions,
- capabilities exist but are hidden only cosmetically,
- product differentiation becomes fork pressure.

This policy exists to stop that.

------------------------------------------------------------------------

## 2. Gating Layers

Strata feature gating has three layers:
- build gating
- product gating
- session gating

Each layer has a different job.

### 2.1 Build Gating

Build gating determines what is physically present in the binary.

### 2.2 Product Gating

Product gating determines what the shipped product exposes from the compiled
set.

### 2.3 Session Gating

Session gating determines what is enabled for one specific runtime session
within the product's allowed surface.

------------------------------------------------------------------------

## 3. Core Rules

1. Build gating may remove features entirely.
2. Product gating may expose only a subset of compiled features.
3. Session gating may narrow product permissions only.
4. `Forge` must enforce all three layers.
5. Capability discovery must report post-gating reality.
6. Forbidden features must fail explicitly if called directly.
7. Backend semantics must not be rewritten just to satisfy product gating.
8. Gating metadata must be machine-readable.

------------------------------------------------------------------------

## 4. Gating Categories

Strata should gate by capability family, not by scattered ad hoc checks.

Recommended gating categories:
- backend availability
- artifact target acceptance
- common lifecycle surface
- input submission surface
- read surface
- probe visibility and probe read surface
- runtime control surface
- native extension families
- diagnostics and privileged inspection
- tooling/compiler target availability where relevant

------------------------------------------------------------------------

## 5. Backend Availability Gating

This category decides whether a backend is:
- not compiled,
- compiled but not exposed,
- compiled and exposed.

Required behavior:
- unavailable backends must not appear in normal backend enumeration
- artifact loading for unavailable targets must fail explicitly
- backend-targeted compile options must be rejected where product policy denies
  them

------------------------------------------------------------------------

## 6. Input Submission Gating

This category governs what values and submission paths callers may use.

Examples:
- common portable input values only
- batch input submission allowed or denied
- backend-native input value classes allowed or denied

Important rule
- denying native input classes must be enforced at `Forge`
- not merely omitted from the UI

If a product exposes only common portable inputs:
- backend-native input APIs must not be advertised
- direct calls to them must fail explicitly

------------------------------------------------------------------------

## 7. Read Surface Gating

This category governs what state may be observed through public APIs.

Examples:
- outputs only
- common normalized signal reads
- descriptor reads
- native richer-state reads denied
- storage inspection denied

Important rule
- a product may permit native backend execution while exposing only common
  normalized reads

That means:
- execution truth remains native
- public observation truth is policy-limited

------------------------------------------------------------------------

## 8. Probe Gating

Probe access must support class-based filtering.

Recommended probe visibility classes:
- common public
- backend public
- backend privileged
- internal only

Required enforcement:
- denied probe classes must disappear from enumeration
- denied probe classes must not be returned by lookup by ID or by name
- probe counts must reflect only visible probes
- direct probe reads against forbidden probes must fail explicitly

------------------------------------------------------------------------

## 9. Runtime Control Gating

This category governs what execution controls are exposed.

Examples:
- common single-step only
- multi-step allowed
- bounded run allowed
- temporal sub-step control denied
- advanced backend-native stepping denied

Important rule
- common controls and backend-native controls must be gated separately

This preserves a clean baseline API while keeping advanced controls available
only in products that intentionally expose them.

------------------------------------------------------------------------

## 10. Native Extension Gating

Backend-native strengths should be exposed through extension families.

Examples:
- native richer-state reads
- advanced temporal controls
- backend-specific probes
- privileged storage inspection

Required behavior:
- denied extension families must not be advertised
- direct extension entrypoints must fail explicitly if reached
- common API must remain truthful whether or not extensions are present

------------------------------------------------------------------------

## 11. Capability Discovery After Gating

Capability discovery is not a raw engine dump.

It must reflect:
- what was compiled,
- what the product allows,
- what the current session allows.

Therefore:
- enumeration results are filtered
- capability flags are filtered
- extension-family discovery is filtered
- descriptor/probe visibility is filtered

Important rule
- discovery must never describe features the caller is not allowed to use

------------------------------------------------------------------------

## 12. Forbidden Call Behavior

When a caller reaches a forbidden feature directly, the response must be
explicit.

Recommended behavior:
- return a distinct forbidden or unsupported result
- do not silently degrade to a weaker behavior
- do not pretend success while clipping behind the scenes

This applies to:
- denied backend selection
- denied native-state reads
- denied native input submission
- denied probe access
- denied advanced runtime controls

------------------------------------------------------------------------

## 13. Descriptor And Lookup Filtering

Gating must apply to lookup surfaces as well as enumeration.

If a feature is hidden:
- it must not appear in count queries
- it must not appear in descriptor-by-index queries
- it must not appear in descriptor-by-ID queries
- it must not appear in descriptor-by-name queries

This matters for:
- probes
- storage objects
- advanced runtime-visible entities
- backend-native descriptor families

------------------------------------------------------------------------

## 14. Artifact Acceptance Gating

Artifact loading must also respect gating.

Examples:
- artifact target denied by build profile
- artifact target denied by product profile
- artifact requires extension family denied to this product

Required behavior:
- `Forge` rejects the artifact cleanly before execution
- session creation must not become the first time the product restriction shows
  up

------------------------------------------------------------------------

## 15. Compiler/Target Gating

Where `Breadboard` exposes target selection or compile options, those surfaces
must also respect profile policy.

Examples:
- product may deny emitting artifacts for some backends
- product may deny artifacts that require privileged extension families

Important rule
- compile-time target availability and runtime artifact acceptance should not
  contradict each other

------------------------------------------------------------------------

## 16. Audit Requirements For Gated Features

Every gated feature should be auditable at three levels:
- discovery behavior
- direct-call behavior
- artifact/session behavior

For a gated capability, tests should eventually prove:
- it does not appear in discovery,
- it cannot be used through direct calls,
- it cannot be smuggled in through artifact loading or session setup.

------------------------------------------------------------------------

## 17. Recommended Future Implementation Pieces

Strata should eventually add:
- build-profile definitions
- product-profile data structures
- session-policy data structures
- capability filtering helpers
- probe visibility metadata
- extension-family admission checks
- explicit forbidden-result code if current result space lacks one

------------------------------------------------------------------------

## 18. What This Policy Deliberately Avoids

This policy does not:
- require multiple backend forks
- require rewriting backend semantics per product
- assume every product needs every extension
- rely on UI code as the enforcement boundary

------------------------------------------------------------------------

## 19. High-Level Conclusion

Strata should gate features through explicit policy enforced at `Forge`, not
through convention or patched-over UI restrictions.

The correct pattern is:
- build-level inclusion,
- product-level exposure,
- session-level narrowing,
- truthful capability discovery,
- explicit rejection of forbidden paths.

That gives Strata one architecture and one codebase while still supporting
sharply different products safely.
