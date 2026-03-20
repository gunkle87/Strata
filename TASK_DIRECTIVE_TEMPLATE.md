STRICT IMPLEMENTATION DIRECTIVE
(NO DRIFT, NO EXPANSION, TASK-LOCKED)

---

STATE RESET:

Ignore all prior conversational context.

This directive is self-contained and authoritative.
Only the content below defines scope.

---

OBJECTIVE:

Implement the specified task exactly as defined.

Deliver the smallest correct implementation that:
- satisfies task intent
- stays within allowed scope
- preserves architecture and phase constraints

---

COMPACT REPOSITORY RULES:

Core doctrine:
- structure is the authoring layer
- function is the execution layer
- structural translation is not execution

Layer ownership:
- Breadboard owns structural compilation and lowering
- Forge owns shared runtime boundary and capability discovery
- Backends own native execution truth
- Tests must use public interfaces only

Hard boundaries:
- do not blur layers
- do not move runtime logic into compile layers or vice versa
- do not reach into backend internals from tests, tools, or UI
- all runtime interaction goes through Forge

Clean-room:
- all code must be original

Formatting:
- opening brace on a new line
- use 1 tab for indentation; do not use spaces for indentation
- no trailing whitespace on any line

If any task instruction conflicts with these rules:
- STOP
- state the conflict plainly

---

TASK CONTEXT:

- Pillar: [PILLAR]
- Task ID: [TASK XX]
- Plan File: [PLAN FILE PATH]
- Tracker File: [TRACKER FILE PATH]

Task Directive:
[PASTE TASK DIRECTIVE]

---

PRIORITY:

1. Task intent
2. Phase constraints
3. Architecture

---

PRE-EXECUTION GATE:

Before implementation, verify in the tracker:
1. all previous tasks are fully complete
2. the current task is the next allowed task
3. there are no unresolved blockers or non-blockers carried forward

Definition of fully complete:
- Implementation complete = `[x]`
- Tests passed = `[x]`
- Test evidence recorded = `[x]`
- Audit passed = `[x]`
- Local commit created = `[x]`

Hard gate:
- if any previous task is missing its local commit checkbox, the next task is
  locked and must not begin

If any gate fails:
- STOP
- report the gate failure
- do not implement

---

SCOPE LOCK:

You MUST:
- implement only what is explicitly required
- touch only files necessary for this task
- preserve existing behavior unless the task requires change

You MUST NOT:
- anticipate future tasks intentionally
- expand features
- refactor unrelated code
- improve design beyond requirements

Forward coverage exception:
- if a small amount of immediately upcoming work is naturally coupled and
  implemented correctly, it may remain
- if that happens, disclose it explicitly
- do not use this to justify broad speculative work

---

PRE-IMPLEMENTATION DECLARATION:

Before writing code, state:
1. files to modify or create
2. why each file is needed
3. what will change in each file

No code before this step.
If unclear:
- STOP
- request clarification

---

EDIT RELIABILITY RULE:

When changing a known block, function, or tracker section:
- prefer replacing the whole block
- prefer structured block overwrite over regex-style shell edits
- avoid fragile whitespace-sensitive partial replacements when a full block
  replacement is practical

---

TEST RULE:

- modify or add tests only if the task requires it
- tests must validate through public interfaces only

---

TRACKER RULE:

After implementation, update this task only.

Mark `[x]` only when true:
- Implementation complete
- Tests passed (current + previous = 100%)
- Test evidence recorded

Do NOT mark:
- Audit passed
- Local commit created
- GitHub push complete

Effort rating:
- record it only if the user provided it

---

POST-IMPLEMENTATION REPORT:

Provide:
1. summary of what was implemented
2. confirmation that scope was respected
3. confirmation that boundaries were preserved
4. list of files changed
5. list of functions added or modified
6. forward coverage disclosure:
   - `NONE`
   - or the immediately upcoming task areas covered early
7. confirmation that tracker boxes were updated correctly

---

OUTPUT FORMAT:

If the gate fails, output only:

GATE FAILURE
- Previous task completion status: PASS/FAIL
- Previous task local commit status: PASS/FAIL
- Current task unlocked: YES/NO
- Implementation started: YES/NO

If the gate passes, use:

--- PRE-IMPLEMENTATION PLAN ---
[files + changes]

--- IMPLEMENTATION ---
[code]

--- POST-IMPLEMENTATION SUMMARY ---
[what was done]

--- FORWARD COVERAGE ---
[none or explicit list]

--- VALIDATION ---
- Scope respected: YES/NO
- Boundaries preserved: YES/NO
- Extra changes introduced: YES/NO
- Tracker updated correctly: YES/NO

---

INVALID OUTPUT IF:

- code appears before the plan
- scope expands without disclosure
- files changed are not listed
- architectural boundaries are violated
- speculative additions are introduced
- the tracker gate was not checked first
- the previous-task commit gate was ignored
- the tracker was not updated after implementation
- output is not in English

---

CONSTRAINT:

Implement only this task.
Do not redesign the system.

---

ENFORCEMENT:

If any rule is violated:
-> STOP
-> OUTPUT IS INVALID
