# Experiment Governance

Purpose
- This file defines the measurement and decision discipline for experimental
  engine work.

## 1. Core Rule

No performance claim is real until it survives the correct comparison class.

## 2. Comparison Classes

Use one and only one primary comparison class per checkpoint.

- `fixture-only`
  - synthetic cases used for correctness or local behavior only
- `artifact-only`
  - one or a few named artifacts, not a protected suite
- `stress-pack`
  - a deliberately heavy bounded set used to fail fast
- `default-suite screening`
  - broad measurement before any live-path admission
- `default-suite admission`
  - the protected decision class for making a change live by default
- `historical recorded comparison`
  - comparison against previously recorded values
- `same-toolchain rerun`
  - direct rerun under the same current toolchain

Rules
- `artifact-only` cannot directly justify admission.
- `historical recorded comparison` must say it is historical.
- `default-suite admission` should be used for any final live-path decision.

## 3. Corpus Provenance

Every protected measurement must record:
- benchmark root
- suite membership
- known exclusions
- sample count
- iteration count
- cycle count
- special flags or environment variables

If corpus identity is unclear, the result is weak evidence.

## 4. Confirmation Rerun Rule

A confirmation rerun is mandatory when:
- the result changes the default live path,
- the observed improvement is small relative to known run variance,
- the change follows a period of regression recovery,
- the result depends on a new benchmark mode or harness behavior,
- the claim is likely to be used as a future baseline.

If not required, say so explicitly.

## 5. Signal Versus Noise

A checkpoint must distinguish:
- directionally interesting signal,
- likely measurement noise,
- trusted admission-grade improvement.

If the author cannot separate those, the checkpoint is incomplete.

## 6. Suite Survival Rule

A candidate survives only if it holds up on the protected corpus, not just on
the artifacts that inspired it.

This is especially important for:
- structural recognizers,
- packed or arithmetic specializations,
- cache-local rewrites,
- macro substitutions,
- mode dispatch changes.

## 7. Instrumentation Rule

Instrumentation may be added to explain bottlenecks, but:
- it must be attributable,
- it must be off or near-zero cost by default when required,
- it must not be silently credited as engine speed work.

## 8. Recovery Governance

After sustained regression:
- freeze broadening work,
- run one bounded recovery or consolidation pass,
- compare directly to the last stable plateau,
- do not continue expanding until the recovery is understood.

## 9. Decision Vocabulary

Allowed outcome labels:
- `ADMIT`
- `KEEP ALTERNATE`
- `KEEP KERNEL, DEFER`
- `REJECT CHANGE`
- `HOLD FOR LATER`

These labels should be stable across repos so history stays portable.

## 10. Governance Failure Modes

Watch for:
- baseline drift
- comparison-class mixing
- unlabeled historical numbers
- artifact wins being over-credited
- harness changes being misreported as runtime wins
- deferred branches silently becoming canonical in conversation
