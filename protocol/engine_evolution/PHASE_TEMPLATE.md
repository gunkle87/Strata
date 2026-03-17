# PHASE TEMPLATE

Use this template to start a new phase.

## Title
- phase name

## Layer In Scope
- one primary layer:
  - semantic
  - compiler or breadboard
  - runtime
  - benchmark or harness
  - control plane

## Purpose
- one short paragraph describing the question this phase exists to answer

## Why This Phase Exists Now
- what current bottleneck, gap, or architectural need justifies this phase

## Protected Baseline
- baseline identifier:
- comparison class:
- corpus:
- settings:
- why this is the correct baseline:

## Phase Hypothesis
- state the expected mechanism of improvement
- state why it should help this layer specifically

## In Scope
- list the changes this phase is allowed to make

## Out Of Scope
- list what this phase must not change

## Candidate Families
- each candidate should define:
  - name
  - shape or mechanism
  - expected benefit
  - legality boundaries
  - obvious failure cases

## Validation Sequence
1. unit or fixture validation
2. parser-backed or realistic validation
3. report-only screening if applicable
4. protected comparison run
5. confirmation rerun if required

## Success Criteria
- what result would justify continuing or admitting

## Failure Criteria
- what result stops the phase or forces deferral

## Deliverables
- required code outputs
- required docs
- required ledgers or status files

## Exit Conditions
- what must be true before this phase is considered complete

## Re-entry Triggers
- when this phase should be revisited later if it does not admit now
