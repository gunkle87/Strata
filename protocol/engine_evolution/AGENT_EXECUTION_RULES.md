# Agent Execution Rules

Purpose
- This file translates the protocol into direct operational rules for agents
  and contributors.
- It is written so a fresh agent can work from it without importing hidden
  context from prior chats.

## 1. Start Rule

Before doing substantial work:
- identify the active phase,
- identify the protected baseline,
- identify the allowed layer of change,
- identify the exact success condition,
- identify the required output documents.

If any of those are missing, stop and write or request the missing phase frame.

## 2. No Silent Scope Expansion

An agent may not:
- widen the phase question,
- add adjacent architecture work,
- redefine success conditions,
- redesign another layer to rescue the current layer,
- convert a narrow pass into a broad rewrite.

If the current work uncovers a larger need:
- checkpoint,
- report the blocker,
- propose the next phase,
- do not smuggle it into the current run.

## 3. Correctness Before Throughput

The order is:
1. fixture or unit correctness
2. parser-backed or realistic correctness
3. report-only or structural qualification
4. replacement or live-path qualification
5. admission

Agents may not skip directly to benchmark claims without earlier gates.

## 4. Report-Only Preference

If the work involves:
- recognition,
- replacement logic,
- macro substitution,
- compiler pattern detection,
- execution-mode selection,

then report-only qualification should be used before replacement whenever
possible.

## 5. Evidence Class Labeling

Every measured result must be labeled as one of:
- fixture-only
- artifact-only
- stress-pack
- default-suite screening
- default-suite admission
- historical recorded comparison
- same-toolchain rerun

Never mix these labels casually.

## 6. Admission Discipline

An agent may only recommend `ADMIT` if:
- correctness is already established,
- the comparison class supports admission,
- the baseline is protected and documented,
- the result is reflected in the required ledgers.

If any of those are missing, the strongest allowed outcome is a non-admission
status.

## 7. Alternate Handling

If a candidate is useful but not admitted:
- keep it clearly labeled as alternate or rejected,
- record why it was not admitted,
- record what it might still be useful for later.

Never let alternates stay ambiguous.

## 8. Benchmark Integrity

Agents may not:
- change corpus membership mid-phase without recording it,
- change measurement settings without saying so,
- use unmarked historical values as if they were reruns,
- over-credit noise,
- present a special-mode win as a default-path win.

## 9. Blocker Behavior

If progress is blocked by:
- missing API surface,
- missing docs,
- conflicting repo state,
- ambiguous semantics,
- broken harness conditions,

then:
- stop the affected line of work,
- write the blocker plainly,
- state what assumption would otherwise be required,
- do not patch around it invisibly.

## 10. Documentation Minimum

A serious implementation run is incomplete unless it updates:
- the checkpoint or creates a new one,
- the phase status or plan if required,
- the admitted-path ledger if the live path changed,
- the baseline ledger if the protected baseline changed.

## 11. Audit Friendliness

Agents should write docs so an auditor can confirm:
- what changed,
- why it changed,
- how it was measured,
- whether it is live,
- what remains deferred.

## 12. Preferred Communication Pattern

Agents should report in this order:
1. what was attempted
2. what was measured
3. what is now live
4. what is explicitly not admitted
5. what the next safe move is

That ordering keeps momentum without sacrificing truthfulness.
