# First Execution Plan

Purpose
- Define the implementation plan for the first real executable vertical path in
  Strata.
- Break the phase into 11 balanced tasks with each task fully specified before
  implementation begins.
- Keep the plan aligned with the current Strata doctrine:
  - `structure is the authoring layer`
  - `function is the execution layer`

Status
- Planning document.
- This plan defines the next major implementation phase after API scaffolding.
- It does not define the final Strata artifact format or final multi-backend
  execution model.

Core phase rule
- make one narrow executable path real before broadening the system

------------------------------------------------------------------------

## 1. Phase Goal

This phase exists to turn the current placeholder-only Breadboard -> Forge path
into the first real executable Strata path.

The first admitted target is:
- `FAST_4STATE`
- one narrow executable structural subset
- one deterministic common runtime advancement model

At the end of this phase, Strata should be able to:
- accept authored structure within the admitted subset,
- lower that structure into real executable fast-path content,
- load it through `Forge`,
- create a real session,
- apply inputs,
- advance execution,
- and read outputs through the public boundary.

------------------------------------------------------------------------

## 2. Phase Boundaries

### In scope

This phase includes:
- one admitted executable subset for `FAST_4STATE`
- first real Breadboard lowering for that subset
- first real executable payload semantics inside the current temporary envelope
- first real Forge session lifecycle for that path
- first real input, step, and output-read behavior through the public API
- public tests proving the executable path end to end

### Out of scope

This phase does not include:
- final artifact-format design
- broad compiler lowering coverage
- recognition-system expansion
- temporal-backend execution
- `HighZ` runtime bring-up
- full probe-system growth
- broad storage inspection
- multi-backend parity
- UI or authoring-model expansion beyond what this executable slice requires

------------------------------------------------------------------------

## 3. Execution Strategy

The implementation strategy for this phase should stay narrow and layered.

1. Pick one backend and one subset.
   - Do not split focus across multiple runtime targets.

2. Keep placeholder and real paths distinct.
   - The placeholder path should remain available for scaffolding and deferred
     targets.
   - The new fast path should be explicitly real, not ambiguous.

3. Add real execution beside the current contract.
   - Do not attempt to redesign the whole artifact model at the same time.

4. Tighten admission only after the path is proven.
   - First make the path real.
   - Then make the repo more strict about when placeholder fallback is allowed.

5. Preserve execution ownership boundaries at all times.
   - `Breadboard` owns legality and lowering.
   - `Forge` owns load, lifecycle, orchestration, and common reads.
   - the backend owns executable truth and primitive evaluation semantics.

6. Treat Tasks 2 through 4 as the bridge from placeholder scaffolding to the
   first real executable path.
   - These tasks exist specifically to create the first real executable content.
   - They must not assume that real executable support already exists below
     them.

------------------------------------------------------------------------

## 4. Planned Tasks

### Task 1. Lock the admitted executable subset

Purpose
- Define the exact authored structural subset that this phase will admit for
  real execution.

Required outcome
- The first primitive family, connection rules, and deterministic advancement
  assumptions are fixed explicitly.
- Supported and unsupported authored shapes are named clearly enough for
  Breadboard validation, lowering, Forge loading, and tests to share one truth.

Important constraint
- The subset must stay intentionally small.
- It should be large enough to prove the architecture, but small enough to keep
  the first executable path honest and auditable.

Task 1 completion definition
- The admitted subset is frozen as:
  - `FAST_4STATE` only
  - flat single-module structure only
  - single-bit signals only
  - combinational logic only
  - no stateful elements
  - no cycles or feedback paths
- The admitted primitive family is frozen as:
  - `BUF`
  - `NOT`
  - `AND`
  - `OR`
  - `XOR`
- Primitive signatures are frozen explicitly:
  - `BUF(in -> out)`
  - `NOT(in -> out)`
  - `AND(a, b -> out)`
  - `OR(a, b -> out)`
  - `XOR(a, b -> out)`
- The endpoint model is frozen explicitly:
  - module input source endpoints
  - primitive output source endpoints
  - primitive input sink endpoints
  - module output sink endpoints
- Connectivity rules are frozen explicitly:
  - every sink has at most one driver
  - every required primitive input must be driven
  - every module output must be driven
  - fan-out is allowed
  - cycles are rejected
  - self-loops are rejected
- Primitive execution semantics are frozen explicitly for portable four-state
  evaluation:
  - `BUF`
    - `0 -> 0`
    - `1 -> 1`
    - `X -> X`
    - `Z -> X`
  - `NOT`
    - `0 -> 1`
    - `1 -> 0`
    - `X -> X`
    - `Z -> X`
  - `AND`
    - `0 & anything -> 0`
    - `1 & 1 -> 1`
    - `1 & X -> X`
    - `1 & Z -> X`
    - `X & X -> X`
    - `X & Z -> X`
    - `Z & Z -> X`
  - `OR`
    - `1 | anything -> 1`
    - `0 | 0 -> 0`
    - `0 | X -> X`
    - `0 | Z -> X`
    - `X | X -> X`
    - `X | Z -> X`
    - `Z | Z -> X`
  - `XOR`
    - `0 ^ 0 -> 0`
    - `0 ^ 1 -> 1`
    - `1 ^ 0 -> 1`
    - `1 ^ 1 -> 0`
    - any case involving `X` or `Z` -> `X`
- Determinism rules are frozen explicitly:
  - deterministic lowering order
  - deterministic descriptor ordering
  - deterministic artifact emission for equivalent structure
  - deterministic runtime evaluation for identical applied inputs
- The canonical proof shapes are frozen explicitly:
  - input -> `BUF` -> output
  - input -> `NOT` -> output
  - two inputs -> binary gate -> output
  - small multi-node combinational chain
  - explicit rejection cases for cycles, unsupported primitives, bad widths,
    undriven sinks, and multiply driven sinks

### Task 2. Define the first real fast-path payload contract

Purpose
- Define the minimal executable payload semantics needed for the admitted fast
  path.

Required outcome
- The payload stops being only a placeholder marker.
- The payload carries enough executable meaning for runtime setup, stepping,
  and output production.
- The design remains compatible with the current temporary envelope approach.

Important constraint
- This does not finalize the long-term artifact format.
- It only makes one narrow payload family real.
- This task begins the bridge from placeholder scaffolding to real executable
  content.

Task 2 completion definition
- The payload contract is frozen as a real fast-path executable block carried
  inside the existing temporary artifact envelope.
- The payload is required to contain, at minimum:
  - executable primitive records
  - executable connection or signal-routing records
  - stable input binding records
  - stable output binding records
  - total executable object counts needed for session allocation
- The executable opcode domain is frozen explicitly to:
  - `BUF`
  - `NOT`
  - `AND`
  - `OR`
  - `XOR`
- The payload is frozen as target-specific for the fast backend only.
- The payload must be self-consistent with envelope-level descriptor and summary
  metadata.
- The payload must not depend on compiler-private in-memory structures at load
  time.
- The payload must support enough runtime setup to:
  - allocate session state
  - bind inputs
  - evaluate the admitted network
  - bind outputs for reads
- The following binding invariants are frozen explicitly:
  - every payload-declared module input binding must map to exactly one admitted
    input descriptor
  - every payload-declared module output binding must map to exactly one
    admitted output descriptor
  - every primitive input reference must point to a valid source record
  - every primitive output reference must point to a valid produced value slot
  - no descriptor required for runtime I/O may exist without a corresponding
    payload binding
- Explicit malformed-payload classes are frozen now:
  - invalid counts
  - invalid references
  - unsupported primitive opcode
  - out-of-bounds binding references
  - impossible topology for the admitted contract
- Real payload validation invariants are frozen explicitly:
  - executable opcode stream must contain only admitted opcodes
  - all binding references must resolve within payload bounds
  - payload counts must agree with header and section metadata
  - descriptor counts required by the payload must agree with descriptor blocks
  - target and payload kind must agree
- Placeholder payloads and real fast payloads must remain distinguishable by
  explicit contract fields, not by inference alone.

### Task 3. Add Breadboard legality checks for the admitted subset

Purpose
- Teach `Breadboard` to distinguish real supported executable structure from
  unsupported requests for the first fast path.

Required outcome
- Unsupported primitives, unsupported topology shapes, and invalid authored
  combinations are rejected explicitly.
- Diagnostics remain stable and honest.
- Requests outside the admitted executable subset do not silently succeed as if
  they were real lowering.

Important constraint
- Rejection behavior is part of the phase, not a secondary cleanup.
- This task continues the bridge from placeholder scaffolding to real
  executable admission.

Task 3 completion definition
- Breadboard legality for the real fast path is frozen to require:
  - admitted primitive kinds only
  - width `1` only
  - flat structure only
  - acyclic structure only
  - all required inputs driven
  - all module outputs driven
  - no stateful declarations in the real path
- The real path must reject explicitly:
  - unknown component kinds
  - widths other than `1`
  - cycles
  - self-loops
  - multiply driven sinks
  - missing output drivers
  - missing required primitive inputs
  - authored requirement profiles that imply temporal or native-only semantics
    beyond the admitted path
- The legality layer must distinguish between:
  - executable-admitted fast structure
  - placeholder-eligible but not executable structure
  - outright invalid structure
- The gating rule is frozen explicitly:
  - if the caller requests the real executable fast path and the structure is
    outside the admitted subset, the compile attempt must hard fail
  - placeholder fallback is only allowed when the caller is explicitly using a
    placeholder-oriented compile path
  - the real executable path must never silently degrade into placeholder
    success
- Diagnostic categories are frozen to cover at least:
  - unsupported executable subset request
  - invalid connectivity
  - invalid primitive kind
  - invalid width
  - invalid target/profile mismatch

### Task 4. Implement Breadboard lowering for the admitted subset

Purpose
- Add the first real lowering path from authored structure into executable
  fast-path content.

Required outcome
- `Breadboard` can produce a real executable draft for admitted fast-path
  structure.
- Lowering is deterministic.
- Equivalent authored structure yields equivalent executable output.

Important constraint
- This is still narrow lowering.
- It must not be overstated as general compiler completeness.
- This task completes the bridge from placeholder scaffolding to the first real
  executable fast-path content.

Task 4 completion definition
- Breadboard lowering is frozen to produce one executable representation for
  the admitted combinational subset only.
- The lowering path must transform authored structure into:
  - executable primitive records in deterministic order
  - explicit signal/binding records for inputs and outputs
  - runtime-usable connectivity references
- Lowering order must be stable for equivalent authored structure.
- Lowering must not depend on placeholder descriptor defaults for admitted real
  structure.
- Real executable drafts must be marked explicitly as real executable drafts,
  not just improved placeholders.
- Unsupported but syntactically valid authored structures must not quietly lower
  into malformed runtime content.
- The lowered result must be sufficient for later session allocation and one
  deterministic evaluation step.
- The real executable lowering path must produce output intended for backend
  execution ownership, not for Forge-side semantic execution.

### Task 5. Bind real descriptor truth to lowered execution output

Purpose
- Tie exported descriptors to real lowered execution truth instead of
  placeholder assumptions.

Required outcome
- Descriptor enumeration and lookup remain stable for executable drafts.
- Input, output, and runtime-visible identity remain coherent across compile,
  export, load, and read behavior.

Important constraint
- Descriptor truth must come from the executable path that was actually
  lowered, not from a synthetic side table disconnected from runtime meaning.

Task 5 completion definition
- Real executable input and output descriptors are frozen to correspond exactly
  to admitted module inputs and module outputs.
- Descriptor widths are frozen to `1` for this phase.
- Descriptor IDs and names must remain stable across:
  - Breadboard compile
  - artifact export
  - Forge load
  - runtime reads
- The descriptor contract must be sufficient for:
  - input application by stable identity or index
  - output reads by stable identity or index
  - deterministic descriptor enumeration
- Probe descriptors are explicitly excluded from the real executable subset in
  this phase, even if broader scaffolding remains in the repo.
- Descriptor queries must continue to fail explicitly on invalid arguments,
  out-of-bounds indices, and missing IDs or names.

### Task 6. Teach Forge to recognize and validate real fast artifacts

Purpose
- Extend `Forge` load behavior so it can admit real fast executable artifacts
  and reject malformed or incompatible ones.

Required outcome
- `Forge` can distinguish executable fast-path artifacts from placeholder
  artifacts.
- Payload coherence, descriptor coherence, and target compatibility are
  validated explicitly.
- Malformed and mismatched artifact paths fail cleanly.

Important constraint
- Load validation must still remain a public-boundary responsibility.
- `Forge` must not require compiler internals to understand artifact validity.
- `Forge` must not become the owner of primitive execution semantics.

Task 6 completion definition
- Forge load validation is frozen to require coherence between:
  - artifact header/version
  - section directory
  - summary/admission metadata
  - descriptor block
  - executable fast payload block
- Forge must explicitly distinguish:
  - placeholder artifact path
  - real fast executable artifact path
  - malformed artifact path
- Validation must reject explicitly:
  - malformed counts
  - malformed offsets
  - malformed payload references
  - payload/header disagreement
  - descriptor/payload disagreement
  - target mismatch
  - unsupported executable opcode or object class
- Successful load must store artifact-owned executable metadata sufficient for
  later session creation.
- Forge must not infer executable validity from one field while ignoring the
  rest of the contract.
- Forge load is frozen as validation and orchestration only:
  - it may decode and store executable metadata
  - it may prepare runtime-owned objects
  - it must not redefine primitive logic semantics that belong to backend
    execution ownership

### Task 7. Implement real session creation and lifetime for the fast path

Purpose
- Replace pure stub lifecycle behavior with a real runtime session for admitted
  fast executable artifacts.

Required outcome
- `Forge` can create, reset, and destroy a real session for the admitted fast
  path.
- Ownership rules remain explicit.
- Invalid lifecycle order still fails explicitly.

Important constraint
- Only the admitted fast path becomes real here.
- Unsupported runtime cases must continue to reject cleanly.
- Session ownership does not imply execution-semantic ownership.

Task 7 completion definition
- Session creation for the real fast path is frozen to allocate only the state
  needed for:
  - input storage
  - intermediate combinational evaluation storage if needed
  - output storage
- Session reset is frozen to restore a clean deterministic pre-step state for
  the admitted subset.
- Session destroy/unload must free all allocations associated with the real fast
  path.
- Lifecycle behavior must remain explicit for:
  - create before use
  - no reads before successful create/load
  - reset only on valid sessions
  - no use-after-destroy
- Unsupported runtime features must still reject explicitly instead of behaving
  as partial stubs.
- Backend execution ownership is frozen explicitly:
  - session objects may hold backend runtime state
  - Forge creates and manages lifecycle around that state
  - primitive evaluation semantics still belong to backend execution logic

### Task 8. Implement input application and deterministic advancement

Purpose
- Add the first real execution control path through the shared runtime API.

Required outcome
- Inputs can be applied through the public `Forge` boundary.
- One deterministic advancement operation exists for the admitted fast path.
- Advancement semantics are stable enough for repeatable tests.

Important constraint
- The advancement model should stay simple and phase-appropriate.
- This task is not the place to introduce temporal control complexity.
- Forge orchestration must remain distinct from backend execution truth.

Task 8 completion definition
- Input application is frozen to support stable binding through the public API
  for admitted input descriptors.
- One common advancement operation is frozen as:
  - evaluate the admitted combinational network from current applied inputs
  - produce committed output values for subsequent reads
- The advancement model explicitly excludes:
  - temporal phases
  - event queues
  - delta stepping
  - persistent sequential state behavior
- Input application must reject explicitly:
  - invalid handles
  - invalid input IDs or indices
  - width mismatch relative to the admitted subset
  - unsupported write paths
- Advancement must be deterministic for identical session state and identical
  applied inputs.
- Advancement ownership is frozen explicitly:
  - Forge invokes the common advancement boundary
  - the backend performs the actual primitive-network evaluation according to
    the frozen semantics for this phase
  - Forge does not independently reinterpret the network during advance

### Task 9. Implement real common reads for outputs and minimal observation

Purpose
- Add the first real common read surface for the admitted executable subset.

Required outcome
- Callers can read outputs after real execution steps.
- The minimum required portable observation surface exists for this subset.
- Read behavior remains descriptor-driven and does not leak backend-private
  runtime layout.

Important constraint
- Only the minimum common observation needed for the admitted subset should be
  implemented here.
- Broader read families belong to later phases.

Task 9 completion definition
- Output reads are frozen as public-boundary reads over admitted output
  descriptors only.
- The common observation surface for this phase is frozen to portable four-state
  values:
  - `0`
  - `1`
  - `X`
  - `Z`
- Reads must be stable after the committed advancement boundary defined in Task
  8.
- Read behavior must reject explicitly:
  - invalid handles
  - invalid descriptors
  - invalid output indices
  - unsupported observation paths
- The first real path does not include broader native-state reads, storage
  inspection, or temporal intermediate observation.
- Read-path gating is frozen explicitly:
  - executable output reads are available only for successfully loaded real fast
    artifacts with valid live sessions
  - placeholder artifact paths must not impersonate executable read success for
    this phase

### Task 10. Prove the real vertical path through public tests

Purpose
- Validate the first executable path through Breadboard, Forge, and
  integration-level public tests.

Required outcome
- Tests prove authored structure -> compile -> export -> load -> session ->
  input -> step -> output read.
- Tests cover deterministic repeatability.
- Tests cover malformed artifact rejection, unsupported-structure rejection,
  invalid-handle behavior, and invalid-lifecycle behavior.

Important constraint
- Tests must exercise the public boundary, not private backend internals.

Task 10 completion definition
- Breadboard tests must cover:
  - admitted executable compile success
  - unsupported executable subset rejection
  - deterministic descriptor mapping
  - deterministic export shape for equivalent structure
- Forge tests must cover:
  - real fast artifact admission
  - malformed executable payload rejection
  - real session create/reset/destroy
  - input application failure and success paths
  - advancement failure and success paths
  - output read failure and success paths
- Integration tests must cover:
  - pass-through shape
  - unary inversion shape
  - binary gate shape
  - small multi-node combinational chain
  - deterministic repeated execution
  - explicit refusal paths for out-of-scope structure
- Tests must remain public-boundary tests through Breadboard and Forge, not
  direct backend-internal tests.

### Task 11. Tighten admission language and close the phase honestly

Purpose
- Update documentation and phase-close language so the repo clearly explains
  what has become real and what remains scaffold-only.

Required outcome
- Placeholder and executable paths are distinguished explicitly.
- Admitted scope, known limits, and deferred work are documented plainly.
- The phase can be audited without ambiguity or overclaim.

Important constraint
- Documentation must remain honest.
- This phase proves the first executable path, not general-purpose Strata
  completion.

Task 11 completion definition
- Docs must state explicitly:
  - what part of Breadboard is now real
  - what part of Forge is now real
  - what artifact family is now executable
  - what still remains placeholder-only
- The admitted executable subset and its exclusions must remain documented in
  one stable place.
- The phase-close language must not claim:
  - final artifact design
  - general compiler completeness
  - temporal-backend readiness
  - broad multi-backend parity
- Audit expectations for phase close are frozen to require:
  - clean working tree after completion
  - public test evidence
  - explicit limitation reporting
  - no false promotion of placeholder behavior to real execution behavior

------------------------------------------------------------------------

## 5. Validation Standard

This phase should only be considered complete if the public boundary proves real
behavior.

### Breadboard evidence
- admitted structure compiles successfully into a real fast-path draft
- unsupported structure is rejected explicitly
- descriptor mapping remains stable after lowering
- equivalent authored structure lowers deterministically

### Forge evidence
- real fast-path artifacts load successfully
- malformed or incompatible executable artifacts are rejected explicitly
- session create, reset, input application, advancement, and output reads work
  for the admitted path
- invalid-handle and invalid-lifecycle behavior remains explicit

### Integration evidence
- the full vertical path works through the public interface
- stable descriptor identity survives the real path
- deterministic repeated execution is proven
- out-of-scope authored structure remains on explicit refusal paths

------------------------------------------------------------------------

## 6. Phase Completion Standard

This phase is complete when all of the following are true:
- `Breadboard` can compile one admitted structural subset into a real fast
  executable artifact.
- `Forge` can load that artifact and create a real session.
- Inputs can be applied and outputs can be read through the public boundary.
- The runtime can advance deterministically through the common lifecycle for
  the admitted subset.
- Placeholder and executable paths remain clearly distinguished.
- Unsupported structure and malformed runtime inputs fail explicitly.
- Public tests prove the path end to end.
- Documentation describes the result honestly without overstating generality.

------------------------------------------------------------------------

## 7. What This Phase Sets Up Next

If this phase succeeds cleanly, the next architectural choices become much
clearer:
- broaden fast-backend executable coverage
- bring `HighZ` toward the shared runtime shape
- expand real common reads, probes, and storage inspection deliberately
- begin broader recognition and lowering growth on top of a proven executable
  path

------------------------------------------------------------------------

## 8. High-Level Conclusion

This phase should be understood as:
- the first real execution checkpoint for Strata
- the point where one narrow path becomes truly executable
- the bridge from contract scaffolding into admitted runtime behavior

The right success condition is not breadth.
The right success condition is one honest, deterministic, auditable real path.
