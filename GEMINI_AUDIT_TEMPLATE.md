STATIC IMPLEMENTATION AUDIT DIRECTIVE (READ-ONLY, NO EXECUTION)

NOTE:
- This file is an audit prompt template only.
- It is not repository governance.
- It does not override `RULES.md` or any project plan.

OBJECTIVE:
Audit the current static state of [REPOSITORY ROOT PATH] for internal consistency across a defined implementation slice.

The audit MUST evaluate whether the current repository contents are:

1. Internally coherent across the selected implementation surfaces
2. Consistent across adjacent system boundaries
3. Aligned with governing documentation
4. Free of observable drift between implementation, tests, and specifications

The audit MUST operate strictly on the current snapshot and determine whether all participating components agree with each other NOW.

---

TARGET IMPLEMENTATION SLICE:
[Describe the exact slice under audit]

Example:
- Breadboard real fast-path lowering
- Breadboard → Forge handoff
- Artifact serialization and load path
- Test validation layer for the above

---

CONSISTENCY AXES (MANDATORY EVALUATION):

Evaluate agreement across ALL of the following:

- Compile / lowering behavior
- Data structures and API contracts
- Serialization / artifact representation
- Boundary handoff behavior between subsystems
- Test expectations and assertions
- Governing architectural and planning documents

The audit MUST detect:
- mismatches between layers
- inconsistencies between producer and consumer components
- divergence between tests and implementation
- deviation from documented contracts

---

SCOPE:

Audit ONLY the following files:

IMPLEMENTATION FILES:
- [FILE PATH]
- [FILE PATH]
- [FILE PATH]

TEST FILES:
- [FILE PATH]
- [FILE PATH]

GOVERNANCE / SPEC DOCUMENTS:
- [DOC PATH]
- [DOC PATH]
- [DOC PATH]

No files outside this scope may be considered unless explicitly referenced by the listed files.

---

STRICT PROHIBITIONS:

EDITING / MODIFICATION:
- DO NOT modify any files
- DO NOT propose or apply patches
- DO NOT simulate edits or hypothetical changes
- DO NOT suggest fixes, refactors, or improvements

EXECUTION (ABSOLUTE BAN):
- DO NOT execute ANY commands under ANY circumstances
- This includes BOTH:
  - state-changing commands
  - read-only commands

Explicitly forbidden (non-exhaustive):
- git grep, grep, rg, find, search commands
- git status, git log, git diff, git show, git blame
- build, test, run, compile, or script execution
- any shell, CLI, IDE task, or tool invocation

- DO NOT use any tool that requires command execution to access data
- ALL analysis MUST be performed via direct file reading only

HISTORY / EVOLUTION:
- DO NOT access or analyze git history, commits, diffs, branches, or PRs
- DO NOT infer historical intent beyond what is explicitly present in current files
- Treat the repository as a static snapshot

---

ALLOWED ACTIONS:

- Read files directly
- Analyze current code, headers, and tests
- Cross-reference implementation against documentation
- Compare implementation components against each other
- Validate producer/consumer agreement across boundaries
- Validate test expectations against implementation behavior

---

OUTPUT REQUIREMENTS:

Produce a structured audit containing ONLY the following sections:

1. OVERALL SUMMARY
- Concise assessment of whether the implementation slice is:
  - coherent
  - partially coherent
  - or materially inconsistent

2. FINDINGS

For each finding include:

- Title
- Classification:
  - COMPLIANT
  - NON-COMPLIANT
  - AMBIGUOUS
- Governance Reference:
  - [document + section]
- Implementation Reference:
  - [file + function / region / line range]
- Discrepancy:
  - clear description of mismatch or alignment
- Evidence:
  - direct reference-based justification

3. CROSS-LAYER CONSISTENCY NOTES
- Explicitly identify whether:
  - Breadboard ↔ Forge alignment holds
  - Serialization ↔ Load interpretation matches
  - Tests ↔ Implementation agree
  - Docs ↔ Code match

4. FINAL ASSESSMENT
- Summarize only high-confidence conclusions
- Clearly separate:
  - confirmed mismatches
  - unresolved ambiguities
Provid the report in a text box using only ASCII formatting.



---

CLASSIFICATION RULES:

COMPLIANT:
- Clear agreement across all relevant surfaces

NON-COMPLIANT:
- Clear contradiction between implementation layers or against documentation

AMBIGUOUS:
- Insufficient direct evidence to confirm alignment

---

EVIDENCE STANDARD:

- ALL conclusions MUST be tied to explicit file or document references
- DO NOT speculate
- DO NOT infer behavior not directly visible in the code
- If evidence is incomplete → classify as AMBIGUOUS

---

CONSTRAINT:

This is a static implementation audit only.

- No history
- No edits
- No execution

---

ENFORCEMENT:

Violation of ANY execution or modification rule INVALIDATES the entire audit.

If any prohibited action occurs:
- STOP immediately
- Report audit as INVALID
