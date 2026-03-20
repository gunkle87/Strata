STATIC ARCHITECTURAL COMPLIANCE AUDIT DIRECTIVE
(READ-ONLY, NO EXECUTION, PHASE-AWARE)

---

STATE RESET REQUIREMENT:

Ignore all prior conversational context unless explicitly restated below.

Treat this request as a fully self-contained execution.

Only the information in this directive is authoritative.

---

OBJECTIVE:

Perform a full architectural compliance audit of the current static state of:

[REPOSITORY ROOT PATH]

The audit must determine whether the repository is aligned with:

1. The intended architecture (architectural truth)
2. The current execution phase (phase truth)

This is a repo-wide audit, not a task-slice audit.

---

GOVERNING DOCUMENT TIERS:

TIER 1 — ARCHITECTURAL TRUTH (PRIMARY AUTHORITY):
- [ARCHITECTURE PLAN PATH]

TIER 2 — PHASE TRUTH (CONSTRAINT AUTHORITY):
- [EXECUTION PLAN PATH]
- [PHASE-SPECIFIC DOCS]

RULE:
- Tier 1 defines what the system SHOULD be
- Tier 2 defines what is TEMPORARILY ALLOWED

---

MANDATORY SOURCE VERIFICATION (REQUIRED FIRST STEP):

Before producing the audit, you MUST:

1. List each document actually read
2. For each document:
   - path
   - sections used
3. Provide at least one direct quote or precise paraphrase

If this step is missing:
? OUTPUT IS INVALID

---

AUDIT QUESTIONS (MANDATORY):

Evaluate ALL of the following:

1. Layering integrity
2. Ownership boundaries
3. Artifact boundary discipline
4. Public-boundary enforcement
5. Scope discipline
6. Documentation ? code agreement
7. Real vs placeholder honesty
8. Architectural drift indicators

---

PHASE-AWARE CLASSIFICATION (CRITICAL):

You MUST classify findings using:

- COMPLIANT
  = aligns with architecture AND phase

- PHASE-COMPLIANT
  = violates pure architecture BUT explicitly allowed by phase plan

- NON-COMPLIANT
  = violates architecture AND NOT justified by phase plan

- AMBIGUOUS
  = insufficient evidence

---

CLASSIFICATION RULES:

A finding MUST be:

PHASE-COMPLIANT if:
- it contradicts Tier 1 (architecture)
AND
- is explicitly permitted or described in Tier 2 (phase)

If not explicitly permitted:
? it is NOT phase-compliant

---

STRICT PROHIBITIONS:

EDITING:
- DO NOT modify files
- DO NOT propose fixes
- DO NOT suggest improvements

EXECUTION (ABSOLUTE BAN):
- DO NOT execute ANY commands
- DO NOT use grep, rg, git, shell, scripts
- DO NOT use tools requiring command execution

HISTORY:
- DO NOT use commits, diffs, branches, PRs
- DO NOT reconstruct evolution

INFERENCE CONTROL:
- DO NOT invent missing behavior
- DO NOT assume intent
- If unclear ? AMBIGUOUS

---

ALLOWED ACTIONS:

- Read files directly
- Analyze code and docs
- Cross-reference layers
- Compare responsibilities

---

OUTPUT REQUIREMENTS:

1. OVERALL SUMMARY
- architecturally coherent / partially coherent / inconsistent
- MUST distinguish:
  - architectural alignment
  - phase alignment

---

2. FINDINGS

For EACH finding:

- Title
- Classification:
  - COMPLIANT
  - PHASE-COMPLIANT
  - NON-COMPLIANT
  - AMBIGUOUS
- Architecture Reference (Tier 1)
- Phase Reference (Tier 2, if applicable)
- Implementation Reference (file + location)
- Discrepancy
- Evidence (must be explicit)

---

3. LAYER COMPLIANCE

Explicitly evaluate:

- Breadboard
- Forge
- Artifact boundary
- Public boundary
- Placeholder vs real
- Documentation alignment

---

4. ARCHITECTURAL DRIFT ASSESSMENT

- Confirmed drift
- Phase-justified deviations
- Fragile boundaries
- Potential future drift risks

---

5. FINAL ASSESSMENT

Clearly separate:

- Confirmed NON-COMPLIANT items
- PHASE-COMPLIANT items (temporary allowances)
- AMBIGUITIES
- Confirmed architectural alignments

---

EVIDENCE STANDARD:

- Every claim MUST reference:
  - file + location
  - document + section

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
- no phase-aware classification
- speculative claims
- command/tool usage
- non-English output
