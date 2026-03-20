# Strata Profile And Gating Docs

Purpose
- Keep Strata product-profile, feature-gating, and runtime enforcement docs in
  one dedicated place.
- Prevent the main `docs` root from becoming a mixed pile of architecture,
  runtime, tooling, and product-policy material.

Scope
- product profile model
- feature gating policy
- `Forge` profile enforcement
- effective profile model
- probe visibility model
- result-code policy related to gating
- future build/product/session profile docs

Current document set
- `STRATA_PRODUCT_PROFILE_MODEL.md`
- `STRATA_FEATURE_GATING_POLICY.md`
- `STRATA_FORGE_PROFILE_ENFORCEMENT_PLAN.md`
- `STRATA_PROFILE_RESULT_CODE_POLICY.md`
- `STRATA_FORGE_EFFECTIVE_PROFILE_MODEL.md`
- `STRATA_FORGE_PROBE_VISIBILITY_MODEL.md`
- `STRATA_FORGE_POLICY_OBJECT_MODEL.md`
- `STRATA_FORGE_PROFILE_INSTALL_AND_SELECTION_PLAN.md`
- `STRATA_FORGE_FORBIDDEN_RESULT_SLICE_PLAN.md`
- `STRATA_FORGE_ARTIFACT_PROFILE_ADMISSION_POLICY.md`
- `STRATA_BREADBOARD_TARGET_GATING_POLICY.md`
- `STRATA_PROFILE_ENFORCEMENT_TEST_MATRIX.md`

Placement rule
- new docs focused primarily on build profiles, product profiles, session
  policy, capability clipping, visibility filtering, or related result behavior
  should live in this directory by default.

Nearby but separate topics
- broad runtime API planning remains in `docs/boundaries/STRATA_FORGE_RUNTIME_API_PLAN.md`
- artifact planning remains in `docs/boundaries/STRATA_SHARED_PLAN_DRAFT.md`
- compiler/layer boundary planning remains in the main `docs` root unless it
  becomes specifically profile-gating related

