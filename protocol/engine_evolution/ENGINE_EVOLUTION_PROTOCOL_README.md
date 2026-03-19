# Engine Optimization And Architecture Evolution Protocol

Purpose
- This directory captures a portable development protocol for evolving logic
  engines, compiler layers, runtime kernels, benchmark harnesses, and shared
  UI/control planes without losing rigor.
- It is intentionally not tied to LXS, HighZ, HEBS, or any single engine.
- It exists so another repo, team, or agent can adopt the same operating
  system without inheriting hidden assumptions.

What this protocol is
- A method for:
  - planning risky architecture work,
  - isolating correctness from performance,
  - separating kernel wins from recognition wins,
  - admitting only measured default-path improvements,
  - documenting rejections and alternates,
  - preserving a trustworthy baseline across long runs.

What this protocol is not
- It is not a coding style guide.
- It is not a product roadmap.
- It is not tied to one simulator model, one logic-state count, or one
  benchmark corpus.

Portable design rules
- Use generic placeholders instead of repo-specific constants.
- Keep benchmark policy separate from engine identity.
- Keep recognition and macro work separate from raw runtime semantics.
- Keep measurement governance separate from implementation enthusiasm.

Directory map
- `ENGINE_OPTIMIZATION_AND_ARCHITECTURE_EVOLUTION_PROTOCOL.md`
  - the full end-to-end method
- `AGENT_EXECUTION_RULES.md`
  - operational rules for agents and contributors
- `EXPERIMENT_GOVERNANCE.md`
  - benchmark discipline, admission rules, and evidence classes
- `PHASE_TEMPLATE.md`
  - required structure for a new phase
- `CHECKPOINT_TEMPLATE.md`
  - minimum structure for a checkpoint
- `ADMISSION_AND_BASELINE_LEDGER_PROTOCOL.md`
  - how admitted wins and protected baselines are recorded
- `PHASE_CLOSE_TEMPLATE.md`
  - how to close a phase cleanly
- `REENTRY_AND_DEFER_POLICY.md`
  - how to stop, defer, or later re-enter work
- `PORTABILITY_ADOPTION_GUIDE.md`
  - how to install this protocol in another repo

Suggested adoption order
1. Read:
   - `ENGINE_OPTIMIZATION_AND_ARCHITECTURE_EVOLUTION_PROTOCOL.md`
2. Install:
   - `PHASE_TEMPLATE.md`
   - `CHECKPOINT_TEMPLATE.md`
   - `ADMISSION_AND_BASELINE_LEDGER_PROTOCOL.md`
3. Enforce:
   - `EXPERIMENT_GOVERNANCE.md`
   - `AGENT_EXECUTION_RULES.md`
4. Use phase closure and re-entry rules for any branch that lasts longer than
   one phase.

Origin note
- This protocol was extracted from a real engine-development run where the
  process itself became one of the highest-value artifacts.
- The purpose of this directory is to preserve that method in a reusable form.
