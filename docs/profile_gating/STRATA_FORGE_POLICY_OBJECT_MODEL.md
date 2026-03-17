# Strata Forge Policy Object Model

Purpose
- Define the logical policy objects that `Forge` should use for build,
  product, session, and effective enforcement state.
- Give implementation work a stable object model before code starts inventing
  incompatible ad hoc structs.

Status
- Planning and implementation-facing design document.
- This file defines object roles and logical fields.
- It does not finalize exact C headers or ownership files.

Core doctrine
- separate policy inputs
- compute one effective profile
- keep call sites simple

------------------------------------------------------------------------

## 1. Why This Model Exists

The gating docs already define:
- what build profiles mean,
- what product profiles mean,
- what session narrowing means,
- what `Forge` must enforce.

The missing piece is the object model for those ideas.

Without it, future implementation will drift toward:
- one-off booleans,
- inconsistent ownership,
- duplicated policy state,
- and hard-to-audit enforcement logic.

------------------------------------------------------------------------

## 2. Required Logical Objects

Strata should eventually define these logical objects:
- build capability set
- product exposure profile
- session restriction profile
- effective profile

These are different objects with different lifetimes.

------------------------------------------------------------------------

## 3. Build Capability Set

Purpose:
- represent what the shipped binary actually contains

Should answer:
- which backends are compiled in
- which extension families are compiled in
- which descriptor/probe families exist at all
- which advanced read/write/control paths exist at all

Expected lifetime:
- library/runtime lifetime

Expected mutability:
- immutable after process startup

------------------------------------------------------------------------

## 4. Product Exposure Profile

Purpose:
- represent what one shipped product is allowed to expose from the compiled set

Should answer:
- which backends are visible
- which artifact targets are accepted
- which common API families are exposed
- which native extension families are exposed
- which probe visibility classes are exposed
- whether native input/read paths are exposed
- whether advanced runtime controls are exposed

Expected lifetime:
- library/runtime configuration lifetime

Expected mutability:
- normally immutable once installed

------------------------------------------------------------------------

## 5. Session Restriction Profile

Purpose:
- represent safe narrowing for one specific session or session class

Should answer:
- whether the session is common-only
- whether probe visibility is narrowed further
- whether advanced controls are disabled for this session
- whether the chosen backend is narrower than the product baseline

Expected lifetime:
- one session or one session-class configuration

Expected mutability:
- preferably immutable once the session is created

------------------------------------------------------------------------

## 6. Effective Profile

Purpose:
- represent the final allowed runtime surface after combining build, product,
  and session policy

Should answer:
- everything the public API needs to know for filtering and validation

Expected lifetime:
- runtime-owned
- associated with the relevant library/context and session as needed

Expected mutability:
- immutable after computation for a given context

Important rule
- public runtime behavior should consult the effective profile
- not rebuild policy logic at each call site

------------------------------------------------------------------------

## 7. Recommended Logical Sections

Each profile object should be organized by section rather than one flat flag
list.

Recommended sections:
- backend availability
- artifact admission
- capability-family exposure
- descriptor visibility
- probe visibility
- state exposure
- input submission
- runtime control
- extension-family exposure
- diagnostics / privileged inspection

------------------------------------------------------------------------

## 8. Ownership And Installation

Recommended ownership pattern:
- build capability set owned at library/registry scope
- product exposure profile installed at runtime/library configuration scope
- session restriction profile provided at session creation scope
- effective profile computed by `Forge` and stored in runtime-owned state

Important rule
- callers should not directly mutate the computed effective profile

------------------------------------------------------------------------

## 9. Validation Rules

Policy objects should be validated when installed or computed.

Examples:
- product cannot expose a backend that was not compiled in
- product cannot expose an extension family that does not exist in the build
- session cannot enable a capability denied by the product
- effective profile cannot contain permissions not present in the build

------------------------------------------------------------------------

## 10. Future Implementation Guidance

The first implementation slice should focus on:
- defining internal logical structs
- computing a default unrestricted effective profile
- proving that filtering can hang off the effective profile cleanly

Later slices can add:
- product installation
- session narrowing
- forbidden-result behavior
- artifact admission gating

------------------------------------------------------------------------

## 11. High-Level Conclusion

Strata should treat policy as first-class runtime data, not scattered control
flow.

That means:
- explicit input objects,
- one computed effective profile,
- validated ownership,
- consistent use across `Forge`.
