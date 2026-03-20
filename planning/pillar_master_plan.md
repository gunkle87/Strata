# Strata Pillar Master Plan

Purpose
- Define the full layered build plan for Strata using a pillar-based execution
  model.
- Keep one stable top-level map of completed, active, and future pillars.
- Allow only the next pillar in line to be refined in detail.

Status
- Active planning control document.
- Pillar 1 is complete.
- Pillar 2 is complete.
- Pillar 3 is complete.
- Pillar 4 is the next pillar to refine and execute.

## Build System Rules

1. No task may begin unless all previous tasks are complete.
2. Every task must be completed as one implementation slice.
3. Every completed task must be audited.
4. A task may be committed only after the audit passes with:
   - blockers = `0`
   - non-blockers = `0`
5. Tracker files are the gatekeepers for implementation order.

## Effort Model

Planning-time sizing:
- pillar effort score = estimated implementation-only effort for the whole
  pillar
- target implementation-only task difficulty = `3`
- planned task count = pillar score divided by `3`, adjusted to a practical
  whole number

Post-task rating:
- `TOO_EASY`
- `EASY`
- `PERFECT`
- `HARD`
- `TOO_HARD`

Important rule:
- effort rating applies only to implementation difficulty
- audit and commit work do not affect the task difficulty score

Historical benchmark:
- Pillar 1 was originally sized to score `33`
- Pillar 1 was executed as `11` tasks
- the previous 11 implementation commits now serve as the first practical
  workload benchmark for future task sizing

## Pillar Status Map

### Pillar 1 - First Executable Fast Path
- Status: `COMPLETED`
- Effort score: `33`
- Planned task count: `11`
- Summary:
  - make one narrow `FAST_4STATE` path real from `Breadboard` through `Forge`
  - prove compile -> export -> load -> session -> input -> step -> output
  - close the phase honestly without overstating generality
- Detailed files:
  - `planning/pillar_1_plan.md`
  - `planning/pillar_1_tracker.md`

### Pillar 2 - Backend Capability And Extension Surface
- Status: `COMPLETED`
- Provisional effort score: `24`
- Provisional task count: `8`
- Summary:
  - define and implement machine-readable backend capability reporting
  - define shared API extension-family discovery
  - make capability-driven refusal and extension lookup part of the public
    boundary
- Detailed files:
  - `planning/pillar_2_plan.md`
  - `planning/pillar_2_tracker.md`

### Pillar 3 - State Projection Policy Implementation
- Status: `COMPLETED`
- Provisional effort score: `27`
- Provisional task count: `9`
- Summary:
  - implement explicit projection rules for targeting reduced-state backends
  - keep approximation in the compiler layer, not hidden in runtime behavior
  - surface projection truth through emitted metadata and Forge visibility
- Detailed files:
  - `planning/pillar_3_plan.md`
  - `planning/pillar_3_tracker.md`

### Pillar 4 - Shared Artifact Evolution
- Status: `NEXT`
- Provisional effort score: `TBD`
- Summary:
  - evolve the temporary executable envelope into a cleaner long-term shared
    artifact contract

### Pillar 5 - Fast Backend Expansion
- Status: `FUTURE`
- Provisional effort score: `TBD`
- Summary:
  - broaden executable coverage beyond the first admitted subset
  - extend primitive, topology, and lowering coverage deliberately

### Pillar 6 - Temporal Backend Bring-Up
- Status: `FUTURE`
- Provisional effort score: `TBD`
- Summary:
  - bring the temporal backend into the shared runtime shape without collapsing
    native temporal truth

### Pillar 7 - Shared Runtime Read And Inspection Expansion
- Status: `FUTURE`
- Provisional effort score: `TBD`
- Summary:
  - expand common reads, probes, storage inspection, and metadata surfaces

### Pillar 8 - Structural Compiler Expansion
- Status: `FUTURE`
- Provisional effort score: `TBD`
- Summary:
  - expand `Breadboard` toward broader import, recognition, macro handling, and
    lowering coverage

### Pillar 9 - Authoring Layer Foundation
- Status: `FUTURE`
- Provisional effort score: `TBD`
- Summary:
  - define and build the editable structure layer above the compiler/runtime
    stack

### Pillar 10 - Tooling Layer
- Status: `FUTURE`
- Provisional effort score: `TBD`
- Summary:
  - organize import/export, corpus preparation, and reporting tools into a
    clean support layer

### Pillar 11 - UI Layer
- Status: `FUTURE`
- Provisional effort score: `TBD`
- Summary:
  - build the presentation layer on top of the authoring and runtime layers
    without mixing responsibilities

## Planning Rule For Future Pillars

- Only the next pillar in line should have a fully detailed plan and tracker.
- Later pillars stay named and summarized here until they become the next
  active pillar.
- This keeps the build structured without freezing stale implementation detail
  too early.
