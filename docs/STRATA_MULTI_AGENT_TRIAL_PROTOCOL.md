# Strata Multi-Agent Trial Protocol

Purpose
- Define the working multi-agent method for the next 15 to 20 Strata tasks.
- Keep execution controlled while we learn the real strengths and failure modes
  of each available agent.
- Prevent unstructured parallelism from creating drift, overlap, or wasted
  cleanup effort.

Status
- Trial protocol
- Semi-permanent for the next 15 to 20 tasks
- Subject to revision after enough evidence is collected

------------------------------------------------------------------------

## 1. Trial Rule

For the next 15 to 20 Strata tasks:
- `Codex` remains continuously active
- only one external agent may be active at a time

This is the core trial constraint.

Reason
- it gives Strata one continuous architecture and integration owner,
- it limits overlap risk,
- it makes attribution and cleanup easier,
- it lets us evaluate each external agent more fairly.

------------------------------------------------------------------------

## 2. Active Lanes

### Lane 1: Codex

`Codex` is always on.

Default responsibilities:
- architecture ownership
- directive writing
- boundary decisions
- implementation review
- cleanup and recovery
- scoped audits
- integration decisions
- commit planning

### Lane 2: One External Agent

Exactly one external agent may be active at a time:
- `Claude`
- `Gemini`
- `Audit agent`

Hard trial rule
- no second external agent begins until the current external agent is fully
  stopped and its results are reviewed.

------------------------------------------------------------------------

## 3. Why This Trial Exists

We are still learning:
- which agents are dependable for exact implementation,
- which agents are good only for bounded mechanical work,
- which agents are strong auditors,
- how much cleanup each agent tends to create,
- what kinds of directives each agent follows well or poorly.

This trial is designed to generate that evidence without letting the repo turn
into a multi-agent collision site.

------------------------------------------------------------------------

## 4. Operating Sequence

Each task should follow this order:

1. `Codex` defines the task boundary
2. `Codex` writes the directive
3. one external agent performs the implementation or audit
4. `Codex` reviews the result
5. `Codex` fixes, tightens, or accepts the work
6. if needed, the external audit agent runs afterward as a separate step
7. `Codex` decides commit readiness

Important rule
- review happens between external-agent runs
- not after several external runs have piled up

------------------------------------------------------------------------

## 5. Allowed External Agent Roles During Trial

### Claude

Preferred use:
- exacting implementation
- medium-sized C tasks
- header/source skeleton work
- tasks where low drift matters

### Gemini

Preferred use:
- bounded implementation
- mechanical expansion work
- isolated tooling work
- support tasks where cleanup is acceptable

### Audit Agent

Preferred use:
- directive compliance audits
- clean-room checks
- repo-boundary checks
- style and constraint audits
- specialty audit passes

------------------------------------------------------------------------

## 6. Codex Role During Trial

`Codex` should remain the stable integration owner.

That means:
- write directives
- decide task boundaries
- review external work
- repair drift
- keep architecture intact
- avoid leaving external-agent work unreviewed

Default rule
- if a task touches:
  - `Forge`
  - `Breadboard`
  - shared artifact contract
  - backend boundary
  - repo rules or protocols
  then `Codex` remains directly involved before and after the external task.

------------------------------------------------------------------------

## 7. Task Selection Rules

During the trial:

Good external-agent tasks:
- isolated tooling
- bounded implementation
- test additions
- file scaffolding
- narrow parser work
- repetitive source creation
- specialty audits

Higher-risk tasks:
- shared runtime boundary work
- cross-layer integration
- artifact loading
- backend registration
- capability registry implementation

For higher-risk tasks:
- `Codex` must write a tighter directive
- `Codex` must review immediately after completion

------------------------------------------------------------------------

## 8. Stop Conditions

An external-agent run should stop and hand back control when:
- scope is drifting,
- files outside directive scope are being touched,
- the agent is stuck,
- the agent begins debugging blindly,
- the agent introduces repo-boundary violations,
- the task begins to affect architecture instead of only implementation.

After stop:
- `Codex` reviews
- `Codex` decides whether to repair, redirect, or reassign

------------------------------------------------------------------------

## 9. Trial Evaluation Criteria

Over the next 15 to 20 tasks, we should evaluate each external agent on:
- directive compliance
- architecture discipline
- cleanup cost
- need for repair after completion
- speed
- quality of test coverage
- quality of status reporting
- tendency to drift

This trial is successful if it gives us a clearer map of:
- what each agent is good for
- what each agent should not be trusted with

------------------------------------------------------------------------

## 10. Trial Notes To Preserve

After notable tasks, we should record:
- which agent ran the task
- what the task type was
- whether cleanup was needed
- whether the directive was followed cleanly
- whether the result was commit-ready

This does not need a heavy process yet.
Simple checkpoint notes are enough.

------------------------------------------------------------------------

## 11. Non-Permanent Status

This protocol is not permanent policy yet.

It is intentionally:
- structured enough to guide behavior now,
- temporary enough to be revised once we have enough data.

After 15 to 20 tasks, reassess:
- whether one external agent at a time is still best,
- whether a dedicated audit phase should always follow implementation,
- whether some agents deserve narrower or broader task classes.

------------------------------------------------------------------------

## 12. High-Level Conclusion

For now, Strata should operate with:
- one continuous architecture owner,
- one external worker at a time,
- explicit review between runs,
- and enough discipline to learn from the agents instead of being dragged by
  them.

That gives us speed without sacrificing control.
