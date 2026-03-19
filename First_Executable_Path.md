## Proposed next major phase

**Phase name:** First Real Executable Vertical Path  
**Primary target:** `FAST_4STATE` / LXS-style backend first

## Why this is the right next phase

We’ve finished the API scaffolding chunk. The next clean move is to stop adding more placeholder richness and make one path real.

That means:
- one real compile path in `Breadboard`
- one real load/session/run path in `Forge`
- one real executable backend payload
- one small admitted authored subset
- one honest common read path

This keeps the repo moving by layers instead of drifting into endless contract work.

## Phase purpose

Turn the current placeholder-only vertical path into the **first real executable Strata path** for a narrow admitted subset, while preserving all the boundary discipline we just established.

The point is not broad feature coverage yet.  
The point is to prove that Strata can now do this for real:

1. accept authored structure
2. compile it in `Breadboard`
3. emit a backend-targeted executable artifact
4. load it through `Forge`
5. create a session
6. apply inputs
7. advance execution
8. read outputs and basic runtime-visible state

## Phase outcome

At the end of this phase, Strata should support a **small but real executable path** for the fast backend.

Not “placeholder handoff passes.”
Not “artifact shape is promising.”
A real end-to-end run.

## Phase scope

### In scope

#### 1. First admitted executable subset
Choose a deliberately small structural subset that is enough to prove the architecture, for example:
- module inputs
- module outputs
- direct connectivity
- a tiny primitive family such as:
  - `NOT`
  - `AND`
  - `OR`
  - `XOR`
  - optional buffer/pass-through

The subset should be:
- deterministic
- easy to validate
- easy to execute
- sufficient for multi-node structural tests

#### 2. Real Breadboard lowering for that subset
`Breadboard` should stop being purely placeholder-oriented for the admitted subset and begin to:
- accept real authored structural declarations
- validate the subset
- lower it into backend-targeted executable content
- emit real descriptor mapping tied to that lowered result
- emit honest diagnostics when structure falls outside the admitted subset

#### 3. First real artifact family
The temporary artifact contract can remain, but this phase should introduce the first **real executable payload semantics** for the fast backend path.

That means:
- payload is no longer just a stub byte marker
- payload actually contains enough executable meaning for runtime setup
- `Forge` load validation can distinguish a real executable fast-path artifact from a placeholder one

Important constraint:
- this still does **not** need to become the final Strata artifact format

#### 4. Real Forge session lifecycle for the fast path
`Forge` should implement a real lifecycle for the admitted executable fast-backend path:
- artifact load
- session create
- reset
- input apply
- step/advance
- output read
- unload/free

The common API should remain honest and narrow.

#### 5. Real common reads for the admitted subset
For the first executable path, `Forge` should provide:
- output descriptor enumeration
- output value reads
- basic input application by stable descriptor identity/index
- stable common portable-state reads where required by the current subset

#### 6. Capability reporting becomes operational
The capability model should begin driving real behavior instead of only placeholder planning:
- fast backend reports actual support for the admitted path
- unsupported lifecycle/read/extension paths stay explicit
- `Breadboard` targeting decisions remain capability-informed, not folklore-driven

## Explicit non-goals

This phase should **not** try to do all of these at once:
- final artifact format
- full graph compiler
- broad recognition system
- macro substitution beyond a tiny admitted kernel set
- temporal execution
- `HighZ` real runtime bring-up
- richer-state native read extensions
- full probe system
- storage inspection beyond what is absolutely needed
- broad authoring model or UI concerns
- multi-backend parity

This phase should stay narrow on purpose.

## Architectural shape of the phase

### Breadboard responsibilities
In this phase, `Breadboard` should become real in one narrow area:
- authored structural legality checks for the admitted subset
- deterministic lowering for the fast target
- descriptor mapping tied to real lowered structure
- honest diagnostics for unsupported structure
- explicit refusal instead of placeholder success when the user asks for real lowering outside the admitted subset

### Forge responsibilities
In this phase, `Forge` should become real in one narrow runtime area:
- real artifact acceptance for the fast-path executable payload
- real session creation
- real runtime advancement
- real value reads
- explicit rejection of unsupported runtime features

### Backend responsibilities
The fast backend path should own:
- executable representation interpretation
- runtime state allocation
- deterministic step behavior
- output computation
- native runtime truth for the admitted subset

Backends should still not absorb:
- structural recognition
- authoring semantics
- compiler policy
- artifact authoring logic

## Recommended execution strategy for the phase

### 1. Pick one backend and one subset
Do not split focus.
This phase should choose:
- `FAST_4STATE`
- one tiny primitive subset
- one deterministic advancement model

### 2. Keep the current placeholder path alive as scaffolding fallback
Do not rip out the placeholder machinery yet.
It still has value for:
- temporal path scaffolding
- comparison tests
- contract preservation while the first real path comes online

### 3. Add reality beside scaffolding, then tighten admission
The clean sequence is:
- add real fast-path executable support
- prove it through public tests
- only then begin reducing dependency on placeholder success paths for that subset

### 4. Keep “real” and “placeholder” explicit
A major risk in this phase is mixed truth.
The repo should stay very explicit about:
- which drafts are still placeholder
- which artifacts are actually executable
- which runtime paths are real
- which targets remain scaffold-only

## Validation expectations for this phase

This phase should be admitted only if the public boundary proves real behavior.

### Breadboard evidence
Need tests for:
- successful compile of the admitted structural subset
- explicit rejection of unsupported structure
- stable descriptor mapping after lowering
- deterministic artifact emission for equivalent structure

### Forge evidence
Need tests for:
- real artifact load for the fast path
- explicit rejection of malformed or mismatched executable payloads
- session create/reset/advance
- input application
- output reads
- invalid-handle and invalid-lifecycle rejection paths

### Integration evidence
Need end-to-end tests for:
- authored structure -> Breadboard compile -> Forge load -> session create -> step -> output read
- stable descriptor identity surviving the real path
- deterministic repeated execution on the same artifact
- refusal paths when structure exceeds the admitted executable subset

## Key design constraints for the phase

### 1. No fake success
If a path is not real yet, it should still fail explicitly.

### 2. No hidden backend coupling above Forge
Tests and upper layers must continue to go through `Forge`.

### 3. No premature final-format lock-in
The payload may become real without pretending the entire serialized artifact design is complete.

### 4. No semantic overclaim
This phase proves:
- first executable lowering
- first runtime execution path

It does **not** prove:
- general-purpose compilation
- complete backend integration
- final runtime model
- temporal parity

## Exit criteria

I would consider this phase complete when all of the following are true:

1. `Breadboard` can compile one admitted structural subset into a real fast-backend executable artifact.
2. `Forge` can load that artifact and create a real session.
3. Inputs can be applied and outputs can be read through the public boundary.
4. The runtime can advance deterministically through the common lifecycle for that subset.
5. Placeholder and real executable paths are clearly distinguished.
6. Unsupported structure and unsupported runtime operations fail explicitly.
7. The full public test suite for this phase passes from a clean state.
8. Docs describe the new real path honestly without overstating generality.

## What this sets up next

If we do this phase well, the next major choices become clean:
- broaden fast-backend executable coverage
- bring `HighZ` up to the same shared runtime shape
- introduce real probe/storage/common read growth
- begin real recognition/lowering expansion above the now-proven runtime path

## Recommendation

This phase should be planned as:

**“First executable fast-backend vertical path for a minimal admitted structural subset.”**

That is the cleanest next major chunk.

If you want, next I can turn this into a repo-style formal phase plan document draft in the Strata doc tone.