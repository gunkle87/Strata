# Strata Forge Probe Visibility Model

Purpose
- Define how `Forge` should classify, filter, and expose probes under product
  and session policy.
- Prevent probe access from becoming an accidental leak of backend-native power.
- Give probe discovery and probe reads one stable visibility model.

Status
- Planning and implementation-facing design document.
- This file defines probe visibility classes and enforcement rules.
- It does not define the final probe descriptor ABI.

Core doctrine
- probes are part of the runtime contract
- probe visibility is policy-driven
- hidden probes must stay hidden in every lookup path

------------------------------------------------------------------------

## 1. Why Probe Visibility Needs Its Own Model

Probes are not just another descriptor list.

They can expose:
- internal signal state,
- backend-native intermediate truth,
- timing or phase information,
- privileged diagnostics,
- details a product may not intend to ship.

Without a dedicated visibility model, probe support will drift toward:
- one flat yes/no switch,
- hidden probes still reachable by ID or name,
- probe counts that leak more than they should,
- UI-level filtering mistaken for real enforcement.

This model exists to stop that.

------------------------------------------------------------------------

## 2. Scope

This model applies to:
- probe descriptor enumeration
- probe descriptor lookup by index
- probe descriptor lookup by ID
- probe descriptor lookup by name
- probe value reads
- grouped/snapshot probe reads later
- capability discovery for probe families

This model does not apply to:
- ordinary input/output descriptors
- backend-private non-probe data structures
- UI presentation choices

------------------------------------------------------------------------

## 3. Core Rules

1. Probe visibility must be class-based, not one flat switch.
2. Hidden probes must not appear in counts or normal discovery.
3. Hidden probes must not resolve through ID or name lookup.
4. Probe value reads must enforce the same visibility policy as discovery.
5. Probe visibility may be narrowed by session policy.
6. Probe filtering must happen in `Forge`, not just in UI code.
7. Probe visibility does not change backend truth, only public exposure.

------------------------------------------------------------------------

## 4. Recommended Probe Visibility Classes

Strata should classify probes into these visibility classes.

### 4.1 Common Public

Meaning:
- safe for cross-product public exposure
- portable enough to expose through common runtime inspection

Examples:
- explicitly admitted public observation points
- probe surfaces intended for ordinary user tooling

### 4.2 Backend Public

Meaning:
- backend-specific probes that are still safe for products intentionally
  exposing that backend surface

Examples:
- backend-specific but non-privileged observation points
- backend-specific diagnostics meant for advanced public products

### 4.3 Backend Privileged

Meaning:
- backend-specific probes that expose advanced or sensitive internal detail

Examples:
- richer-state internals
- phase-sensitive observation
- privileged temporal or lane-level information

These should be denied for many products.

### 4.4 Internal Only

Meaning:
- never exposed outside internal/testing/development contexts

Examples:
- deep debug probes
- development-only verification points
- internal correctness instrumentation

------------------------------------------------------------------------

## 5. Probe Descriptor Metadata Requirements

To enforce visibility cleanly, each runtime-visible probe descriptor should
eventually carry enough metadata to answer:
- probe ID
- probe name
- probe width
- probe backend ownership
- probe visibility class
- probe value class if needed later
- probe grouping or snapshot family if applicable

Important rule
- visibility metadata must live in runtime descriptor information
- not only in external docs or UI tables

------------------------------------------------------------------------

## 6. Visibility Filtering Rules

Probe filtering should happen before results are returned to the caller.

Required behavior:
- count queries include only visible probes
- enumeration returns only visible probes
- index lookup operates only over the visible filtered probe set
- ID lookup returns only visible probes
- name lookup returns only visible probes

Important rule
- a hidden probe must be invisible across every normal lookup mode

------------------------------------------------------------------------

## 7. Probe Read Rules

Probe value reads must enforce the same effective visibility policy as probe
descriptor discovery.

Required behavior:
- visible probe reads may proceed normally
- hidden probe reads must fail explicitly
- internal-only probes must never become readable in ordinary public products

Important rule
- it is not enough to hide a probe from enumeration if direct reads still work

------------------------------------------------------------------------

## 8. Session Narrowing Rules

Session policy may narrow probe visibility further than the product profile.

Examples:
- common-only session exposes only common public probes
- restricted session hides backend public probes that the product normally
  allows
- no-debug session hides privileged diagnostic probes

Important rule
- session policy may never expose a probe class the product profile denies

------------------------------------------------------------------------

## 9. Probe Families And Capability Discovery

Capability discovery should report probe support after visibility filtering.

That means:
- if a product denies backend privileged probes, they should not appear as
  available capability families
- if a product allows only common probes, probe capability reporting should
  reflect only that public surface

This is important because otherwise a caller can infer more than it should from
capability discovery even if actual descriptors are filtered later.

------------------------------------------------------------------------

## 10. Not Found Versus Forbidden Behavior

Probe APIs need a clean split between filtered absence and explicit denial.

Recommended rule:
- ordinary filtered discovery/lookups should make hidden probes appear absent
- privileged/native probe entrypoints should fail explicitly as forbidden once a
  dedicated result code exists

Until a distinct forbidden result exists, the temporary behavior may use the
documented stand-in from `STRATA_PROFILE_RESULT_CODE_POLICY.md`.

------------------------------------------------------------------------

## 11. Normal Public Lookup Behavior

For ordinary public probe APIs:
- index lookup should operate only on the visible filtered set
- out-of-range index returns `OUT_OF_BOUNDS`
- hidden ID/name should behave as absent, not as a disclosure of hidden
  existence

This reduces information leakage while keeping public APIs predictable.

------------------------------------------------------------------------

## 12. Privileged Probe Entry Behavior

If Strata later exposes backend-native or privileged probe entrypoints, those
should:
- validate handle
- validate lifecycle
- validate effective profile
- then return explicit forbidden if policy denies them

That gives a clean distinction between:
- public lookup surfaces that hide
- privileged surfaces that deny

------------------------------------------------------------------------

## 13. Artifact And Session Interaction

Probe visibility should not require changing backend execution truth.

That means:
- an artifact may contain probe metadata beyond what a product exposes
- the effective profile filters the visible probe set at load/session time
- session-specific narrowing may reduce it further

Important rule
- a hidden probe may still exist internally without becoming publicly
  discoverable

------------------------------------------------------------------------

## 14. Recommended Implementation Order

### Phase 1

- define logical probe visibility classes
- add visibility metadata to probe descriptors conceptually

### Phase 2

- filter probe counts
- filter probe enumeration
- filter probe lookup by ID and name

### Phase 3

- filter probe reads
- add explicit privileged/native probe denial behavior

### Phase 4

- add session-level narrowing tests
- add capability-discovery filtering for probe families

------------------------------------------------------------------------

## 15. Required Test Families

When this model lands in code, tests should verify:
- visible probe count excludes hidden probes
- hidden probe absent from enumeration
- hidden probe ID lookup behaves as absent on public surfaces
- hidden probe name lookup behaves as absent on public surfaces
- visible probe read succeeds where allowed
- denied privileged probe read fails explicitly
- session narrowing reduces visible probe set
- capability discovery reflects filtered probe families

------------------------------------------------------------------------

## 16. What This Model Avoids

This model deliberately avoids:
- a single yes/no probe switch
- UI-only probe filtering
- leaking hidden probes through alternate lookups
- treating privileged probes as ordinary public descriptors

------------------------------------------------------------------------

## 17. High-Level Conclusion

Strata should treat probes as a policy-sensitive runtime surface with their own
visibility model.

That model should:
- classify probes explicitly,
- filter them consistently across counts and lookups,
- enforce the same policy on reads,
- and allow session narrowing without widening product exposure.

That is the clean path to useful probe support without accidental capability
leaks.
