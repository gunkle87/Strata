# Strata Repo Bootstrap Plan

Purpose
- Define the first safe construction pass for the new Strata repo.
- Prevent LXS-era mixed ownership from being copied forward without review.
- Establish the repo structure, migration rules, and first buildable milestone.

Status
- bootstrap baseline
- no backend code migrated yet

## 1. Bootstrap Goal

Create a clean, architecture-aligned starting point for Strata with:
- the correct directory structure,
- the right planning documents,
- the right governance protocols,
- only clearly reusable documentation copied in,
- no accidental code inheritance yet.

## 2. Fixed Naming Baseline

Naming rules
- `PascalCase`
  - types
  - major concepts
  - plan/object names
- `snake_case`
  - functions
  - variables
  - file names
- `UPPER_SNAKE_CASE`
  - constants
  - macros
  - compile-time flags

System names
- `Strata`
  - full layered platform
- `Breadboard`
  - structural compiler layer
- `Forge`
  - shared runtime API layer
- `LXS`
  - fast backend
- `HighZ`
  - temporal backend

## 3. Initial Repo Structure

Top-level directories
- `docs/`
- `protocol/`
- `src/authoring/`
- `src/breadboard/`
- `src/forge/`
- `src/backends/lxs/`
- `src/backends/highz/`
- `src/common/`
- `include/`
- `tools/`
- `tests/`

## 4. Migrated In Bootstrap

These items are safe to bring over immediately.

From LXS docs
- `docs/architecture/COMBINED_ENGINE_ARCHITECTURE_PLAN.md`
- `docs/architecture/COMBINED_ENGINE_FOUNDATION_NOTES.md`
- `docs/architecture/ENGINE_LAYER_HIERARCHY_MAP.md`
- `docs/architecture/ENGINE_LAYER_HIERARCHY_BOTTOM_UP.md`
- `docs/architecture/ITERATION_7_CROSS_ENGINE_STRENGTH_MATRIX.md`
- `docs/reference/PROBE_PROTOCOL.md`
- `docs/reference/TAB_PROTOCOL.md`

From LXS protocol bundle
- `protocol/engine_evolution/*`

Reason
- these govern architecture, rigor, and planning
- they are not engine-specific implementation baggage

## 5. Explicit Non-Migrations For Bootstrap

Do not copy these into Strata during bootstrap without redesign:
- `lxs_compiler.c`
- `lxs_engine.c`
- `lxs_api.c`
- any HighZ runtime source
- any HEBS runtime source
- UI code
- benchmark runner code
- recognition implementation code
- macro implementation code

Reason
- bootstrap is for architecture and repo purity first
- implementation comes after boundaries are fixed

## 6. Layer Ownership Baseline

### Authoring
- user-facing structure
- edits
- hierarchy
- ports
- connections

### Breadboard
- import
- normalization
- validation
- recognition
- macro substitution
- backend-targeted lowering

### Forge
- shared runtime lifecycle
- common reads
- common probes
- capability discovery
- backend dispatch boundary

### Backend-specific execution
- native state model
- native scheduling
- execution kernels
- state commit behavior
- backend-specific diagnostics

### Tooling
- format conversion
- corpus prep
- benchmark orchestration
- reporting
- governance support

## 7. First Buildable Milestone

The first buildable milestone should not be a full simulator.

It should be:
- a minimal Forge API surface draft,
- a backend capability model,
- a stub backend registration path,
- a no-op or smoke-path runtime session,
- tests proving the lifecycle shape exists.

Reason
- this locks the boundary early without prematurely locking the shared plan
  contract.

## 8. Required Next Documents

In order:
1. `docs/boundaries/STRATA_FORGE_RUNTIME_API_PLAN.md`
2. `docs/reference/STRATA_STATE_PROJECTION_POLICY.md`
3. `docs/boundaries/STRATA_BREADBOARD_BOUNDARY_SPEC.md`
4. `docs/reference/STRATA_BACKEND_CAPABILITY_REGISTRY.md`
5. `docs/boundaries/STRATA_SHARED_PLAN_DRAFT.md`

Important sequencing rule
- the shared runtime API comes before the shared plan contract
- the shared plan draft must derive from:
  - Forge lifecycle requirements
  - Breadboard compiler outputs
  - backend capability constraints

## 9. Commit Discipline

Bootstrap commits should stay narrow.

Recommended order:
1. repo structure and bootstrap docs
2. protocol migration
3. Forge API plan
4. capability registry
5. Breadboard boundary spec
6. initial header skeletons

## 10. Success Condition For Bootstrap

Bootstrap is complete when:
- the repo tree is in place,
- the core planning docs are present,
- the portable protocol bundle is present,
- the naming baseline is fixed,
- the next design documents are known,
- no backend code has been copied prematurely.

