STRICT IMPLEMENTATION DIRECTIVE
(NO DRIFT, NO EXPANSION, TASK-LOCKED)

---

STATE RESET REQUIREMENT:

Ignore all prior conversational context.

This directive is fully self-contained and authoritative.

Only the information provided below defines scope.

---

OBJECTIVE:

Implement the specified task EXACTLY as defined.

The goal is to produce a minimal, correct implementation that:
- satisfies the task intent
- does not exceed scope
- preserves architectural and phase constraints

---

EMBEDDED REPOSITORY RULES SUMMARY
(AUTHORITATIVE, COMPACT, NO EXTERNAL DEPENDENCY):

Core doctrine:
- structure is the authoring layer
- function is the execution layer
- authored structure is not execution
- structural translation is not execution

System identity:
- Breadboard = structural compiler layer
- Forge = shared runtime API layer
- LXS = fast backend
- HighZ = temporal backend

Layer ownership:
- Authoring owns structure, hierarchy, ports, connections, and edit intent
- Breadboard owns import, normalization, validation, recognition, macro
  substitution, and backend-targeted lowering
- Forge owns shared lifecycle, common reads, common probes, capability
  discovery, and backend dispatch boundary
- Backends own native state model, native scheduling, execution kernels, state
  commit behavior, and backend-specific diagnostics
- Tooling owns conversion, corpus prep, benchmarking, reporting, and support

Hard boundary rules:
- do not blur layers
- do not move behavior into the wrong layer for convenience
- do not couple backends together
- all runtime interaction must happen through Forge
- no UI, tool, or test may reach into backend internals directly

Recognition and macro rules:
- recognition belongs in Breadboard, not in the backends
- macro semantic definition belongs above execution
- macro execution kernels belong in or beside the backend that executes them

State-model rules:
- preserve native backend truth for execution
- shared observation may normalize for interoperability
- unsupported state distinctions must be handled during structural compilation
- do not force one backend to carry another backend's native runtime truth

Testing and tooling rules:
- tests and tooling must remain distinct from engine code
- tests must exercise public interfaces, not private internals
- benchmarks or tooling must not become the real engine by accident

Clean-room rule:
- all code must be original work for Strata or clean-room work based only on
  allowed internal host-system architectural references
- no copying or adapting external code

Style baseline:
- PascalCase for types and major concepts
- snake_case for functions, variables, and file names
- UPPER_SNAKE_CASE for constants and macros
- keep core logic flat and data-oriented
- no object-oriented design patterns in core engine code
- Opening brace on new line, one tab beyond the parent declaration or command.
- Body aligned to the opening brace indentation.
- Closing brace on its own line, followed by a blank line.

Conflict handling:
- if the task conflicts with these rules, STOP
- state the conflict plainly
- do not silently proceed

Response style:
- use plain English
- keep language short and clear
- avoid unnecessary jargon

---

TASK CONTEXT:

- Pillar: [PILLAR]
- Task ID: [TASK XX]

Task Directive:
[PASTE TASK DIRECTIVE]

---

GOVERNING PRIORITY:

1. Task intent (absolute authority)
2. Phase constraints (allowed temporary behavior)
3. Architecture (final system rules)

---

SCOPE LOCK (CRITICAL):

You MUST:

- implement ONLY what is explicitly required
- touch ONLY files necessary for this task
- avoid introducing any behavior not required

You MUST NOT:

- anticipate future tasks
- expand features
- generalize beyond task intent
- improve design beyond requirements

---

PRE-IMPLEMENTATION DECLARATION (REQUIRED):

Before writing code, you MUST state:

1. Files that will be modified or created
2. Why each file is required
3. What will change in each file (brief)

RULE:
- No code before this step
- If unclear -> STOP and request clarification

---

IMPLEMENTATION RULES:

- All code must be original (clean-room)
- Follow existing project structure and naming
- Maintain strict layer boundaries
- Preserve all existing behavior unless task requires change

---

BOUNDARY ENFORCEMENT:

You MUST NOT:

- move logic across layers
- introduce cross-layer coupling
- bypass defined APIs
- embed runtime logic into compile layers or vice versa

---

PLACEHOLDER / PHASE RULES:

- If task uses placeholders:
  - they must remain clearly identifiable
  - must not masquerade as final implementation

- If introducing real behavior:
  - must align with phase constraints
  - must not break existing placeholder pathways

---

TEST ALIGNMENT:

- Only modify or create tests if required by task
- Tests must:
  - validate through public interfaces
  - not rely on internal shortcuts

---

STRICT PROHIBITIONS:

- NO unrelated refactoring
- NO cleanup outside task scope
- NO performance optimization unless required
- NO speculative improvements
- NO undocumented behavior

---

POST-IMPLEMENTATION VALIDATION (REQUIRED):

After implementation, you MUST provide:

1. Summary of what was implemented
2. Confirmation that:
   - scope was not exceeded
   - no unrelated files were modified
   - architectural boundaries were preserved

3. Explicit list:
   - files changed
   - functions added/modified

---

OUTPUT FORMAT:

--- PRE-IMPLEMENTATION PLAN ---
[files + changes]

--- IMPLEMENTATION ---
[code]

--- POST-IMPLEMENTATION SUMMARY ---
[what was done]

--- VALIDATION ---
- Scope respected: YES/NO
- Boundaries preserved: YES/NO
- Extra changes introduced: YES/NO

---

FAIL CONDITIONS (INVALID OUTPUT):

- code written before plan
- scope expansion
- missing file list
- undocumented changes
- architectural boundary violations
- speculative additions
- non-English output

---

CONSTRAINT:

- Implement only this task
- Do not anticipate future work
- Do not modify system design

---

ENFORCEMENT:

If any rule is violated:
-> STOP
-> OUTPUT IS INVALID
