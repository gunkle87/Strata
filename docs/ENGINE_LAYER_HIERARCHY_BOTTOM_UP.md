# Engine Layer Hierarchy Bottom-Up

Purpose
- This document describes the intended layered system from the bottom up.
- It starts with the engine as the parent execution system and expands upward
  into the layers that should feed it.
- It is written as a target hierarchy, not a claim that current LXS is already
  fully separated this way.

Key framing
- `structure is the authoring layer`
- `function is the execution layer`

That means:
- the engine should own function execution,
- the translation layer should turn structure into function,
- the authoring layer should own user-facing circuit meaning.

------------------------------------------------------------------------

## 1. Root Parent: Engine

Definition
- The engine is the function executor.
- It should not own authoring structure.
- It should not own structural recognition policy.
- It should not own file conversion tooling.

Core functions
- load an executable plan
- allocate and own runtime state
- apply inputs
- execute one tick or many ticks
- preserve deterministic state transition rules
- expose outputs, probes, and runtime-visible state
- reset and free runtime state

Current LXS anchor points
- [lxs_engine.c](/c:/DEV/LXS/src/core/lxs_engine.c)
- [lxs_api.h](/c:/DEV/LXS/include/lxs_api.h)
- [lxs_types.h](/c:/DEV/LXS/include/lxs_types.h)

Children
- Runtime state model
- Execution scheduler
- Native function catalog
- Storage-function executors
- Probe and diagnostics surface
- Runtime lifecycle boundary

------------------------------------------------------------------------

## 2. Child: Runtime State Model

Definition
- The runtime state model is the engine-owned memory image of execution.
- It is the lowest stable truth of live simulation state.

Functions
- hold net values and masks
- hold sequential current and next state
- hold primary input and primary output banks
- hold runtime counters and probes
- provide deterministic storage layout for the execution kernels

Children
- Net banks
- Sequential banks
- Input/output banks
- Probe banks

### 2.1 Grandchild: Net Banks

Functions
- store combinational net state
- provide indexed read/write targets for executable functions
- keep value and mask storage aligned and parallel

Typical contents
- `net_value[]`
- `net_mask[]`

Why it matters
- this is the raw execution substrate for gate and macro kernels
- no names, maps, or strings should be needed in hot execution

### 2.2 Grandchild: Sequential Banks

Functions
- hold current committed state
- hold next captured state
- support staged state transition

Typical contents
- current register state
- next register state
- sequential storage for RAM, regfile, and other stateful ops

Why it matters
- this is what keeps execution deterministic and cycle-correct
- sequential semantics must remain engine-owned

### 2.3 Grandchild: Input/Output Banks

Functions
- accept external input application
- expose externally readable outputs
- isolate runtime execution from external caller representation

Why it matters
- these are the clean handoff points into and out of execution

### 2.4 Grandchild: Probe Banks

Functions
- count engine activity
- count runtime events
- expose bottleneck and correctness-supporting diagnostics

Examples in current LXS API
- `input_apply`
- `chunk_exec`
- `gate_eval`
- `dff_exec`
- `tick_count`
- `state_commit_count`

Why it matters
- probes explain performance and behavior without changing semantics

------------------------------------------------------------------------

## 3. Child: Execution Scheduler

Definition
- The execution scheduler is the engine-owned mechanism that runs executable
  plan units in the correct order.

Functions
- execute combinational work in planned order
- execute stateful storage functions at the correct phase
- capture next state
- commit state
- preserve deterministic tick semantics

Current LXS anchor points
- `lxs_execute_levels`
- `lxs_capture_next_state`
- `lxs_commit_state`
- `lxs_execute_plan`
  in [lxs_types.h](/c:/DEV/LXS/include/lxs_types.h)

Children
- Plan walker
- Chunk dispatcher
- Macro dispatcher
- Functional-region dispatcher
- Sequential commit path

### 3.1 Grandchild: Plan Walker

Functions
- traverse the compiled execution plan
- step level by level or region by region
- pass the right spans to the right executor

Why it matters
- the engine should execute offsets and spans, not rediscover structure

### 3.2 Grandchild: Chunk Dispatcher

Functions
- execute primitive gate chunks
- pick the correct specialized chunk executor
- stream through contiguous plan units

Current LXS anchor points
- chunk plans in [lxs_types.h](/c:/DEV/LXS/include/lxs_types.h)
- logic chunk executors in [lxs_engine.c](/c:/DEV/LXS/src/core/lxs_engine.c)

Lowest-level responsibilities
- contiguous primitive execution
- minimal branching in hot loops
- value/mask update for primitive functions

### 3.3 Grandchild: Macro Dispatcher

Functions
- execute admitted single macros
- execute admitted multi-macros
- treat larger functions as first-class execution units

Current LXS anchor points
- macro plan structs in [lxs_types.h](/c:/DEV/LXS/include/lxs_types.h)
- `lxs_execute_macros` in [lxs_engine.c](/c:/DEV/LXS/src/core/lxs_engine.c)

Why it matters
- this is the branch where `AND2` and `ALU8` become peers as executable
  functions even if their internal width and complexity differ

### 3.4 Grandchild: Functional-Region Dispatcher

Functions
- execute compiled functional regions
- run higher-order op sequences admitted by the translation layer
- use temporary storage local to the region as required

Why it matters
- this allows execution of compiled subgraphs without forcing the runtime to
  rediscover that structure each tick

### 3.5 Grandchild: Sequential Commit Path

Functions
- capture next state after combinational evaluation
- commit state atomically for the next tick
- preserve register and memory semantics

Why it matters
- this is the engine’s semantic spine
- this should never be delegated upward

------------------------------------------------------------------------

## 4. Child: Native Function Catalog

Definition
- The native function catalog is the set of execution-unit types the engine
  knows how to run directly.

Important split
- semantic definition of a function can live above the engine
- execution kernel of a native function lives in or next to the engine

Functions
- define the runtime instruction vocabulary
- provide dispatchable execution categories
- establish what counts as native executable function

Children
- Primitive functions
- Standard macros
- Multi-macros
- Stateful standard functions

### 4.1 Grandchild: Primitive Functions

Functions
- execute minimal native logic operators

Current LXS examples
- `AND`
- `OR`
- `XOR`
- `NOT`
- `BUF`
- `TRI`
- `DFF`

Current anchor
- `lxs_gate_type` in [lxs_types.h](/c:/DEV/LXS/include/lxs_types.h)

Why it matters
- primitives are the irreducible native execution vocabulary
- they should be stable and boring

### 4.2 Grandchild: Standard Macros

Functions
- execute single-output or compact higher-order native functions

Current LXS examples
- `MUX2`
- `XOR2`
- `XNOR2`
- `PARITY4`
- `PARITY8`

Why it matters
- these are still native runtime functions
- they are no longer authored structure, they are executable meaning

### 4.3 Grandchild: Multi-Macros

Functions
- execute multi-output or structured arithmetic/control patterns

Current LXS examples
- `HALF_ADDER`
- `FULL_ADDER`
- `RIPPLE_SLICE2`
- `RIPPLE_ADD4`
- `CARRY_SAVE_ROW4`
- `REDUCE_PROPAGATE4`
- `AND_FAN8`

Why it matters
- these prove that native runtime functions do not need to be “small”
- function size is not the category boundary
- executability is the category boundary

### 4.4 Grandchild: Stateful Standard Functions

Functions
- execute higher-order stateful blocks using engine-owned sequential semantics

Current LXS examples
- registers
- ROM
- RAM
- regfile
- standard compare
- standard ALU

Why it matters
- these are still engine-native execution families
- their authored structure may be broad, but their runtime contract should be
  compact and explicit

------------------------------------------------------------------------

## 5. Child: Storage-Function Executors

Definition
- These are the engine-side kernels for stateful native functions.

Functions
- execute ROM reads
- execute RAM behavior
- execute regfile behavior
- execute register behavior

Current LXS anchor points
- `lxs_execute_roms`
- `lxs_execute_rams`
- `lxs_execute_regfiles`
  in [lxs_engine.c](/c:/DEV/LXS/src/core/lxs_engine.c)

Why it matters
- stateful function behavior should remain close to execution, because it is
  tightly coupled to commit semantics and runtime storage layout

------------------------------------------------------------------------

## 6. Child: Probe And Diagnostics Surface

Definition
- This is the engine-owned introspection layer for runtime state and activity.

Functions
- expose counters
- expose current outputs
- expose internal net reads
- expose register, RAM, and regfile reads
- support performance attribution and tooling inspection

Children
- Runtime counters
- State readers
- Structural metadata readers

### 6.1 Grandchild: Runtime Counters

Functions
- report engine activity
- explain cost centers
- support attribution phases

Why it matters
- this is where execution explains itself to the outside world

### 6.2 Grandchild: State Readers

Functions
- read outputs
- read a specific net
- read a register image
- read a RAM word
- read a regfile word

Why it matters
- this is the observation boundary needed by UI and tooling

### 6.3 Grandchild: Structural Metadata Readers

Functions
- expose plan counts
- expose input and output names
- expose net names and net IDs
- expose object widths and shapes needed for tooling

Why it matters
- this is the minimum metadata needed to bridge executable function back to
  meaningful external inspection

------------------------------------------------------------------------

## 7. Child: Runtime Lifecycle Boundary

Definition
- This is the narrow external contract directly on top of execution.
- This is the correct home for the engine API.

Functions
- load or receive a compiled plan
- create an engine session
- reset the engine
- apply inputs
- tick the engine
- query state and probes
- free resources

Current LXS anchor
- [lxs_api.h](/c:/DEV/LXS/include/lxs_api.h)

Important clarification
- the engine API is not the top of the entire stack
- it sits directly above execution

Children
- Plan intake API
- Engine session API
- State query API
- Diagnostics API

### 7.1 Grandchild: Plan Intake API

Functions
- accept a compiled plan
- expose plan counts and names
- provide a stable external handle for executable function data

### 7.2 Grandchild: Engine Session API

Functions
- create and destroy an execution session
- reset runtime state
- apply inputs
- tick once or many times

### 7.3 Grandchild: State Query API

Functions
- read outputs
- read nets
- read registers
- read memories

### 7.4 Grandchild: Diagnostics API

Functions
- read probes
- clear probes
- expose last error and result strings

------------------------------------------------------------------------

## 8. Next Parent Up: Structural Compiler Or Breadboard Layer

Definition
- This layer owns the translation from structure to function.
- This is where recognition belongs.
- This is where macro admission and structural lowering belong.

Core functions
- ingest structure from authoring or imported formats
- validate structural legality
- normalize structural graph
- recognize higher-order motifs
- replace recognized motifs with admitted functions
- lower structure into executable plan units
- emit the engine-facing function plan

Children
- Import and normalization
- Structural validation
- Recognition
- Macro substitution
- Plan generation

### 8.1 Grandchild: Import And Normalization

Functions
- ingest `.bench`, BLIF-derived forms, or UI-authored structure
- normalize names, ports, and graph shape
- produce a stable structural model before recognition

Important note
- format converters such as `blif2bench` fit here or below this layer as
  tooling, not inside the raw engine

### 8.2 Grandchild: Structural Validation

Functions
- confirm legal authored structure
- check widths, port use, fanout legality, and stateful block contracts
- reject malformed structure before execution planning

### 8.3 Grandchild: Recognition

Functions
- find meaningful motifs in structure
- classify structural opportunities
- operate in report-only or replace mode
- record legality, abort reasons, and coverage

Why it belongs here
- recognition is structural translation
- it is not raw execution
- it refines structure into better executable function

### 8.4 Grandchild: Macro Substitution

Functions
- replace recognized or explicit structural motifs with admitted native
  functions
- preserve semantic equivalence
- lower broad authored structure into compact executable meaning

Important split
- macro semantic contract belongs at this layer
- macro execution kernel belongs at the engine layer

### 8.5 Grandchild: Plan Generation

Functions
- levelize and schedule executable units
- assign offsets and spans
- build chunk, macro, multi-macro, and functional-region plans
- emit the final engine-facing plan

Why it matters
- this is the exact handoff point where structure becomes function

------------------------------------------------------------------------

## 9. Next Parent Up: Authoring Or Structure Layer

Definition
- This layer owns what the user means, not what the engine executes.

Core functions
- hold authored components, traces, and hierarchy
- preserve names and editing intent
- support hot edits and design-time validation
- act as the source of truth for structure

Children
- Circuit model
- Edit system
- Structural UI semantics
- Export to structural compiler

### 9.1 Grandchild: Circuit Model

Functions
- represent components, wires, nodes, ports, ownership, and hierarchy

### 9.2 Grandchild: Edit System

Functions
- create, delete, duplicate, connect, and move authored structure
- preserve undo and redo history

### 9.3 Grandchild: Structural UI Semantics

Functions
- hit testing
- placement legality
- selection rules
- connection rules
- viewport behavior

### 9.4 Grandchild: Export To Structural Compiler

Functions
- serialize or pass the authored structural model into the translation layer
- trigger recompilation after hot edits when required

------------------------------------------------------------------------

## 10. Sibling Tooling Layer

Definition
- These are standalone tools, not engine responsibilities.

Examples
- `blif2bench`
- corpus prep
- conversion utilities
- benchmark packaging
- report post-processing

Why this layer should stay separate
- tooling supports structure and experiments
- it should not contaminate the engine execution contract

------------------------------------------------------------------------

## 11. Bottom-Up Summary

If this hierarchy is respected:

- the engine becomes a clean function executor
- the runtime API sits directly on top of execution
- recognition moves out of the engine and into translation
- macro semantics live in translation
- macro kernels live in execution
- structure stays in the authoring layer
- tooling stays outside the engine

That gives the full system a clean meaning:

1. structure is authored
2. structure is translated into function
3. function is executed
4. state is observed through the runtime API

That is the architecture this repo should be moving toward, even if the current
codebase still contains mixed ownership from earlier phases.
