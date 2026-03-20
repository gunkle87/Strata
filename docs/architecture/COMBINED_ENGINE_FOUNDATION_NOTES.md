# Combined Engine Foundation Notes

Purpose
- Preserve the key architectural conclusions reached during the combined-engine
  planning discussion.
- Keep these notes separate from any individual engine implementation plan.
- Record the reasoning plainly enough that a future contributor or agent can
  recover the intent without replaying the conversation.

Status
- These notes are design conclusions and planning guidance.
- They are not yet a committed implementation roadmap.

------------------------------------------------------------------------

## 1. Primary Framing

The clearest framing reached in this discussion is:

- `structure is the authoring layer`
- `function is the execution layer`

Why this matters
- It gives the whole system a simple mental model.
- It separates what the user builds from what the runtime executes.
- It explains where recognition belongs.
- It explains why the API sits lower in the stack than originally assumed.

Interpretation
- `structure`
  - components
  - wires
  - ports
  - hierarchy
  - edit intent
- `function`
  - executable plan
  - native ops
  - macros
  - runtime scheduling
  - state transition behavior

Key consequence
- structure should not be executed directly
- structure must be translated into function

------------------------------------------------------------------------

## 2. Layered System Model

The intended system is layered.

Bottom-up stack

1. `execution backend layer`
- runs compiled function
- owns runtime truth

2. `shared runtime API layer`
- sits directly on top of execution
- exposes stable lifecycle and inspection operations

3. `structural compiler layer`
- converts structure into backend-targeted function
- owns recognition and macro substitution

4. `authoring layer`
- owns the circuit model the user edits
- owns UI-facing structure semantics

5. `tooling layer`
- converters
- corpus prep
- report generation

Key consequence
- the engine is not the whole system
- the engine is only the execution layer

------------------------------------------------------------------------

## 3. Engine Purity Rule

The execution backend should be kept as pure as possible.

The engine should own
- runtime state
- scheduling
- execution kernels
- state commit semantics
- probes and diagnostics
- runtime session lifecycle

The engine should not own
- authored structure
- recognition policy
- file conversion tooling
- broad structural normalization
- UI semantics

This is one of the most important architectural corrections from this planning
discussion.

------------------------------------------------------------------------

## 4. Recognition Placement

Recognition should not live in the raw engine.

Recognition is:
- structural translation
- motif discovery
- legality checking
- report-only census
- replacement qualification

Recognition is not:
- raw execution
- primitive runtime semantics
- authoring itself

Correct placement
- structural compiler layer

Reason
- recognition refines authored structure into better executable function
- it does not need to burden the engine hot path
- it is portable across multiple backends if kept above execution

Important realization
- recognition is not mainly about rewriting into `.bench`
- recognition is about refining structure into native executable form

------------------------------------------------------------------------

## 5. Macro And Primitive Placement

Important split
- macro semantic definition belongs above the engine
- macro execution kernel belongs in or beside the engine

Similarly
- primitive semantic identity can be cataloged above the engine
- primitive execution remains engine-native

This leads to a useful rule:
- an `AND` gate and an `ALU8` can both be treated as executable functions
  at the function layer

The category boundary is not size.
The category boundary is executability.

Key consequence
- the engine should not philosophically distinguish “small” and “large”
  functions beyond what scheduling and storage require

------------------------------------------------------------------------

## 6. API Placement

One of the most important corrections made in this discussion:

- the engine API does not sit at the very top of the system
- the engine API sits directly on top of execution

This means there may be multiple interfaces in the full system:

- `authoring API`
  - for structure and editor interactions
- `compiler API`
  - for structure-to-function translation
- `runtime API`
  - for execution, stepping, reads, probes, reset

When discussing “the engine API,” the important one is the runtime API.

The runtime API should expose
- load compiled plan
- create session
- reset
- apply inputs
- tick or run
- read outputs
- read nets and stateful objects
- read probes
- query metadata
- report capabilities

------------------------------------------------------------------------

## 7. Shared Common API Versus Extension APIs

The intended combined system should use:
- one `shared common runtime API`
- backend-specific `extension APIs`

Shared common API
- lifecycle operations
- portable state reads
- output reads
- metadata queries
- common probes
- capability reporting

Backend-specific extensions

Temporal backend examples
- step one propagation unit
- step one delta phase
- inspect intermediate propagation state
- pause on transition boundaries

Fast backend examples
- bulk tick many
- high-throughput run modes
- fast-path profiling controls
- packed or region execution diagnostics

Key rule
- backend-specific features must be explicit
- they should not be hidden inside the shared contract

------------------------------------------------------------------------

## 8. Two Backend Conclusion

Current conclusion:
- if the goal is both:
  - `rocket speed execution`
  - `granular propagation control`
- then the clean architecture is likely:
  - one system
  - one structure layer
  - one structural compiler
  - one shared common runtime API
  - two execution backends

This is better than trying to force one execution core to perfectly serve both
purposes.

Reason
- a fast backend and a temporal backend want different runtime priorities
- trying to merge them deeply often weakens both

Recommended framing
- not two separate products
- but two backends inside one larger simulation ecosystem

------------------------------------------------------------------------

## 9. State Model Conclusion

Another major conclusion:
- do not force all backends into one native runtime truth

Example problem
- HighZ-style backend has 7-state or 8-state semantics
- LXS-style backend has 4-state semantics

Recommended rule
- preserve native truth for execution
- normalize only for interoperability

Portable principle
- `normalize for interoperability, preserve native truth for execution`

Implications
- LXS remains 4-state natively
- HighZ remains richer-state natively
- shared API exposes a common portable state view
- backend extensions expose native richer state where appropriate

------------------------------------------------------------------------

## 10. Unsupported State Handling

### `UNINIT`

Conclusion
- `UNINIT` should not survive into steady-state execution in a throughput
  backend like LXS
- it should be resolved during initialization, reset, or first-resolution setup

Reason
- it is an initialization concern more than a runtime-native state

### `WEAK_1` and `WEAK_0`

Conclusion
- preserve them in richer-state backends
- project or flatten them when targeting a 4-state backend like LXS

Important caution
- flattening is not “free truth”
- it is an explicit semantic approximation policy

Best placement
- translation-time projection in the structural compiler

Not recommended
- carrying richer state semantics into the fast backend runtime unless that
  backend is intentionally being redesigned around them

------------------------------------------------------------------------

## 11. Translation-Time Projection Rule

When targeting a backend:
- the structural compiler should know which backend is being targeted
- the structural compiler should apply backend-specific state projection rules
- the emitted executable plan should already fit the target backend’s native
  model

This means:
- unsupported states are handled before steady-state execution
- the engine runtime remains cleaner
- backend semantics stay explicit

Important nuance
- projection does not always mean literal expansion into ordinary logic gates
- some behavior is best represented as backend-specific semantic lowering, not
  giant baked gate trees

------------------------------------------------------------------------

## 12. Tooling Placement

Realization
- some items do not belong in the engine repo at all

Example
- `blif2bench` should likely be a standalone tool or tooling-layer app

Reason
- format conversion is not execution identity
- tooling belongs beside authoring and translation, not inside raw runtime

Recommended separation
- tooling repo or package
- structural compiler repo or package
- engine repo
- UI repo

------------------------------------------------------------------------

## 13. UI Planning Lesson

A broader lesson emerged from the UI experience:
- the UI work did not fail because the requested features were wrong
- it struggled because the request shape was feature-first instead of layer-first

What happened
- foundation and roof were built together
- interaction truth and higher-level features were mixed in one pass

Corrected understanding
- the UI should also be layered

Suggested UI layering
- interaction foundation
- authoring model
- translation boundary
- execution presentation
- polish layer

This lesson strongly reinforced the same principle used for the engine stack:
- build by layers, not by mixed feature piles

------------------------------------------------------------------------

## 14. Process Lesson

Another major outcome:
- one of the highest-value assets created in LXS was not only code
- it was the development protocol

That protocol includes
- phase plans
- bounded candidates
- report-only before replacement
- admission discipline
- baseline tracking
- checkpointed measurement
- phase-close summaries
- alternate retention without over-credit

This led to the creation of:
- [ENGINE_OPTIMIZATION_AND_ARCHITECTURE_EVOLUTION_PROTOCOL.md](/c:/DEV/LXS/protocol/engine_evolution/ENGINE_OPTIMIZATION_AND_ARCHITECTURE_EVOLUTION_PROTOCOL.md)

Reason to preserve this here
- the combined-engine effort should use the same rigor from the start

------------------------------------------------------------------------

## 15. Current Architectural Direction

The current intended direction is:

1. separate one execution backend cleanly from everything above it
2. formalize the shared common runtime API
3. formalize backend capability reporting and extension surfaces
4. define structural compiler ownership clearly
5. move recognition and macro semantic policy above execution
6. define state projection rules for backend targeting
7. bring a second backend into the shared API shape

This is the beginning of a combined-engine program, not just an LXS refactor.

------------------------------------------------------------------------

## 16. Key One-Line Principles

- `structure is the authoring layer`
- `function is the execution layer`
- `recognition is structural translation`
- `the engine is a function executor`
- `normalize for interoperability, preserve native truth for execution`
- `shared common API for universals, extension APIs for strengths`
- `build by layers, not by mixed feature piles`

------------------------------------------------------------------------

## 17. Relationship To Other Docs

These notes pair with:
- [ENGINE_LAYER_HIERARCHY_MAP.md](/c:/DEV/LXS/docs/architecture/ENGINE_LAYER_HIERARCHY_MAP.md)
- [ENGINE_LAYER_HIERARCHY_BOTTOM_UP.md](/c:/DEV/LXS/docs/architecture/ENGINE_LAYER_HIERARCHY_BOTTOM_UP.md)
- [COMBINED_ENGINE_ARCHITECTURE_PLAN.md](/c:/DEV/LXS/docs/architecture/COMBINED_ENGINE_ARCHITECTURE_PLAN.md)
- [HighZ-AUDIT.md](/c:/DEV/LXS/HighZ-AUDIT.md)
- [HEBS_AUDIT.md](/c:/DEV/LXS/HEBS_AUDIT.md)

Role of this file
- capture the reasoning and conclusions from the planning conversation
- preserve the logic that produced the current architecture direction
- reduce the chance of forgetting why these boundaries were chosen

