# CHECKPOINT TEMPLATE

Use this template for every meaningful measurement or decision point.

## Title
- checkpoint name and phase

## Comparison Class
- one of:
  - fixture-only
  - artifact-only
  - stress-pack
  - default-suite screening
  - default-suite admission
  - historical recorded comparison
  - same-toolchain rerun

## Candidate
- candidate name:
- layer attribution:

## Baseline
- baseline identifier:
- whether baseline is historical or rerun:

## Corpus
- benchmark root:
- suite membership or artifact list:
- known exclusions:

## Settings
- samples:
- sample select mode:
- iterations:
- cycles:
- special flags:

## Validation State
- fixture correctness:
- parser-backed correctness:
- report-only state:

## Result
- candidate result:
- baseline result:
- delta:

## Interpretation
- one short paragraph explaining:
  - what changed
  - why the result is or is not trustworthy
  - whether the attribution is clean

## Admission Status
- one of:
  - ADMIT
  - KEEP ALTERNATE
  - KEEP KERNEL, DEFER
  - REJECT CHANGE
  - HOLD FOR LATER

## Confirmation Rerun Status
- one of:
  - not required
  - required and completed
  - required and pending

## Current Live State
- what is live now
- what is explicitly not admitted

## Follow-On Action
- next safe move

## Ledger Update Requirement
- if `ADMIT`, update:
  - admitted-path ledger
  - baseline ledger if the protected baseline changes
- if not admitted but preserved:
  - alternate or no-admission inventory
