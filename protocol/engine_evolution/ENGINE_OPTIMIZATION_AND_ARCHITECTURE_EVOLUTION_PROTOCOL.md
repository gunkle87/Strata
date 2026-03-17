# Engine Optimization And Architecture Evolution Protocol

Purpose
- This document defines a portable operating protocol for evolving simulation
  engines and their surrounding compiler, recognition, macro, benchmark, and
  UI-control layers.
- The goal is to make difficult systems work auditable, measurable, and safe to
  continue across long time spans, multiple agents, and multiple repos.

Core belief
- Engine work fails most often from drift, hidden assumption stacking, baseline
  loss, and over-crediting artifact wins.
- This protocol exists to prevent those failure modes.

## 1. Problem This Protocol Solves

Engine projects regularly mix together:
- semantic correctness work,
- compiler lowering work,
- structural recognition work,
- runtime fast-path work,
- benchmark harness changes,
- measurement instrumentation,
- UI or API integration work.

If those move in parallel without strict separation, the project loses the
ability to answer basic questions:
- What changed?
- What actually improved?
- Which layer owns the win?
- Which branch is live?
- Was the benchmark gain real or accidental?
- Can the work be reused in another engine?

This protocol forces those answers to remain available.

## 2. Design Goals

The protocol optimizes for:
- correctness before speed claims,
- attribution before architecture conclusions,
- admission discipline before branch enthusiasm,
- portability of method across engines,
- recoverability after regression,
- continuity across contributors and agents,
- explicit stopping rules.

## 3. Layer Separation Rules

Every change must be classified before implementation.

Canonical layers:
- `semantic layer`
  - logic model
  - state definition
  - sequential commit rules
  - observable behavior
- `compiler or breadboard layer`
  - parsing
  - lowering
  - structural recognition
  - macro admission or replacement
  - executable plan construction
- `runtime layer`
  - scheduling
  - execution kernels
  - data layout
  - dispatch
  - probes and counters
- `benchmark or harness layer`
  - corpus selection
  - sample policy
  - median or aggregate policy
  - report formats
- `control plane layer`
  - API
  - UI bridge
  - hot-edit or recompile path
  - tooling integration

Rules
- A phase must declare which layer it is allowed to modify.
- If a change needs another layer to move, the phase pauses and a new phase must
  be declared.
- Runtime work may not silently alter semantics.
- Recognition work may not silently redefine kernel meaning.
- Benchmark harness work may not be credited as engine improvement unless the
  effect is explicitly runtime-affecting and default-live.

## 4. Required Work Units

All work must live inside one of these units:
- `phase`
  - a bounded work program with explicit success criteria
- `candidate`
  - a proposed implementation or recognizer family under evaluation
- `checkpoint`
  - a measured record of what happened
- `admission`
  - a decision to make a runtime-affecting change live by default
- `phase close`
  - a closure document that records what the phase proved and what it did not

If work cannot be expressed in those units, it is not ready.

## 5. Phase Lifecycle

Every phase must follow this sequence.

1. Define the phase
- State the layer in scope.
- State the exact question being tested.
- State what counts as success.
- State what counts as failure.
- State what is explicitly out of scope.

2. Pin the baseline
- Record the protected comparison point.
- Record corpus, settings, toolchain assumptions, and measurement mode.
- Record whether the baseline is historical, rerun, or same-toolchain.

3. Define candidates
- Name each candidate clearly.
- State the hypothesis for each candidate.
- State the legality boundaries.
- State what negative cases must fail.

4. Validate correctness first
- Unit tests
- fixture tests
- parser-backed tests if relevant
- equivalence or behavior checks

5. Run report-only qualification
- Measure coverage, structure, or artifact behavior without replacing the live
  path if possible.
- If report-only fails to justify continuation, stop the family.

6. Run replacement or live-path qualification
- Compare only against the pinned baseline or the current admitted live path.
- Record whether the result is artifact-only or suite-surviving.

7. Decide
- `ADMIT`
- `KEEP ALTERNATE`
- `KEEP KERNEL, DEFER RECOGNITION`
- `REJECT CHANGE`
- `HOLD FOR LATER`

8. Update ledgers
- admitted-path ledger
- baseline ledger
- no-admission or alternate inventory

9. Close the phase
- State what is now live.
- State what remains deferred.
- State re-entry criteria.

## 6. Attribution Discipline

A result is only meaningful if its owning layer is known.

Attribution classes
- `semantic`
  - changes observable logic behavior or correctness
- `kernel`
  - changes primitive or runtime execution cost
- `compiler`
  - changes lowering, packing, recognition, or macro selection
- `boundary`
  - changes transition cost between layers or execution modes
- `instrumentation`
  - changes reporting or counters but should not materially change runtime
- `benchmark governance`
  - changes the way measurements are collected or interpreted

Rules
- Every checkpoint must name one primary attribution class.
- If more than one class moved, the checkpoint must say so explicitly.
- If attribution is ambiguous, the result is provisional.
- A provisional result cannot be admitted.

## 7. Recognition And Macro Placement Rule

Recognition and macro systems are not engine identity by default.

Portable rule
- structural recognition,
- macro discovery,
- macro replacement,
- admission heuristics,
- match legality,
- report-only census,
- replacement qualification

should be treated as compiler or breadboard layer services unless there is a
clear reason they must live inside the raw runtime.

Reason
- these capabilities can often improve multiple engines,
- they are easier to govern above the runtime,
- they are easier to disable, compare, and port when kept separate.

## 8. Default-Path Admission Rule

A change may only be credited as a real engine improvement if:
- it affects the default live path,
- it survives the protected comparison corpus,
- it has a recorded checkpoint,
- its baseline and settings are known,
- it is reflected in the admitted-path ledger.

Things that are not enough on their own
- isolated fixture wins
- single artifact wins
- internal counters that look promising
- a faster branch that is not admitted
- a change that only helps under special flags

## 9. Report-Only Before Replacement

Whenever feasible, use report-only qualification before replacement.

Purpose
- separate structure discovery from replacement profitability,
- reduce correctness risk,
- avoid over-crediting recognizers that match but do not pay off,
- learn whether the candidate is failing due to legality, coverage, or runtime
  cost.

If report-only is impossible, the phase document must explain why.

## 10. Recovery Rule

When a branch regresses against the last stable plateau:
- stop adding new concept families,
- checkpoint the current state,
- run a bounded consolidation or recovery pass,
- compare directly against the protected plateau,
- do not reopen expansion until recovery is evaluated.

This protects the project from compounding regressions under optimism.

## 11. Alternate Preservation Rule

A rejected or non-admitted branch is still valuable if documented correctly.

Keep alternates when they provide one of:
- stronger artifact behavior,
- future re-entry value,
- insight into bottlenecks,
- correctness scaffolding,
- architectural lessons.

But do not allow alternates to masquerade as live wins.

## 12. Phase Closure Rule

A phase is not complete when implementation stops.
It is complete when the following exist:
- phase-close summary,
- current live decision,
- deferred inventory,
- re-entry criteria,
- baseline impact summary.

Without that closure, later work will misremember what happened.

## 13. Auditability Rule

The protocol assumes future audits.

Therefore every serious phase should leave behind:
- exact corpus identity,
- settings,
- comparison class,
- baseline reference,
- current live decision,
- rerun requirement status,
- known weak points,
- unresolved questions.

## 14. Agent-Transfer Rule

The protocol must survive model swaps and contributor turnover.

Therefore:
- phase plans must be explicit,
- templates must be shared,
- ledgers must be current,
- non-admissions must be recorded,
- blockers must be written down,
- no important state may live only in chat or commit history.

## 15. Hard Stops

Stop and realign when any of these occur:
- semantics need to change inside a performance phase,
- attribution becomes ambiguous,
- benchmark policy drifts mid-phase,
- the candidate expands beyond the declared layer,
- regression continues across multiple checkpoints without a recovery pass,
- correctness confidence drops below measurement confidence.

## 16. Outputs This Protocol Requires

At minimum, a repo using this protocol should maintain:
- a phase plan per active phase,
- a checkpoint file per meaningful measured step,
- an admitted-path ledger,
- a baseline ledger,
- a no-admission or alternate inventory,
- a phase-close note for every concluded branch,
- a corpus provenance record,
- a re-entry policy for deferred work.

## 17. What Success Looks Like

The protocol is working if, at any point, a new contributor or agent can answer:
- what is live,
- what was tried,
- what failed,
- what was deferred,
- what the current baseline is,
- why the last admission was allowed,
- which layer owns the current bottleneck,
- what the next safe move is.

If those questions cannot be answered quickly, the protocol is not yet being
followed completely.
