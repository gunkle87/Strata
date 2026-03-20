# Strata Documentation Index

Purpose
- Keep the `docs/` tree easier to navigate and harder to misuse.
- Separate architectural truth from process material, implementation notes, and
  specialty references.

Use this file first when you need to know where a document belongs.

## Category Layout

### `docs/architecture/`
Use for top-level architecture intent, foundation reasoning, and system-shape
documents.

Current examples:
- `docs/architecture/COMBINED_ENGINE_ARCHITECTURE_PLAN.md`
- `docs/architecture/COMBINED_ENGINE_FOUNDATION_NOTES.md`
- `docs/architecture/ENGINE_LAYER_HIERARCHY_MAP.md`
- `docs/architecture/ENGINE_LAYER_HIERARCHY_BOTTOM_UP.md`
- `docs/architecture/ITERATION_7_CROSS_ENGINE_STRENGTH_MATRIX.md`

### `docs/boundaries/`
Use for active boundary contracts between major layers.

Current examples:
- `docs/boundaries/STRATA_BREADBOARD_BOUNDARY_SPEC.md`
- `docs/boundaries/STRATA_FORGE_RUNTIME_API_PLAN.md`
- `docs/boundaries/STRATA_SHARED_PLAN_DRAFT.md`

### `docs/reference/`
Use for specialty reference material that supports the architecture but is not
itself the main planning spine.

Current examples:
- `docs/reference/STRATA_BACKEND_CAPABILITY_REGISTRY.md`
- `docs/reference/STRATA_STATE_PROJECTION_POLICY.md`
- `docs/reference/PROBE_PROTOCOL.md`
- `docs/reference/TAB_PROTOCOL.md`
- `docs/reference/PROFILE_BOUNDARY_PLAN_CANONICAL_INDEX.md`

### `docs/notes/`
Use for implementation-status notes and phase-close explanatory notes.

Current examples:
- `docs/notes/BREADBOARD_SKELETON_NOTES.md`
- `docs/notes/FORGE_SKELETON_NOTES.md`

### `docs/process/`
Use for build/process/support plans that are helpful but should not dominate the
main architecture surface.

Current examples:
- `docs/process/BUILD_AND_TEST.md`
- `docs/process/STRATA_FORGE_TEST_AND_ADMISSION_PLAN.md`
- `docs/process/STRATA_MULTI_AGENT_TRIAL_PROTOCOL.md`
- `docs/process/STRATA_REPO_BOOTSTRAP_PLAN.md`

### `docs/profile_gating/`
Use for product-profile, gating, and enforcement policy documents.

### `docs/tools/`
Use for tool-specific documentation.

## Planning Versus Documentation

Use `planning/` for:
- the master pillar roadmap
- the active next pillar plan
- per-pillar trackers
- directive templates

Use `docs/` for:
- architecture truth
- boundary contracts
- reference material
- implementation notes
- process/support documents

Important rule
- planning files drive execution order
- docs files support understanding and architectural alignment

## Recommended Starting Points

If you are orienting to the project:
1. `docs/architecture/COMBINED_ENGINE_ARCHITECTURE_PLAN.md`
2. `docs/architecture/COMBINED_ENGINE_FOUNDATION_NOTES.md`
3. `planning/pillar_master_plan.md`

If you are working on runtime/compiler boundaries:
1. `docs/boundaries/STRATA_FORGE_RUNTIME_API_PLAN.md`
2. `docs/boundaries/STRATA_BREADBOARD_BOUNDARY_SPEC.md`
3. `docs/boundaries/STRATA_SHARED_PLAN_DRAFT.md`

If you are checking specialty support policy:
1. `docs/reference/STRATA_BACKEND_CAPABILITY_REGISTRY.md`
2. `docs/reference/STRATA_STATE_PROJECTION_POLICY.md`
3. `docs/profile_gating/PROFILE_GATING_DOC_INDEX.md`
