# Strata Forge Profile Install And Selection Plan

Purpose
- Define how product profiles and session policies should be installed into
  `Forge`.
- Specify when policy selection happens and how sessions inherit or narrow it.

Status
- Planning and implementation-facing design document.
- This file defines lifecycle and ownership flow for policy installation.

Core doctrine
- install once
- narrow safely
- do not let sessions widen product exposure

------------------------------------------------------------------------

## 1. Why This Plan Exists

Even with a strong policy model, Strata still needs a clear answer to:
- where product policy enters the runtime,
- when session policy is applied,
- how default behavior works,
- and how to avoid contradictory policy state.

This plan exists to answer those lifecycle questions before code lands.

------------------------------------------------------------------------

## 2. Recommended Policy Installation Stages

Policy should enter `Forge` in three stages:
- build capabilities available automatically
- product profile installed explicitly
- session restriction supplied at session creation

------------------------------------------------------------------------

## 3. Build Capabilities

Build capabilities should be treated as inherent runtime facts.

They should be:
- known when the runtime/library initializes
- available to backend and capability discovery
- immutable for the lifetime of the process

------------------------------------------------------------------------

## 4. Product Profile Installation

Product profile installation should happen before ordinary client use of the
runtime boundary.

Recommended behavior:
- one product profile installed into the library/runtime context
- all subsequent discovery and session creation derive from it

Important rule
- the product profile should not be an optional afterthought if the product
  intends clipping

------------------------------------------------------------------------

## 5. Default Behavior Before Product Installation

Strata should define an explicit default.

Recommended default for development:
- unrestricted profile bounded only by build capabilities

Recommended rule for shipping products:
- product profile installation should be mandatory in product-owned startup
  code

This preserves engineering convenience without turning shipping behavior into
accidental unrestricted runtime.

------------------------------------------------------------------------

## 6. Session Policy Selection

Session policy should be provided when creating a session, or selected from a
small approved set of session classes.

Examples:
- common-only session
- restricted-observation session
- diagnostics-disabled session

Important rule
- session policy narrows from the installed product profile
- it never widens it

------------------------------------------------------------------------

## 7. Effective Profile Creation

At session creation time, `Forge` should:
- start from build capabilities
- intersect with installed product profile
- narrow with requested session policy
- compute and store the effective profile

This should happen once, not on every call.

------------------------------------------------------------------------

## 8. Discovery Timing

Some discovery operates before session creation.

Therefore Strata should support:
- library/context-level discovery filtered by build + product profile
- session-level discovery filtered by build + product + session policy

This lets the UI/tooling layer know:
- what the product offers generally
- and what a specific session offers specifically

------------------------------------------------------------------------

## 9. Error Cases

Profile installation and session selection should fail explicitly when:
- a product profile requests a compiled-out backend or extension
- a session policy requests a capability denied by the product
- a session requests a backend not visible in the installed product profile

These are policy validation failures, not silent fallbacks.

------------------------------------------------------------------------

## 10. Future Implementation Order

Recommended order:
1. build capability discovery
2. installable product profile object
3. default unrestricted development profile
4. session restriction object
5. effective profile computation at session creation
6. discovery filtering at library and session scopes

------------------------------------------------------------------------

## 11. High-Level Conclusion

Strata should install policy intentionally and early.

That means:
- build capabilities define the maximum
- product profile defines the shipped surface
- session policy narrows it
- and every session runs under a computed effective profile

That is the cleanest route to predictable product clipping without forks.
