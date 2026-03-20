# Task Audit Template

STRICT TASK AUDIT DIRECTIVE
(READ-ONLY, NO EXECUTION-DRIFT, TASK-LOCKED)

---

STATE RESET:

Ignore all prior conversational context.

This directive is self-contained and authoritative.
Only the content below defines scope.

---

OBJECTIVE:

Audit one completed task exactly as implemented.

Determine whether the task:
- satisfies task intent
- stays within acceptable scope
- preserves architecture and phase boundaries
- has complete tracker evidence before commit
- contains any harmless forward coverage

---

TASK CONTEXT:

- Pillar: [PILLAR]
- Task ID: [TASK XX]
- Plan File: [PLAN FILE PATH]
- Tracker File: [TRACKER FILE PATH]

Audit Scope Files:
[PASTE FILE LIST]

Format and matching rules:
- use 1 tab for indentation; do not use spaces for indentation
- no trailing whitespace on any line
- when a tracker update is required, prefer whole-block replacement over
  whitespace-sensitive partial edits

---

PRE-AUDIT GATE:

Before auditing, verify in the tracker:
1. all previous tasks are fully complete
2. target task implementation checkbox is complete
3. target task tests-passed checkbox is complete
4. target task test-evidence checkbox is complete

Definition of fully complete:
- Implementation complete = `[x]`
- Tests passed = `[x]`
- Test evidence recorded = `[x]`
- Audit passed = `[x]`
- Local commit created = `[x]`

Hard gate:
- if any previous task is missing its local commit checkbox, this audit is
  locked and must not proceed

If any gate fails:
- STOP
- return FAIL
- do not continue

---

AUDIT SCOPE LOCK:

You MUST:
- audit only the task scope and required references
- verify tracker evidence before judging pre-commit readiness

You MUST NOT:
- expand into future tasks unnecessarily
- suggest unrelated improvements
- broaden scope beyond what is needed to judge this task

---

REQUIRED CHECKS:

1. previous-task completion gate verified
2. previous-task local commit gate verified
3. task-scope compliance
4. functional correctness for task intent
5. architectural boundary preservation
6. placeholder vs real honesty
7. test evidence validity
8. tracker completeness for pre-commit readiness

---

FORWARD COVERAGE RULE:

Distinguish between:
- harmful scope drift
- harmless forward coverage

Harmless forward coverage means all of the following are true:
- the code is functionally correct
- the code does not violate architecture or phase boundaries
- the code does not make false claims
- the code belongs to an immediately upcoming planned task
- the code does not make later work harder or more confusing

If all are true:
- do not require rollback
- do not fail the audit for that reason alone
- record it explicitly as forward coverage

---

TRACKER RULE:

After audit, update this task only.

If verdict is `PASS` or `PASS WITH FORWARD COVERAGE`:
- mark Audit passed

If verdict is `FAIL`:
- do not mark Audit passed

Do NOT mark:
- Local commit created
- GitHub push complete

---

STRICT PROHIBITIONS:

- no code changes
- no future-task design work
- no scope expansion
- no mixed indentation styles in any tracker update
- no trailing whitespace

---

OUTPUT FORMAT:

1. Verdict: PASS, PASS WITH FORWARD COVERAGE, or FAIL
2. Blockers
3. Non-blockers
4. Forward coverage notes
5. File-by-file findings
6. Tracker gate verification
7. Tracker update performed: YES/NO
8. Recommended next step

Hard rule:
- if any blocker exists, verdict must be FAIL
- non-blockers that are only harmless forward coverage do not force FAIL

---

INVALID OUTPUT IF:

- previous-task tracker gate was not checked
- previous-task local commit gate was not checked
- tracker evidence was not checked before audit
- scope expanded without need
- blockers/non-blockers structure is missing
- harmless forward coverage was forced to FAIL without a real defect
- audit pass was marked despite blockers
- output is not in English

---

CONSTRAINT:

Audit only this task.
Do not modify implementation.
Do not anticipate future work.

---

ENFORCEMENT:

If any rule is violated:
-> STOP
-> OUTPUT IS INVALID
