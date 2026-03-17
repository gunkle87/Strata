# Strata Forge Artifact Profile Admission Policy

Purpose
- Define how product and session policy affect artifact loading and admission in
  `Forge`.
- Distinguish artifact incompatibility from product-level denial.

Status
- Planning and implementation-facing design document.
- This file defines policy rules for artifact acceptance.

Core doctrine
- compatible is not the same as allowed
- product policy must be checked before execution begins

------------------------------------------------------------------------

## 1. Why This Policy Exists

A runtime artifact can be:
- structurally valid,
- backend-compatible,
- and still not acceptable for a given product.

If Strata does not model that explicitly, artifact loading will drift toward:
- overbroad acceptance,
- late rejection during session use,
- confusion between compatibility and policy.

------------------------------------------------------------------------

## 2. Admission Layers

Artifact admission should have three gates:
- structural/artifact validity
- backend/runtime compatibility
- profile admission

These are different checks with different failure meanings.

------------------------------------------------------------------------

## 3. Structural Validity

Examples:
- valid artifact header
- valid version
- valid section integrity

Failure here is an artifact problem.

------------------------------------------------------------------------

## 4. Backend Compatibility

Examples:
- target backend mismatch
- required runtime assumptions unsupported
- required executable payload incompatible with selected backend

Failure here should use artifact-incompatible behavior.

------------------------------------------------------------------------

## 5. Profile Admission

Examples:
- target backend is hidden by product profile
- required extension family is denied
- artifact assumes privileged probe surface denied to the product
- artifact assumes native observation paths denied to the product

Failure here is not incompatibility.
It is policy denial.

------------------------------------------------------------------------

## 6. Required Policy Questions At Load Time

`Forge` should answer:
- is this backend target visible in the effective profile
- are this artifact's required extension families allowed
- are this artifact's required visible runtime surfaces allowed
- does this session type permit loading this artifact class

------------------------------------------------------------------------

## 7. Timing Rule

Policy admission should happen before:
- session creation,
- execution setup,
- and user-facing runtime use.

Important rule
- `Forge` should not allow load to succeed and then discover later that the
  product forbids the artifact's required surface

------------------------------------------------------------------------

## 8. Result Behavior

Use:
- artifact-incompatible for real compatibility failures
- forbidden/access-denied for policy-denied but otherwise valid artifacts

Until a distinct forbidden result exists:
- follow the temporary documented stand-in policy

------------------------------------------------------------------------

## 9. Interaction With Breadboard

`Breadboard` should eventually respect target and product restrictions, but
`Forge` still owns final runtime admission.

That means:
- `Breadboard` may avoid emitting denied targets
- `Forge` still validates final admissibility

------------------------------------------------------------------------

## 10. Test Requirements

Future tests should verify:
- valid compatible allowed artifact loads
- valid compatible product-denied artifact is rejected explicitly
- invalid artifact still fails for validity reasons
- incompatible artifact still fails for compatibility reasons

------------------------------------------------------------------------

## 11. High-Level Conclusion

Strata should treat artifact admission as a three-stage gate:
- valid
- compatible
- allowed

That keeps product clipping clean and avoids confusing policy denial with
engine incapability.
