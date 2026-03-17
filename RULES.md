# Strata Repository Rules

Scope
- Entire repository

Purpose
- Define the non-negotiable architectural, interface, provenance, and style
  rules for Strata.
- Keep these rules separate from phase plans and implementation documents.

Plain-language rule
- Use plain English and conversational tone.
- This is intentional and important.

## 1. Core System Identity

Strata is the layered simulation platform that contains:
- `Breadboard`
  - structural compiler layer
- `Forge`
  - shared runtime API layer
- `LXS`
  - fast backend
- `HighZ`
  - temporal backend

Core doctrine
- `structure is the authoring layer`
- `function is the execution layer`

This means:
- authored structure is not execution
- structural translation is not execution
- the engine executes compiled function

## 2. Architectural Invariants

The following are hard constraints:

1. Layer separation is mandatory.
   - Layers must not converge.
   - Responsibilities must not collapse back into monolithic files or mixed
     ownership blocks.

2. Backend separation is mandatory.
   - `LXS` and `HighZ` must not converge.
   - They may share external contracts through `Forge`.
   - They may not silently share runtime semantics or become one compromised
     engine.

3. Shared user experience is required.
   - User-facing transition between engine modes must be smooth.
   - Internal backend boundaries must remain strict even when user-facing mode
     switching feels unified.

4. Tooling isolation is mandatory.
   - Testing, benchmarking, and tooling must remain clearly outside engine code
     and outside the composite runtime layers.
   - No benchmark runner, test harness, or tooling workflow may become the real
     engine by accident.

5. API-only interaction is mandatory.
   - All interfacing must happen through `Forge`.
   - No UI, tool, or test may reach directly into backend internals.

## 3. Backend Identity Rules

### 3.1 LXS

`LXS` must remain:
- 4-state logic
- zero-delay
- deterministic
- performance-first

Additional constraint
- LXS must not absorb richer runtime state semantics if doing so weakens its
  fast-backend identity.

### 3.2 HighZ

`HighZ` must remain:
- 7-state logic
- 3-phase delta
- temporal
- deterministic
- gate-level evaluable
- capable of multi-clock temporal behavior if required

Additional constraint
- HighZ must emphasize granularity first and performance second, but performance
  work must not sacrifice its temporal purpose.

## 4. Layer Ownership Rules

### Authoring
- authored circuit structure
- edit intent
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
- shared lifecycle
- common state reads
- common probes
- capability discovery
- backend dispatch boundary

### Backends
- native state model
- native scheduling
- native execution kernels
- state commit behavior
- backend-specific diagnostics

### Tooling
- format conversion
- corpus preparation
- benchmark orchestration
- reporting
- development governance support

Hard rule
- If code belongs to a different layer, move the code or redesign the boundary.
- Do not blur the layer just because the change is convenient.

## 5. API and Capability Rules

1. `Forge` is the only supported runtime interface boundary.
2. The shared runtime API must define:
   - common lifecycle
   - common reads
   - common probes
   - capability discovery
   - backend extension discovery
3. The shared runtime API must not silently decide the universal plan format too
   early.
4. The shared plan contract must be derived from:
   - the `Forge` lifecycle boundary
   - the `Breadboard` compiler boundary
   - backend capability constraints

## 6. State Model Rules

1. Native backend truth must be preserved.
   - `LXS` remains 4-state natively.
   - `HighZ` remains 7-state natively.

2. Shared observation may normalize state for interoperability.

3. Unsupported state distinctions must be handled during structural compilation
   when targeting a backend that cannot represent them natively.

4. `UNINIT` should not survive into steady-state fast-backend execution.

5. `WEAK` semantics may be projected during lowering for backends that do not
   support them natively.

Hard rule
- Do not force one backend to carry the other backend's native runtime truth.

## 7. Recognition and Macro Rules

1. Recognition belongs to `Breadboard`, not to the backends.
2. Macro semantic definition belongs above execution.
3. Macro execution kernels belong in or beside the backend that executes them.
4. Report-only before replacement remains the default recognition discipline
   whenever feasible.

## 8. Testing, Benchmarking, and Tooling Rules

1. Tests, benchmarks, and tooling must remain distinct from engine code.
2. No test or benchmark may mutate backend internals directly.
3. Public interfaces must be exercised through `Forge`.
4. Benchmark discipline, corpus provenance, and admission tracking must remain
   explicit and documented.

Canonical support files
- `docs/PROBE_PROTOCOL.md`
- `docs/TAB_PROTOCOL.md`
- `protocol/engine_evolution/*`

## 9. Response Style

1. Use plain English and conversational tone.
2. Keep sentences short and easy to follow.
3. Avoid jargon unless necessary; define it briefly when used.
4. Prefer concrete examples over abstract statements.
5. Keep answers brief by default unless detail is requested.
6. Ask one quick clarifying question when requirements are unclear.

## 10. Code Provenance and Clean-Room Rule

This rule is absolute.

1. No code may be copied, reused, reproduced, or adapted from external sources
   outside the host system.
2. Clean-room precedence always applies.
3. This rule must be upheld under all circumstances.
4. External code, patented ideas, or third-party code patterns are not to be
   imported into Strata.

Allowed source basis
- original work written for Strata
- internal host-system repos used as architectural references where explicitly
  chosen

Not allowed
- importing code from outside the host system
- lifting implementation blocks from external libraries or examples

## 11. Code Style Baseline

### Naming

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

### Design style

1. Core logic must remain flat, data-oriented, and cache-aware.
2. Object-oriented design patterns are prohibited in core engine code.
3. Interior function comments are allowed when they capture invariants,
   non-obvious constraints, or tricky data flow.
4. Avoid narration comments that merely restate obvious code.

### Brace style

1. Opening brace on new line, one tab beyond the parent declaration or command.
2. Body aligned to the opening brace indentation.
3. Closing brace on its own line, followed by a blank line.

## 12. Conflict Handling Rule

When a requested change conflicts with these rules:
- pause
- state the conflict plainly
- do not silently proceed past the rule

Hard rule
- architecture convenience is never a valid reason to violate a core repo rule
  without explicit documented change.
