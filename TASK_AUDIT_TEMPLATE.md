TASK SLICE COMPLIANCE AUDIT DIRECTIVE
(READ-ONLY, NO EXECUTION, PHASE-AWARE)

---

STATE RESET REQUIREMENT:

Ignore all prior conversational context unless explicitly restated below.

Treat this request as a fully self-contained execution.

Only the information in this directive is authoritative.

---

OBJECTIVE:

Audit the implementation of a single completed task slice for correctness,
coherence, and alignment with both:

1. Task intent (what this slice was supposed to do)
2. System architecture (architectural truth)
3. Current execution phase constraints (phase truth)

This is a slice audit — NOT a repo-wide audit.

---

TASK CONTEXT (MANDATORY INPUT):

- Pillar: [PILLAR NAME / DOCUMENT]
- Task ID: [TASK XX]
- Commit / Snapshot: [COMMIT ID OR STATE]

Task Intent (authoritative):
[PASTE TASK DIRECTIVE OR SUMMARY]

---

GOVERNING DOCUMENT TIERS:

TIER 1 — ARCHITECTURAL TRUTH:
- [ARCHITECTURE PLAN PATH]

TIER 2 — PHASE TRUTH:
- [EXECUTION PLAN PATH]
- [PHASE DOCS]

TIER 3 — TASK TRUTH:
- Task definition above (PRIMARY for this audit)

RULE PRIORITY:
- Task intent defines what MUST be implemented
- Phase defines what is allowed
- Architecture defines what should ultimately be true

---

MANDATORY SOURCE VERIFICATION (REQUIRED FIRST STEP):

Before auditing, you MUST:

1. List all files and documents actually read
2. For each:
   - path
   - sections or regions used
3. Provide at least one quote or precise paraphrase

If missing:
→ OUTPUT IS INVALID

---

AUDIT QUESTIONS (MANDATORY):

Evaluate ALL of the following:

1. Task Intent Fidelity
   - Was the task implemented exactly as defined?

2. Slice Completeness
   - Is the slice complete for its intended scope?

3. Boundary Discipline
   - Did this slice stay within its intended layer?

4. Cross-Layer Consistency
   - Do all touched layers agree with each other?

5. Artifact / Data Contract Integrity
   - Do produced structures match expected consumers?

6. Test Alignment
   - Do tests validate the correct behavior through public APIs?

7. Scope Control
   - Did the slice avoid expanding beyond task intent?

8. Placeholder vs Real Behavior
   - Are temporary constructs correctly represented and contained?

---

PHASE-AWARE CLASSIFICATION:

Use ONLY:

- COMPLIANT
  = correct for task, architecture, and phase

- PHASE-COMPLIANT
  = violates architecture BUT explicitly allowed by phase

- NON-COMPLIANT
  = violates task intent OR architecture without phase justification

- AMBIGUOUS
  = insufficient evidence

---

CRITICAL CLASSIFICATION RULES:

- If task intent is violated → NON-COMPLIANT (always)
- If architecture is violated but phase allows → PHASE-COMPLIANT
- If neither doc proves behavior → AMBIGUOUS

---

STRICT PROHIBITIONS:

EDITING:
- DO NOT modify files
- DO NOT suggest fixes
- DO NOT propose improvements

EXECUTION:
- DO NOT run ANY commands
- NO grep, rg, git, shell, scripts

HISTORY:
- DO NOT analyze commits, diffs, or evolution

INFERENCE CONTROL:
- DO NOT assume intent
- DO NOT invent missing behavior
- If unclear → AMBIGUOUS

---

ALLOWED ACTIONS:

- Read files directly
- Analyze code and tests
- Cross-reference documentation
- Compare expected vs actual behavior

---

OUTPUT REQUIREMENTS:

1. TASK SUMMARY
- Restate task intent (brief, accurate)
- Confirm scope of slice

---

2. FINDINGS

For EACH finding:

- Title
- Classification:
  - COMPLIANT
  - PHASE-COMPLIANT
  - NON-COMPLIANT
  - AMBIGUOUS
- Task Reference (what was expected)
- Architecture Reference (if applicable)
- Phase Reference (if applicable)
- Implementation Reference (file + location)
- Discrepancy
- Evidence

---

3. SLICE INTEGRITY

Explicitly confirm:

- Slice completeness
- Boundary adherence
- No unintended side effects
- No cross-layer contamination

---

4. TEST VALIDATION

- Are tests:
  - aligned with task intent?
  - using correct public boundaries?
- Any gaps between tests and actual behavior?

---

5. DRIFT CHECK (TASK LEVEL)

- Did this task introduce:
  - architectural drift?
  - hidden coupling?
  - premature capability expansion?

Classify each as:
- none
- phase-justified
- concerning

---

6. FINAL ASSESSMENT

Clearly separate:

- Confirmed NON-COMPLIANT issues
- PHASE-COMPLIANT allowances
- AMBIGUITIES
- Confirmed correct implementations

---

EVIDENCE STANDARD:

- Every claim MUST include:
  - file + location
  - document reference

- No speculation
- No unsupported claims

---

CONSTRAINT:

- Static snapshot only
- No execution
- No edits
- No history

---

ENFORCEMENT:

Output is INVALID if:

- no source verification
- missing references
- incorrect classification usage
- speculative reasoning
- command/tool usage
- non-English output