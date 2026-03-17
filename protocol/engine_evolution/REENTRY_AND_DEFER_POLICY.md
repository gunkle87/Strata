# Re-Entry And Defer Policy

Purpose
- This file defines how to stop work cleanly without losing future value.

## 1. Defer Is Not Forget

Deferred work must leave behind enough structure that a future contributor can
resume it without reconstructing the entire history.

## 2. Allowed Deferred States

- `HOLD FOR LATER`
  - promising but not timely
- `KEEP ALTERNATE`
  - useful branch retained outside the live path
- `KEEP KERNEL, DEFER`
  - foundational work kept, broader deployment deferred
- `REJECT CHANGE`
  - not worth carrying forward unless new evidence emerges

## 3. Required Re-Entry Record

Deferred work should record:
- what it tried to do
- why it stopped
- what blocked it
- what would justify trying again
- what baseline it was last compared against

## 4. Good Re-Entry Triggers

- a new baseline changes the tradeoff
- a stronger kernel becomes available
- attribution data points to the same bottleneck again
- another engine needs the same capability at a cleaner layer
- a prior blocker has been removed

## 5. Bad Re-Entry Triggers

- frustration with current work
- sunk-cost pressure
- vague hope that one more tweak will save the branch
- inability to remember why the branch was deferred

## 6. Recovery Versus Re-Entry

Use `recovery` when:
- the current live path regressed and needs consolidation

Use `re-entry` when:
- a previously deferred idea becomes justified again

These should not be conflated.
