# Strata Planning System

Purpose
- Explain how the pillar-based planning and execution system works.
- Keep the build process understandable without requiring a reader to infer it
  from multiple files.

## Planning Entry Points

Start here:
1. `planning/pillar_master_plan.md`
2. `planning/pillar_<N>_plan.md`
3. `planning/pillar_<N>_tracker.md`
4. `planning/directive_templates.md`

Historical planning material:
- `planning/legacy/`

## Core Build Process

Strata now executes work by pillars.

Each pillar is a build layer that:
- has a clear goal
- is broken into tasks
- depends on the pillars before it

Each task follows this lifecycle:
1. implementation
2. tests
3. audit
4. commit

Hard rule:
- no task may begin unless all previous tasks are complete

## File Roles

### `pillar_master_plan.md`
This is the top-level roadmap.

It defines:
- all project pillars
- completed pillars
- the next active pillar
- future pillars at summary level
- effort model for pillar sizing

### `pillar_<N>_plan.md`
This defines one pillar in detail.

It contains:
- pillar goal
- effort score
- task count
- task list

Important rule:
- only the next pillar in line should be fully detailed

### `pillar_<N>_tracker.md`
This is the execution gate.

It records:
- implementation completion
- tests passed
- test evidence
- audit passed
- local commit
- push completion
- implementation-only effort rating

No task should proceed out of order.

### `directive_templates.md`
This contains the working templates for:
- implementation directives
- audit directives
- commit directives

Top-level template files also exist for constrained external-agent usage:
- `TASK_DIRECTIVE_TEMPLATE.md`
- `TASK_AUDIT_TEMPLATE.md`
- `COMPLIANCE_AUDIT_TEMPLATE.md`

## Effort Model

Planning-time sizing:
- pillar effort score
- target implementation-only task difficulty = `3`
- task count derived from score and adjusted to a practical whole number

Post-task rating:
- `TOO_EASY`
- `EASY`
- `PERFECT`
- `HARD`
- `TOO_HARD`

Important rule:
- effort rating applies only to implementation work
- audit and commit work are not part of the difficulty score

## Pillar Refinement Rule

The whole project should have a stable pillar map.

But only the next pillar should be refined in detail.

Reason:
- plans change based on current state
- future pillars should not be over-specified too early

This means:
- the master plan stays broad for future pillars
- the next pillar gets the detailed plan and tracker
- later pillars are refined only when they become next

## Legacy Material

Older planning documents that remain useful for history or provenance should be
kept under:
- `planning/legacy/`

They should not remain mixed with active planning truth.
