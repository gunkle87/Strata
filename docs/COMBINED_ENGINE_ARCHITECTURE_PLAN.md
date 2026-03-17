# Combined Engine Architecture Plan

Purpose
- Define the target architecture for a shared simulation system with:
  - one authoring model,
  - one structural compiler,
  - one shared common runtime API,
  - multiple execution backends.
- The immediate motivating backends are:
  - a fast 4-state backend like LXS,
  - a richer temporal backend like HighZ.

Core doctrine
- `structure is the authoring layer`
- `function is the execution layer`

That means:
- authored structure is not execution,
- recognition is not execution,
- the engine executes compiled function,
- backend differences should be isolated below the shared runtime boundary.

------------------------------------------------------------------------

## 1. System Goal

Build one simulation ecosystem that supports:
- shared UI and tooling,
- shared structure model,
- shared structural compilation path,
- backend-specific execution strengths,
- clean interoperability without forcing all engines into one semantic model.

The system should allow:
- high-speed execution where throughput is primary,
- granular propagation control where visibility and timing control are primary,
- a common user-facing and tooling-facing contract above execution.

------------------------------------------------------------------------

## 2. Architectural Stack

Bottom up:

1. `execution backend layer`
   - executes compiled function natively
   - owns runtime semantics

2. `shared runtime API layer`
   - exposes the stable common execution contract
   - sits directly on top of execution

3. `structural compiler layer`
   - converts structure into backend-targeted executable function
   - owns recognition and macro substitution

4. `authoring layer`
   - owns circuit structure
   - owns edit intent
   - owns design-time legality

5. `tooling layer`
   - import/export tools
   - corpus preparation
   - report generation

------------------------------------------------------------------------

## 3. Shared Pieces

The following should be shared across the whole system.

### 3.1 Authoring Model

Functions
- represent components, traces, nodes, ports, and hierarchy
- support editing, duplication, deletion, and connection rules
- preserve user-authored structure independent of backend choice

### 3.2 Structural Compiler

Functions
- import and normalize structure
- validate structure
- recognize higher-order motifs
- substitute admitted functions and macros
- lower structure into backend-targeted executable form

Important rule
- recognition belongs here, not inside the raw execution backend

### 3.3 Shared Common Runtime API

Functions
- load compiled plan
- create session
- reset
- apply inputs
- run, tick, or step at the common lifecycle level
- read outputs
- read state
- read probes
- query metadata
- report capabilities

Important rule
- this API sits directly above execution
- it is not the topmost system API

------------------------------------------------------------------------

## 4. Backend Families

### 4.1 Fast Backend Family

Representative target
- LXS-style backend

Primary goal
- maximize throughput

Native characteristics
- 4-state native execution model
- compiled spans, macros, and region-style execution
- coarse tick-oriented execution
- minimal overhead in hot loops

Strengths
- high GEPS
- strong compiled execution behavior
- good fit for bulk simulation and throughput-driven benchmarking

Limits
- reduced native temporal granularity
- reduced native state richness

### 4.2 Temporal Backend Family

Representative target
- HighZ-style backend

Primary goal
- maximize propagation visibility and control

Native characteristics
- 7-state or 8-state native execution model
- delta or phase-oriented stepping
- richer intermediate visibility
- stronger support for causal and temporal inspection

Strengths
- granular stepping
- richer electrical or logical state expression
- stronger debugging and propagation analysis value

Limits
- more runtime semantic complexity
- lower top-end throughput relative to the fast backend

------------------------------------------------------------------------

## 5. Shared API Contract Versus Backend Extensions

The system should use:
- one `shared common API`
- backend-specific `extension APIs`

### 5.1 Shared Common API

This should include only behavior that all backends can support coherently:
- load compiled plan
- create and free runtime session
- reset session
- apply inputs
- advance simulation at the common lifecycle level
- read outputs
- read portable state
- read common probes
- query plan and object metadata
- query backend capabilities

### 5.2 Backend Extension APIs

These expose strengths that are not universal.

Examples:

Temporal backend extensions
- step one propagation unit
- step one delta phase
- inspect sub-cycle or intermediate propagation state
- pause on event or transition boundaries

Fast backend extensions
- high-throughput run modes
- bulk tick many
- optimized profiling controls
- packed or region execution diagnostics where applicable

Rule
- extension APIs must be explicit
- backend-specific features must not be smuggled into the shared common API

------------------------------------------------------------------------

## 6. State Model Policy

This system must support different native execution state models without
forcing all backends into one runtime truth.

### 6.1 Native Truth Rule

Each backend should preserve its native execution semantics.

Examples
- LXS remains 4-state natively
- HighZ remains 7-state or 8-state natively

Reason
- forcing the fast backend to carry richer runtime state will likely reduce its
  speed and complicate its kernels
- forcing the temporal backend to collapse into 4-state loses meaningful
  observability and fidelity

### 6.2 Shared Observation Rule

The shared common API should expose a portable state view.

Recommended common-denominator view
- `0`
- `1`
- `X`
- `Z`

Backend extensions may expose richer native state where available.

### 6.3 Translation-Time Projection Rule

Unsupported state distinctions should be projected during structural compilation
when targeting a backend that cannot represent them natively.

Examples
- `UNINIT` should be resolved before steady-state execution begins
- `WEAK_1` and `WEAK_0` may be flattened when targeting a 4-state backend

Important caution
- this projection is not “free truth”
- it is an explicit semantic approximation policy for the target backend

### 6.4 Principle

- `normalize for interoperability, preserve native truth for execution`

------------------------------------------------------------------------

## 7. Unsupported State Handling

### 7.1 `UNINIT`

Policy
- `UNINIT` should not survive as a steady-state runtime value in a throughput
  backend like LXS
- it should be resolved during initialization, reset, or first-resolution setup

Reason
- `UNINIT` is primarily a pre-execution or initialization concern
- carrying it deep into a fast runtime weakens execution purity

### 7.2 `WEAK_1` and `WEAK_0`

Policy
- preserve natively in richer-state backends
- project explicitly when targeting 4-state backends

Possible projection outcomes
- flatten to strong `1` or `0`
- in some conflict cases map to `X` or `Z` depending on the target contract

Important rule
- projection semantics must be explicit and documented in the compiler layer
- they must not be hidden in ad hoc runtime code

------------------------------------------------------------------------

## 8. Recognition And Macro Policy

### 8.1 Recognition

Recognition should live in the structural compiler layer.

Functions
- detect structure
- evaluate legality
- perform report-only census
- perform replacement qualification
- emit better executable function

Recognition is:
- structural translation
- not raw execution

### 8.2 Macros

Macro systems should be split into:

1. `macro semantic definition`
- semantic contract
- ports
- legality
- replacement meaning
- belongs in the structural compiler and function catalog domain

2. `macro execution kernel`
- native executor implementation
- belongs in or beside the engine backend

This preserves portability while keeping execution fast.

------------------------------------------------------------------------

## 9. Plan Targeting Strategy

The structural compiler should target backends explicitly.

That means:
- the compiler knows which backend is being targeted
- the compiler applies backend-specific legality and projection rules
- the compiler emits the executable form the backend expects

This allows:
- backend-pure execution
- portable authoring
- portable recognition policy
- clear capability negotiation at load time

------------------------------------------------------------------------

## 10. Tooling Placement

Standalone tools should remain outside the raw engine backend.

Examples
- `blif2bench`
- import/export utilities
- corpus prep
- report processing

Reason
- tooling supports authoring and translation
- it is not part of execution identity

------------------------------------------------------------------------

## 11. Recommended Implementation Order

1. Cleanly separate one execution backend from everything above it
2. Formalize the shared common runtime API
3. Formalize backend capability reporting and extension surfaces
4. Formalize the structural compiler boundary above the runtime API
5. Move recognition and macro semantic policy into the structural compiler
6. Define state projection policy for backend targeting
7. Bring the second backend up to the shared API contract
8. Add shared UI and tooling on top of the structure and API layers

------------------------------------------------------------------------

## 12. Immediate Planning Consequences

This means the next architectural planning should focus on:
- shared runtime API contract
- backend capability model
- state projection policy
- structural compiler ownership boundaries
- explicit separation of execution kernels from recognition policy

This also means there is not yet a complete combined-engine plan in the repo.
This document is the first dedicated architecture plan for that system.
