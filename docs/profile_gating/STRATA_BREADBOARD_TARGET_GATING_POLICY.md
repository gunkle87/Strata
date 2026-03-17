# Strata Breadboard Target Gating Policy

Purpose
- Define how `Breadboard` should respect build and product target restrictions
  without becoming the final runtime enforcement boundary.
- Prevent compiler-side target selection from contradicting runtime policy.

Status
- Planning and implementation-facing design document.
- This file defines compiler-side target gating behavior.

Core doctrine
- `Breadboard` may respect policy
- `Forge` still owns final enforcement

------------------------------------------------------------------------

## 1. Why This Policy Exists

If `Breadboard` can target artifacts that the active product/runtime will never
accept, users get:
- confusing workflows,
- wasted compile time,
- and inconsistent expectations.

But if `Breadboard` becomes the only place that enforces target restrictions,
runtime policy becomes fragile.

This policy exists to keep those responsibilities clean.

------------------------------------------------------------------------

## 2. Breadboard Responsibilities

`Breadboard` may:
- expose only allowed targets to normal compile workflows
- reject target requests denied by build/product policy
- refuse artifact configurations that require denied extension families

`Breadboard` must not:
- become the sole enforcement boundary
- rely on UI-only hiding of denied targets
- assume runtime admission without `Forge` validation

------------------------------------------------------------------------

## 3. Policy Inputs

Compiler-side target gating should derive from:
- build capabilities
- product profile
- optional compile-session narrowing if Strata later supports it

------------------------------------------------------------------------

## 4. Required Behavior

`Breadboard` should:
- expose only legal targets for the current product/runtime context
- reject explicitly denied target requests
- avoid emitting artifacts that require denied extension families where
  possible

But `Forge` must still:
- validate artifact admission independently

------------------------------------------------------------------------

## 5. Discovery And UX Consequence

Normal target selection surfaces should only show:
- targets that are both build-available and product-allowed

This keeps authoring and compile tooling aligned with shipping reality.

------------------------------------------------------------------------

## 6. Result Behavior

Compiler-side target denial should be explicit in diagnostics.

Recommended meaning:
- target denied by policy
- target unavailable in build
- target incompatible with requested compile options

These should not all collapse into a vague unsupported error.

------------------------------------------------------------------------

## 7. Relationship To Runtime

Even when `Breadboard` rejects correctly, `Forge` must still perform final
admission checks.

Reason:
- artifacts may come from outside current tooling
- runtime policy cannot trust compiler-side discipline alone

------------------------------------------------------------------------

## 8. Test Requirements

Future tests should verify:
- denied target absent from compile target discovery
- denied target request fails explicitly
- allowed target request still succeeds
- runtime still rejects denied artifacts independently

------------------------------------------------------------------------

## 9. High-Level Conclusion

`Breadboard` should respect target policy early for better workflow and fewer
surprises, but `Forge` must remain the final enforcement boundary.
