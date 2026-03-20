# Directive Templates (Implementation + Audit)

Use these templates when handing work to a new model with zero context.

Keep each directive tight.
Do not include extra scope.

## Delivery Policy

- By default, directive drafts are provided inline in chat inside a copy/paste
  code block.
- Do not create task-specific directive files unless explicitly requested by
  the user.

---

## Template A: Implementation Directive

Copy and fill this template for each implementation task.

```md
# Implementation Directive

## Directive Identity
- Directive ID: `<id>`
- Pillar: `<pillar>`
- Task ID: `<task id>`
- Tracker File: `<path>`
- Plan File: `<path>`

## Required Reading (Must Read In Order)
1. `<pillar master plan file>`
2. `<pillar-specific plan file>`
3. `<pillar-specific tracker file>`
4. `<pillar-specific architecture/reference docs only if required>`

## Pre-Execution Gate (Required)
1. Verify all previous tasks are marked complete in the tracker.
2. If any previous task is incomplete, stop and report. Do not implement.
3. Confirm this directive scope matches the current task ID.

## Task Scope
- Objective: `<exact expected outcome>`
- In scope:
  - `<item>`
  - `<item>`
- Out of scope:
  - `<item>`
  - `<item>`

## File Authority (Strict)
- Allowed to read:
  - `<path>`
  - `<path>`
- Allowed to edit:
  - `<path>`
  - `<path>`
- Allowed to create:
  - `<path>`
- Allowed to delete:
  - `<path or NONE>`
- Forbidden to touch:
  - all files not explicitly listed above

## Execution Rules
1. Follow project intent, not internal model preferences.
2. Stay inside this directive scope only.
3. Do not make speculative refactors.
4. Do not change governance or planning files unless explicitly authorized.
5. Keep implementation effort sized to the task objective only.

## Required Reporting Policy
At completion, report only:
1. Summary of changes
2. Files touched
3. Tests executed (command + result)
4. Open blockers/non-blockers
5. Implementation-only effort rating:
   - `TOO_EASY`
   - `EASY`
   - `PERFECT`
   - `HARD`
   - `TOO_HARD`
6. Next handoff recommendation

## Tracker Update Policy
- Update tracker only for this task.
- Mark **Implementation complete** as `[x]` when implementation is done.
- Mark **Tests passed (current + previous = 100%)** as `[x]` after test proof.
- Mark **Test evidence recorded** as `[x]` after command/result evidence is
  entered.
- Mark **Effort rating recorded** as `[x]` after the implementation-only
  rating is entered.
- Do not mark audit/commit checkboxes in this directive.

## Completion Condition
- Implementation is complete and reported.
- Tracker implementation + tests passed + test evidence + effort rating
  checkboxes for this task are marked `[x]`.
```

---

## Template B: Audit Directive

Copy and fill this template for each task audit.

```md
# Audit Directive

## Directive Identity
- Directive ID: `<id>`
- Pillar: `<pillar>`
- Task ID: `<task id>`
- Tracker File: `<path>`

## Optional Context Inputs
- Prior implementation report (if available)
- Prior record-correction note (if available)

## Required Reading (Must Read In Order)
1. `<pillar master plan file>`
2. `<pillar-specific tracker file>`
3. `<pillar-specific plan file>`
4. `<task-scope code/docs only>`

## Pre-Execution Gate (Required)
1. Verify target task implementation checkbox is `[x]`.
2. Verify previous required task documentation is complete in the tracker
   before auditing.
3. Verify audit scope file list is present.
4. If any prerequisite is missing, stop and report `FAIL`.

## Audit Scope (Strict)
- Audit only files touched by prior implementation:
  - `<path>`
  - `<path>`
- Do not expand scope beyond this file list unless the directive explicitly
  authorizes required architecture context.

## File Authority (Strict)
- Allowed to read:
  - listed audit-scope files
  - referenced planning/architecture files
- Allowed to edit:
  - `NONE` unless explicitly authorized in this directive
- Allowed to create:
  - `NONE` unless explicitly authorized in this directive
- Allowed to delete:
  - `NONE`

## Audit Checks
1. Scope compliance
2. Tracker completeness compliance before audit progression
3. Functional correctness for the task objective
4. Boundary compliance
5. Test evidence validity
6. Regression risk in touched files

## Required Reporting Policy
Report using this exact structure:
1. Verdict: `PASS` or `FAIL`
2. Blockers (must be empty for `PASS`)
3. Non-blockers (must be empty for `PASS`)
4. File-by-file findings
5. Required fixes (if `FAIL`)
6. Recommended next step

## Tracker Update Policy
- Do not alter implementation checkbox state.
- If verdict is `PASS`, mark only the **Audit passed** checkbox for this task
  as `[x]`.
- If verdict is `FAIL`, do not mark audit checkbox.

Hard rule:
- If any blocker or non-blocker is reported, verdict must be `FAIL`.

## Completion Condition
- Audit report delivered.
- Tracker updated per verdict policy.
```

---

## Template C: Commit Directive

Copy and fill this template when a task has passed audit and is ready to be
committed.

```md
# Commit Directive

## Directive Identity
- Directive ID: `<id>`
- Pillar: `<pillar>`
- Task ID: `<task id>`
- Tracker File: `<path>`

## Pre-Commit Gate (Required)
1. Verify implementation checkbox is `[x]`.
2. Verify tests-passed checkbox is `[x]`.
3. Verify test-evidence checkbox is `[x]`.
4. Verify audit-passed checkbox is `[x]`.
5. If any prerequisite is incomplete, stop and report.

## Commit Scope
- Commit only files belonging to the task scope.
- Do not include unrelated changes.

## Required Reporting Policy
Report only:
1. Commit hash
2. Commit message
3. Files included
4. Whether push was completed

## Tracker Update Policy
- Mark **Local commit created** as `[x]` with timestamp and hash.
- Mark **GitHub push complete** only if push actually happened.
```
