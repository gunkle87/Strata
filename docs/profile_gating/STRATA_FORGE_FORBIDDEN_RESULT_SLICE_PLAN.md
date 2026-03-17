# Strata Forge Forbidden Result Slice Plan

Purpose
- Define the implementation slice that adds an explicit forbidden/access-denied
  result to `Forge`.
- Prevent profile-enforcement code from overusing `FORGE_ERR_UNSUPPORTED`.

Status
- Planning and implementation-facing design document.
- This file defines the intended change set and sequencing for adding a
  dedicated forbidden result.

Core doctrine
- unsupported and forbidden are different truths
- the API should say which one happened

------------------------------------------------------------------------

## 1. Why This Slice Exists

The current `ForgeResult` surface is good enough for early skeleton work, but
not good enough for final profile enforcement.

The missing concept is:
- explicit policy denial

This slice exists to add that concept cleanly before profile gating spreads too
far through the code.

------------------------------------------------------------------------

## 2. Recommended Result

Recommended result name:
- `FORGE_ERR_FORBIDDEN`

Acceptable alternate:
- `FORGE_ERR_ACCESS_DENIED`

Recommended meaning:
- the runtime could support the request in principle,
- but the effective profile intentionally forbids it.

------------------------------------------------------------------------

## 3. Where The New Result Should Be Used

Examples:
- denied native-state reads
- denied native input submission
- denied backend-native extension entrypoints
- denied privileged probe reads
- valid but product-denied artifact classes
- denied advanced runtime controls

------------------------------------------------------------------------

## 4. Where It Should Not Be Used

Do not use `FORGE_ERR_FORBIDDEN` for:
- invalid arguments
- invalid handles
- invalid lifecycle order
- truly unsupported backend/build features
- incompatible artifacts
- normal filtered public lookup absence

------------------------------------------------------------------------

## 5. Recommended Change Set

This slice should update:
- `include/forge_result.h`
- relevant `Forge` public API comments
- profile-enforcement paths in `src/forge`
- tests for denied paths
- policy docs if wording needs tightening

------------------------------------------------------------------------

## 6. Required Test Cases

When this slice lands, tests should verify:
- denied native read returns forbidden
- denied native input returns forbidden
- denied privileged probe read returns forbidden
- denied advanced control returns forbidden
- valid but product-denied artifact returns forbidden
- genuinely unsupported feature still returns unsupported

------------------------------------------------------------------------

## 7. Migration Rule

While the codebase still lacks `FORGE_ERR_FORBIDDEN`:
- temporary denial paths may continue using the documented stand-in
- but new enforcement code should be written so the swap to forbidden is local
  and easy

------------------------------------------------------------------------

## 8. High-Level Conclusion

Strata should add a real forbidden result before profile enforcement is treated
as complete.

That one change will make:
- tests clearer,
- logs clearer,
- policy audits stronger,
- and product clipping much easier to reason about.
