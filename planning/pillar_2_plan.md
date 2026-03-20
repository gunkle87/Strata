# Pillar 2 Plan: Backend Capability And Extension Surface

This plan defines the next pillar after the first executable fast-path phase.

## Pillar Goal

Deliver a clean, machine-readable capability and extension surface so Strata
can report:
- what each backend supports
- what the common runtime API can rely on
- what requires explicit backend extensions

This pillar should make capability reporting a real public-boundary contract,
not an implied behavior discovered by failure.

## Effort Model

- Provisional pillar score: `24`
- Target implementation-only task difficulty: `3`
- Provisional task count: `8`

Sizing note:
- This pillar is intentionally sized using Pillar 1 as the workload benchmark.
- Task sizing may be refined before Task 2.1 begins if current repo state or
  scope discovery justifies an adjustment.

## Task Chunks

### Task 2.1 - Capability Data Contracts
Define core capability records, enums, flags, and result surfaces for backend
identity, lifecycle support, read support, and extension-family reporting.

### Task 2.2 - Backend Enumeration Surface
Implement public backend enumeration and identity query behavior through the
shared runtime boundary.

### Task 2.3 - Common Capability Query Surface
Implement machine-readable capability reporting for common lifecycle, common
reads, and common observation support.

### Task 2.4 - Extension Family Discovery Surface
Implement explicit extension-family reporting and lookup boundaries so
backend-specific strengths are discoverable without contaminating the common
API.

### Task 2.5 - Artifact Compatibility Capability Plumbing
Bind capability reporting to artifact target compatibility and load-time
refusal behavior.

### Task 2.6 - Capability-Driven Refusal Paths
Implement explicit refusal behavior when callers ask for unsupported common or
extension behaviors.

### Task 2.7 - Public Tests For Capability And Extension Reporting
Add public-boundary tests proving capability enumeration, extension discovery,
and capability-driven refusal paths.

### Task 2.8 - Pillar Integration And Validation
Integrate the capability and extension surface end to end, verify honesty of
the reported contract, and produce pillar-close evidence.
