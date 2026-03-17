# Portability Adoption Guide

Purpose
- This guide explains how to install this protocol in another repo without
  importing repo-specific assumptions.

## 1. Minimum Install Set

Copy or recreate:
- `ENGINE_OPTIMIZATION_AND_ARCHITECTURE_EVOLUTION_PROTOCOL.md`
- `EXPERIMENT_GOVERNANCE.md`
- `PHASE_TEMPLATE.md`
- `CHECKPOINT_TEMPLATE.md`
- `ADMISSION_AND_BASELINE_LEDGER_PROTOCOL.md`

That is the minimum viable protocol bundle.

## 2. Recommended Install Set

Also include:
- `AGENT_EXECUTION_RULES.md`
- `PHASE_CLOSE_TEMPLATE.md`
- `REENTRY_AND_DEFER_POLICY.md`

## 3. Repo-Specific Customization Points

Customize only these:
- corpus names
- metric names
- toolchain commands
- phase status file locations
- benchmark roots
- decision thresholds if your org uses them

Do not customize away:
- comparison-class discipline
- admission vocabulary
- baseline provenance
- phase closure
- alternate tracking

## 4. Suggested Directory Placement

Any of these are valid:
- `protocol/engine_evolution/`
- `docs/protocol/`
- `governance/engine_protocol/`

Choose one and keep it stable.

## 5. Suggested First Repo Wiring

Add:
- one active phase plan using `PHASE_TEMPLATE.md`
- one checkpoint using `CHECKPOINT_TEMPLATE.md`
- one admitted-path ledger
- one baseline ledger
- one no-admission inventory

That is enough to start benefiting immediately.

## 6. Multi-Engine Program Use

If multiple engines share one UI or compiler layer:
- keep one protocol bundle shared across the program,
- keep per-engine ledgers local to each repo,
- keep cross-engine comparison documents in a neutral location.

This avoids tying the method to a single backend.

## 7. Why This Bundle Is Worth Keeping

The biggest loss in long-running engine work is usually not code.
It is decision integrity.

This protocol is meant to preserve that integrity so future work can move
faster without becoming less trustworthy.
