# Iteration 7 Cross-Engine Strength Matrix

Date: 2026-03-16  
Scope: Comparative architecture synthesis across `LXS`, `HighZ_I3`, `HighZ_I1`, `HighZ_I2`, `HEBS_I2`, and original `HEBS`  
Purpose: Convert the completed engine audits into a concrete inheritance and rejection matrix for iteration-7 planning.

## 1. Executive Summary

Iteration 7 should not inherit any prior engine wholesale.

The audit set shows a stable pattern:
- `LXS` is the strongest reference for:
  - fast backend execution shape
  - public runtime API
  - benchmark governance
  - protocol discipline
- `HighZ_I3` is the strongest reference for:
  - temporal runtime mechanics
  - dirty scheduling and delta execution
  - SoA-style hot-path storage thinking
- `HighZ_I2` is the strongest reference for:
  - compact drive-lane evaluation
  - low-branch primitive kernels
  - stronger parser direction than the earlier HighZ line
- `HighZ_I1` is the strongest reference for:
  - timing-wheel research value
  - early temporal-control intent
  - shadow verification discipline
- `HEBS_I2` is the strongest reference for:
  - clean loader/plan/runtime separation
  - breadboard-style layering instinct
- original `HEBS` is the strongest reference for:
  - primitive semantic completeness
  - useful batch and lowering ideas
  - algorithmic ideas that should be extracted, not inherited whole

The recommended iteration-7 architecture is:
- shared authoring layer
- shared structural compiler
- shared common runtime API
- backend-specific execution layers
  - fast 4-state backend
  - temporal richer-state backend

The main rejection rule is equally important:
- do not inherit historical drift, patchwork, or monolithic ownership just
  because a repo contains one strong subsystem.

## 2. Input Audit Set

Source audits used:
- [LXS_AUDIT.md](/c:/DEV/LXS/LXS_AUDIT.md)
- [HighZ_AUDIT.md](/c:/DEV/LXS/HighZ_AUDIT.md)
- [HighZ_I1_AUDIT.md](/c:/DEV/LXS/HighZ_I1_AUDIT.md)
- [HighZ_I2_AUDIT.md](/c:/DEV/LXS/HighZ_I2_AUDIT.md)
- [HEBS_AUDIT.md](/c:/DEV/LXS/HEBS_AUDIT.md)
- [HEBS_I1_AUDIT.md](/c:/DEV/LXS/HEBS_I1_AUDIT.md)

Planning context documents used:
- [COMBINED_ENGINE_FOUNDATION_NOTES.md](/c:/DEV/LXS/docs/architecture/COMBINED_ENGINE_FOUNDATION_NOTES.md)
- [COMBINED_ENGINE_ARCHITECTURE_PLAN.md](/c:/DEV/LXS/docs/architecture/COMBINED_ENGINE_ARCHITECTURE_PLAN.md)
- [ENGINE_LAYER_HIERARCHY_MAP.md](/c:/DEV/LXS/docs/architecture/ENGINE_LAYER_HIERARCHY_MAP.md)
- [ENGINE_LAYER_HIERARCHY_BOTTOM_UP.md](/c:/DEV/LXS/docs/architecture/ENGINE_LAYER_HIERARCHY_BOTTOM_UP.md)

## 3. Selection Criteria

An engine or subsystem was selected as strongest in a category only if it met
some combination of:
- architectural coherence
- technical usefulness
- evidence of working implementation
- portability into the intended layered system
- low baggage relative to the value offered

Important rule
- "historically interesting" is not enough.
- iteration 7 should inherit only ideas that remain defensible inside the new
  layered architecture.

## 4. Cross-Engine Strength Matrix

| Category | Best known source repo | Carry forward | Caveat or baggage | Target layer |
| --- | --- | --- | --- | --- |
| State representation | Split: `LXS` and `HighZ_I2` / `HighZ_I3` | Keep 4-state purity for the fast backend and lane-plus-resolution semantics for the temporal backend | Do not force one native state truth across both backends | Backend-specific execution layer |
| Execution model | Split: `LXS` and `HighZ_I3` | Use compiled fast execution plus temporal delta execution as separate backend families | One-core compromise would weaken both | Backend-specific execution layer |
| Temporal / delta / propagation control | `HighZ_I3` | Use current/next work stacks, dirty-driven propagation, and explicit phase-based runtime | HighZ frontend and parser are not the model to copy | Backend-specific execution layer |
| Timing wheel or event scheduling | `HighZ_I1` | Salvage concepts and heuristics for study only | Direct inheritance is too complex and under-validated | Backend-specific execution research |
| Primitive evaluation strategy | Split: `HighZ_I2` and original `HEBS` | Combine `HighZ_I2` low-branch evaluation style with original `HEBS` semantic completeness as a reference | Do not inherit original `HEBS` monolithic runner | Backend-specific execution layer |
| Memory layout and hot-path storage model | `HighZ_I3` | Use aligned SoA-style hot storage and hot/cold split thinking | `HighZ_I3` frontend is weaker than its runtime core | Backend-specific execution layer |
| Compiled plan representation | Split: `HEBS_I2` and `LXS` | Use `HEBS_I2` as the cleaner plan boundary shape and `LXS` as evidence of richer function plan content | Do not freeze a universal plan format before the shared API and compiler boundary are defined | Structural compiler layer plus shared runtime API layer |
| Parser / frontend / structural compiler shape | `HEBS_I2` | Use loader -> plan -> runtime separation as the packaging baseline | `HEBS_I2` is cleaner than LXS here, but less rich in higher-order lowering | Structural compiler layer |
| Recognition placement and value | `LXS` | Keep recognition as a compiler-layer service with report-only and admission discipline | Do not keep recognition buried in one backend repo | Structural compiler layer |
| Macro system placement and value | `LXS` | Keep macro semantic definition above execution and macro kernels at the backend boundary | LXS currently concentrates too much macro ownership in one repo | Structural compiler layer plus backend-specific execution layer |
| Public runtime API design | `LXS` | Use LXS runtime API as the baseline for the shared common API | Add explicit capability discovery and backend extension surfaces | Shared runtime API layer |
| Diagnostics / probes / inspectability | Split: `LXS` and `HighZ_I3` | Use LXS common probes and allow temporal inspection extensions | Keep backend-specific diagnostics explicit, not hidden in the common API | Shared runtime API layer plus backend-specific execution layer |
| Benchmark harness and measurement discipline | `LXS` | Use the LXS governance model program-wide | HighZ_I2 contributes useful gold-file habits, but not the stronger system | Tooling layer |
| Documentation / governance protocol | `LXS` | Use the portable protocol bundle as a program asset | Do not let governance remain repo-private or chat-only | Tooling layer / program governance |
| Repo layering / packaging structure | `HEBS_I2` | Use HEBS_I2 as the packaging reference for loader-plan-engine-harness split | Do not copy original HEBS monolith or empty-core mismatch | Structural compiler layer plus backend-specific execution layer plus tooling layer |
| UI / tooling implications | `LXS` plus combined-engine planning docs | Build UI on authoring + compiler + shared runtime API, not directly on backend internals | Do not repeat feature-first UI layering mistakes | Authoring layer plus shared runtime API layer |
| Hot-edit / recompilation implications | `LXS` plus `HEBS_I2` | Recompile at the structural compiler boundary, then reload the backend plan through the runtime API | Do not mutate deep runtime internals for authoring edits | Structural compiler layer plus shared runtime API layer |
| Portability to a shared multi-backend architecture | Split: `HEBS_I2`, `LXS`, and combined-engine docs | Build one structure model, one compiler, one common API, and two backends | Any attempt to inherit an older repo whole will re-import drift | Whole layered system |

## 5. Category-by-Category Inheritance Decisions

### 5.1 State Representation

Decision
- inherit two native state models, not one unified compromise

Carry forward
- `LXS`
  - pure 4-state dual-rail execution for the fast backend
- `HighZ_I2` / `HighZ_I3`
  - drive-lane accumulation plus resolved physical state for the temporal backend

Reason
- forcing LXS to carry richer runtime truth would likely slow it
- collapsing HighZ-style richer semantics too early would erase its value

Belongs in
- backend-specific execution layer

### 5.2 Execution Model

Decision
- inherit two execution families

Carry forward
- `LXS`
  - compiled plan execution with explicit staged commit
- `HighZ_I3`
  - delta-phase scheduling and dirty-net driven runtime mechanics

Belongs in
- backend-specific execution layer

### 5.3 Temporal Control

Decision
- inherit temporal stepping concepts from HighZ, not from LXS

Carry forward
- `HighZ_I3`
  - current/next work stacks
  - dirty-driven propagation
  - explicit phase-based runtime

Potential secondary source
- `HighZ_I1`
  - timing-wheel ideas if later required

Belongs in
- temporal backend execution layer

### 5.4 Timing Wheel

Decision
- do not adopt `HighZ_I1` timing-wheel implementation wholesale

Carry forward
- concepts and heuristics only as future research input

Reject
- the I1 timing-wheel implementation as an iteration-7 baseline

Belongs in
- backend-specific execution research

### 5.5 Primitive Evaluation

Decision
- combine `HighZ_I2` kernel style with original `HEBS` semantic catalog thinking

Carry forward
- `HighZ_I2`
  - low-branch lane/LUT primitive evaluation style
- original `HEBS`
  - semantically complete primitive coverage as a reference model

Belongs in
- backend-specific execution layer

### 5.6 Memory Layout

Decision
- inherit SoA and aligned hot arrays from the modern HighZ line

Carry forward
- `HighZ_I3`
  - hot/cold split thinking
  - aligned contiguous arrays
- `LXS`
  - compiled-span-friendly storage assumptions

Belongs in
- backend-specific execution layer

### 5.7 Compiled Plan Representation

Decision
- split inheritance

Carry forward
- `HEBS_I2`
  - clean plan object boundary
  - clear load/init/tick/query shape
- `LXS`
  - richer admitted function families inside the plan

Important caution
- the API draft should not silently decide the shared plan format too early
- define common lifecycle, reads, probes, and capability discovery first
- let the plan contract follow from the runtime API and compiler boundary

Belongs in
- structural compiler layer plus shared runtime API layer

### 5.8 Structural Compiler Shape

Decision
- use `HEBS_I2` as the primary packaging reference
- use `LXS` as proof that recognition and macro substitution are worth having

Carry forward
- `HEBS_I2`
  - loader -> plan -> engine split
- `LXS`
  - report-only recognition discipline
  - macro and function admission evidence

Belongs in
- structural compiler layer

### 5.9 Recognition

Decision
- inherit LXS recognition value, not LXS recognition placement

Carry forward
- report-only before replacement
- family legality
- admission discipline
- explicit rejection and alternate tracking

Belongs in
- structural compiler layer

### 5.10 Macro System

Decision
- inherit LXS macro evidence and split it by ownership

Carry forward
- macro semantic definition above execution
- macro kernels at backend boundary

Belongs in
- structural compiler layer and backend-specific execution layer

### 5.11 Public Runtime API

Decision
- use LXS as the primary baseline

Carry forward
- opaque handles
- lifecycle-driven design
- state reads
- metadata reads
- probe reads
- error surface

Add for iteration 7
- backend capability reporting
- explicit extension discovery
- common lifecycle and read semantics before shared plan standardization

Belongs in
- shared runtime API layer

### 5.12 Diagnostics And Probes

Decision
- use LXS for common probe surface
- allow HighZ-derived temporal inspection as backend-specific extension

Carry forward
- `LXS`
  - standard counters
  - state readers
  - API-visible probe access
- `HighZ`
  - finer temporal inspection opportunities

Belongs in
- shared runtime API layer plus backend-specific execution extensions

### 5.13 Benchmark Governance

Decision
- adopt the LXS governance system program-wide

Carry forward
- phase plans
- checkpoint discipline
- admission ledgers
- baseline ledgers
- report-only before replacement
- explicit phase close and re-entry

Belongs in
- tooling and program governance layer

### 5.14 Repo Layering And Packaging

Decision
- use `HEBS_I2` as the strongest reference for clean packaging

Carry forward
- separate loader, runtime, and harness roles

Reject
- original `HEBS` mismatch between docs and code
- engine designs where the tools directory contains the actual engine

Belongs in
- whole repo/program structure

### 5.15 UI And Tooling Implications

Decision
- the UI must sit above structure and compiler boundaries, not directly on
  backend internals

Carry forward
- LXS runtime API as the basis for backend interaction
- combined-engine layering docs as the architecture doctrine

Belongs in
- authoring layer and shared runtime API layer

### 5.16 Hot-Edit And Recompilation

Decision
- handle edits at the structural compiler boundary

Carry forward
- `LXS`
  - executable plan as a real object
- `HEBS_I2`
  - plan-and-engine separation that makes reloads conceptually clean

Belongs in
- structural compiler layer plus shared runtime API layer

## 6. Explicit Rejection List

Do not inherit these as direct iteration-7 baselines:
- original `HEBS` monolithic runner as the primary engine shape
- original `HEBS` empty-core / docs-versus-code mismatch
- `HighZ_I1` timing wheel implementation
- `HighZ_I1` parser limitations and custom `.hz` path
- `HighZ_I2` code/docs state-model mismatch as a truth source
- `HighZ_I3` parser/front-end as the model for structural compiler design
- `LXS` current concentration of compiler, recognition, and macro ownership
  inside one engine repo
- any single repo’s whole architecture as a complete inheritance target

## 7. Combined Architecture Recommendations

Recommended layered architecture:

1. `authoring layer`
- structure model
- UI-facing edit system

2. `structural compiler layer`
- import and normalization
- validation
- recognition
- macro substitution
- backend-targeted plan emission

3. `shared runtime API layer`
- common lifecycle
- common state reads
- common probes
- metadata
- capability reporting

4. `backend-specific execution layers`
- fast 4-state compiled backend
- temporal richer-state backend

5. `tooling and governance layer`
- format tools
- benchmark harness
- protocol docs
- ledgers and checkpoint discipline

## 8. Risks If The Wrong Pieces Are Mixed

Main failure modes:
- forcing one native state model across both backends
- forcing one execution core to serve both speed and granular temporal control
- keeping recognition inside backend repos
- inheriting timing-wheel complexity before proving the need for it
- copying the original HEBS monolithic runner architecture
- inheriting parser limitations from the HighZ line
- letting the shared API absorb backend-specific behavior without capability
  separation
- deciding the universal plan contract before the runtime boundary is actually
  agreed

## 9. Recommended Iteration 7 Baseline Stack

### Fast backend
- execution identity from `LXS`

### Temporal backend
- runtime mechanics from `HighZ_I3`
- selected scheduling concepts studied from `HighZ_I1`
- selected primitive kernel ideas from `HighZ_I2`

### Shared structural compiler
- packaging and boundary shape from `HEBS_I2`
- recognition and macro value model from `LXS`

### Shared common runtime API
- primary contract from `LXS`
- plus explicit backend capability reporting

### Shared tooling and governance
- from `LXS`

### Semantic reference layer
- selected primitive and state semantics from original `HEBS`
  where they remain useful as truth references

## 10. Recommended Next Design Documents

1. `shared runtime API contract`
- common lifecycle
- common reads
- common probes
- capability discovery
- backend extension model
- plan-format agnostic boundary rules

2. `state projection policy`
- portable state view
- native backend truth
- translation-time projection rules

3. `structural compiler boundary spec`
- inputs
- outputs
- recognition placement
- macro ownership

4. `backend capability registry`
- what is common
- what is optional
- what is backend-specific

5. `plan contract draft`
- derive from the shared runtime API and structural compiler boundary
- do not freeze the shared executable plan format before the runtime boundary is
  agreed
- identify what must be common versus backend-specific

6. `extraction map from LXS`
- what remains backend-local
- what moves upward into the shared compiler or tooling layers

## 11. High-Level Conclusion

Iteration 7 should be built from selected strengths, not historical loyalty.

The audit set shows a clear synthesis:
- `LXS` should anchor the fast backend, shared runtime API, and governance
- `HighZ` should anchor the temporal backend and richer-state runtime mechanics
- `HEBS_I2` should anchor clean structural separation between loader, plan,
  runtime, and harness
- original `HEBS` should be mined as a semantic and primitive reference, not as
  the structural baseline

The winning architecture is:
- one structure model
- one structural compiler
- one shared common runtime API
- multiple backend-specific execution engines
- one tooling and governance system

That is the cleanest path to making iteration 7 the first intentionally unified
system rather than the next isolated engine experiment.

