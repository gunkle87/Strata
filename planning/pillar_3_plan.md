# Pillar 3 Plan: State Projection Policy Implementation

This plan defines the next pillar after the backend capability and extension
surface.

## Pillar Goal

Implement explicit backend-targeted state projection so Strata can:
- preserve native backend execution truth
- project unsupported state distinctions during structural compilation
- expose projection and approximation honestly through public-facing metadata
  and diagnostics

This pillar should move projection out of implied runtime behavior and make it
an explicit compiler-owned policy that can be tested and reported.

## Effort Model

- Provisional pillar score: `27`
- Target implementation-only task difficulty: `3`
- Provisional task count: `9`

Sizing note:
- This pillar is intentionally sized using Pillar 2 as the latest practical
  workload benchmark.
- Projection policy crosses `Breadboard`, artifact metadata, and `Forge`
  visibility, so the work is split into more medium slices instead of fewer
  heavy slices.

## Task Chunks

### Task 3.1 - Projection Policy Data Contracts
Define the core enums, records, and reporting surfaces for projection families,
approximation categories, and target-specific projection outcomes.

### Task 3.2 - Targeted Projection Compile Controls
Implement explicit `Breadboard` compile controls and target-selection plumbing
for projection-aware lowering.

### Task 3.3 - Backend-Targeted State Legality Checks
Implement compiler-side legality checks that detect unsupported state
distinctions before runtime for reduced-state targets.

### Task 3.4 - `UNINIT` Projection Handling
Implement explicit lowering rules for initialization-state handling so
`UNINIT`-style semantics do not leak into ordinary steady-state fast-backend
execution.

### Task 3.5 - Strength-State Projection Handling
Implement explicit lowering rules for richer strength distinctions such as
`WEAK_0` and `WEAK_1` when targeting reduced-state backends.

### Task 3.6 - Projection Metadata Plumbing
Bind projection outcomes into emitted draft or artifact metadata so semantic
approximation is visible rather than hidden.

### Task 3.7 - Forge Projection Visibility Surface
Implement the `Forge`-side visibility needed to surface projection metadata and
portable observation truth without redefining backend-native semantics.

### Task 3.8 - Public Tests For Projection And Approximation Reporting
Add public-boundary tests proving legality checks, projection behavior, and
projection metadata visibility.

### Task 3.9 - Pillar Integration And Validation
Integrate the projection policy end to end, verify policy honesty across
`Breadboard` and `Forge`, and produce pillar-close evidence.
