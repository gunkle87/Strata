# Task Audit Template

STRICT TASK AUDIT DIRECTIVE
(READ-ONLY, NO EXECUTION-DRIFT, TASK-LOCKED)

---

STATE RESET REQUIREMENT:

Ignore all prior conversational context.

This directive is fully self-contained and authoritative.

Only the information provided below defines scope.

---

OBJECTIVE:

Audit one completed task exactly as implemented.

The goal is to determine whether the task:
- satisfies its stated intent
- stays within acceptable scope
- preserves architectural and phase boundaries
- has complete tracker evidence before commit
- distinguishes real defects from harmless forward coverage

---

TASK CONTEXT:

- Pillar: [PILLAR]
- Task ID: [TASK XX]
- Plan File: [PLAN FILE PATH]
- Tracker File: [TRACKER FILE PATH]

Audit Scope Files:
[PASTE FILE LIST]

FORMAT AND MATCHING RULES:

- Use 1 tab for all indentation levels; do not use spaces for indentation.
- No trailing whitespace is allowed on any line.
- When a tracker update is required, prefer replacing the full task block or
  known section rather than attempting whitespace-sensitive surgical edits.
- Prefer whole-block overwrite behavior over regex-based shell edits when a
  structured block replacement is practical.

---

PRE-AUDIT GATE (REQUIRED):

Before auditing, you MUST verify in the tracker:
1. all previous tasks are fully complete
2. the target task implementation checkbox is complete
3. the target task tests-passed checkbox is complete
4. the target task test-evidence checkbox is complete

Definition of "fully complete" for every previous task:
- Implementation complete = `[x]`
- Tests passed = `[x]`
- Test evidence recorded = `[x]`
- Audit passed = `[x]`
- Local commit created = `[x]`

Hard gate:
- If any previous task is missing its local commit checkbox, this audit is
  INVALID and MUST STOP.

If any gate fails:
- STOP
- return FAIL
- do not continue the audit

---

AUDIT SCOPE LOCK:

You MUST:
- audit ONLY the task scope and required references
- verify tracker evidence before judging the task ready for commit

You MUST NOT:
- expand into future tasks
- suggest unrelated improvements
- broaden scope beyond what is needed to judge this task

---

REQUIRED CHECKS:

1. previous-task completion gate verified
2. task-scope compliance
3. functional correctness for task intent
4. architectural boundary preservation
5. placeholder vs real honesty
6. test evidence validity
7. tracker completeness for pre-commit readiness

FORWARD COVERAGE RULE:

If the implementation includes work that naturally reaches into the immediately
upcoming task or tasks, the audit MUST distinguish between:
- harmful scope drift
- harmless forward coverage

Harmless forward coverage means ALL of the following are true:
- the code is functionally correct
- the code does not violate architecture or phase boundaries
- the code does not make false claims about completed behavior
- the code belongs to an immediately upcoming planned task
- the code does not make later work harder or more confusing

If all of the above are true:
- do NOT require rollback
- do NOT classify this alone as FAIL
- record it explicitly as forward coverage

---

TRACKER UPDATE REQUIREMENT:

After audit, update the tracker for this task only.

If verdict is PASS:
- mark Audit passed

If verdict is FAIL:
- do NOT mark Audit passed

You MUST NOT mark:
- Local commit created
- GitHub push complete

---

STRICT PROHIBITIONS:

- NO code changes
- NO fix proposals in place of findings
- NO future-task design work
- NO scope expansion
- NO mixed indentation styles in any tracker or template update
- NO trailing whitespace

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
- If any blocker exists, verdict must be FAIL
- Non-blockers that are only harmless forward coverage do NOT force FAIL
- PASS WITH FORWARD COVERAGE is allowed only when there are:
  - no blockers
  - no architectural violations
  - no correctness failures

---

FAIL CONDITIONS (INVALID OUTPUT):

- previous-task tracker gate not checked
- previous-task local commit gate not checked
- tracker evidence not checked before audit
- scope expansion
- missing blockers/non-blockers structure
- harmless forward coverage incorrectly forced to FAIL without a real defect
- audit pass box marked despite findings
- non-English output

---

CONSTRAINT:

- Audit only this task
- Do not modify implementation
- Do not anticipate future work

---

ENFORCEMENT:

If any rule is violated:
-> STOP
-> OUTPUT IS INVALID
