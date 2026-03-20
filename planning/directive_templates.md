# Directive Templates (Implementation + Audit)

Use these templates when handing work to a new model with zero context.

Keep directives tight.
Do not add extra scope.

## Delivery Policy

- Provide task-specific directives inline in chat by default.
- Do not create extra directive files unless explicitly requested.

---

## Template A: Implementation Directive

```md
# Implementation Directive

## Directive Identity
- Directive ID: `<id>`
- Pillar: `<pillar>`
- Task ID: `<task id>`
- Tracker File: `<path>`
- Plan File: `<path>`

## Required Reading
1. `<pillar master plan file>`
2. `<pillar-specific plan file>`
3. `<pillar-specific tracker file>`
4. `<task-specific architecture/reference docs only if required>`

## Pre-Execution Gate
1. Verify all previous tasks are fully complete.
2. Verify the current task is the next allowed task.
3. Verify there are no unresolved blockers or non-blockers carried forward.
4. If any gate fails, stop and report. Do not implement.

Definition of fully complete:
- Implementation complete = `[x]`
- Tests passed = `[x]`
- Test evidence recorded = `[x]`
- Audit passed = `[x]`
- Local commit created = `[x]`

## Task Scope
- Objective: `<exact expected outcome>`
- In scope:
  - `<item>`
  - `<item>`
- Out of scope:
  - `<item>`
  - `<item>`

## File Authority
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
1. Stay inside task intent.
2. Do not make speculative refactors.
3. Do not change governance or planning files unless explicitly authorized.
4. Use 1 tab for indentation; do not use spaces for indentation.
5. Do not leave trailing whitespace.
6. Prefer full-block replacement over fragile partial edits.
7. Do not intentionally implement future tasks early.
8. If small immediately upcoming work is naturally covered and correct, disclose it.

## Required Reporting
1. Summary of changes
2. Files touched
3. Tests executed (command + result)
4. Open blockers/non-blockers
5. Forward coverage:
   - `NONE`
   - or short list of immediately upcoming task areas covered early
6. Implementation-only effort rating:
   - `TOO_EASY`
   - `EASY`
   - `PERFECT`
   - `HARD`
   - `TOO_HARD`
7. Next handoff recommendation

## Tracker Update Policy
- Update tracker only for this task.
- Mark `[x]` only for:
  - Implementation complete
  - Tests passed
  - Test evidence recorded
  - Effort rating recorded, but only if the user provided the rating
- Do not mark audit/commit/push checkboxes in this directive.

## Completion Condition
- Implementation is complete and reported.
- Tracker implementation/tests/evidence boxes are marked `[x]`.
```

---

## Template B: Audit Directive

```md
# Audit Directive

## Directive Identity
- Directive ID: `<id>`
- Pillar: `<pillar>`
- Task ID: `<task id>`
- Tracker File: `<path>`

## Required Reading
1. `<pillar master plan file>`
2. `<pillar-specific tracker file>`
3. `<pillar-specific plan file>`
4. `<task-scope code/docs only>`

## Pre-Audit Gate
1. Verify all previous tasks are fully complete.
2. Verify target task implementation checkbox is `[x]`.
3. Verify target task tests-passed checkbox is `[x]`.
4. Verify target task test-evidence checkbox is `[x]`.
5. Verify audit-scope file list is present.
6. If any prerequisite is missing, stop and report `FAIL`.

Definition of fully complete:
- Implementation complete = `[x]`
- Tests passed = `[x]`
- Test evidence recorded = `[x]`
- Audit passed = `[x]`
- Local commit created = `[x]`

## Audit Scope
- Audit only files touched by the implementation plus required context.
- Do not expand scope unless explicitly authorized.

## Format Rules
- Use 1 tab for indentation; do not use spaces for indentation.
- Do not leave trailing whitespace.
- Prefer whole-block replacement for tracker updates.

## Audit Checks
1. Scope compliance
2. Tracker completeness before audit progression
3. Functional correctness
4. Boundary compliance
5. Test evidence validity
6. Regression risk in touched files
7. Whether any scope overlap is harmless forward coverage or a real defect

## Required Reporting
1. Verdict: `PASS`, `PASS WITH FORWARD COVERAGE`, or `FAIL`
2. Blockers
3. Non-blockers
4. Forward coverage notes
5. File-by-file findings
6. Required fixes (if `FAIL`)
7. Recommended next step

Hard rule:
- blockers force `FAIL`
- harmless forward coverage alone does not force `FAIL`

## Tracker Update Policy
- If verdict is `PASS` or `PASS WITH FORWARD COVERAGE`, mark only **Audit passed**.
- If verdict is `FAIL`, do not mark audit.
- Do not mark commit/push checkboxes in this directive.

## Completion Condition
- Audit report delivered.
- Tracker updated per verdict policy.
```

---

## Template C: Commit Directive

```md
# Commit Directive

## Directive Identity
- Directive ID: `<id>`
- Pillar: `<pillar>`
- Task ID: `<task id>`
- Tracker File: `<path>`

## Pre-Commit Gate
1. Verify implementation checkbox is `[x]`.
2. Verify tests-passed checkbox is `[x]`.
3. Verify test-evidence checkbox is `[x]`.
4. Verify audit-passed checkbox is `[x]`.
5. Verify all previous tasks remain fully complete.
6. If any prerequisite is incomplete, stop and report.

Definition of fully complete:
- Implementation complete = `[x]`
- Tests passed = `[x]`
- Test evidence recorded = `[x]`
- Audit passed = `[x]`
- Local commit created = `[x]`

## Commit Scope
- Commit only files belonging to the task scope.
- Do not include unrelated changes.

## Required Reporting
1. Commit hash
2. Commit message
3. Files included
4. Whether push was completed

## Tracker Update Policy
- Mark **Local commit created** with timestamp and hash.
- Mark **GitHub push complete** only if push actually happened.
```
