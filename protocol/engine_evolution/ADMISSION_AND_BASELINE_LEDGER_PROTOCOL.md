# Admission And Baseline Ledger Protocol

Purpose
- This file defines how live wins and protected baselines must be recorded.

## 1. Why Ledgers Matter

Without a ledger, projects begin to rely on:
- memory,
- informal chat summary,
- commit names,
- branch mythology.

That always decays.

## 2. Required Ledgers

Every repo using this protocol should keep:
- `admitted default-path ledger`
  - only runtime-affecting changes that are live by default
- `baseline ledger`
  - protected comparison points and their provenance
- `alternate or no-admission inventory`
  - useful branches that are not live

## 3. Admitted Default-Path Ledger Entry

Minimum fields
- admission date
- commit or revision identifier
- checkpoint reference
- phase reference
- owning layer
- effect summary
- corpus and settings basis
- current status

Rules
- only live runtime-affecting wins belong here
- report-only wins do not belong here
- artifact-only wins do not belong here

## 4. Baseline Ledger Entry

Minimum fields
- baseline identifier
- commit or revision identifier
- comparison class
- corpus
- settings
- headline metric
- provenance note
- whether it is still protected

Rules
- every major new plateau should be recorded
- historical values must be labeled as historical
- reruns must be labeled as reruns

## 5. Alternate Or No-Admission Entry

Minimum fields
- candidate name
- checkpoint reference
- why it was not admitted
- what value it still has
- re-entry trigger if any

## 6. Update Sequence After Admission

1. Write the checkpoint
2. Update the admitted-path ledger
3. Update the baseline ledger if needed
4. Update alternate inventory if a previous branch was superseded
5. Update phase status or close note

## 7. Hard Rule

No change should be described as a live engine improvement in summary docs
unless it exists in the admitted default-path ledger.
