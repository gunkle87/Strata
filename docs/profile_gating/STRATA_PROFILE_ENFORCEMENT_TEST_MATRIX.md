# Strata Profile Enforcement Test Matrix

Purpose
- Define the test and audit matrix for Strata profile enforcement.
- Make sure discovery filtering, direct-call rejection, artifact admission, and
  session narrowing are all covered systematically.

Status
- Planning and verification document.
- This file defines test families and coverage goals.

Core doctrine
- profile enforcement is not real until it is tested from multiple angles

------------------------------------------------------------------------

## 1. Coverage Areas

The profile system needs coverage in these areas:
- build capability boundaries
- product profile filtering
- session narrowing
- discovery filtering
- descriptor filtering
- probe filtering
- direct-call rejection
- artifact admission
- result-code semantics

------------------------------------------------------------------------

## 2. Build Capability Tests

Examples:
- compiled-out backend absent from discovery
- compiled-out extension family absent from discovery
- compiled-out native path returns unsupported

------------------------------------------------------------------------

## 3. Product Profile Tests

Examples:
- hidden backend absent from product discovery
- denied extension absent from capability discovery
- denied native read family absent from visible surface

------------------------------------------------------------------------

## 4. Session Narrowing Tests

Examples:
- session can narrow probe visibility below product baseline
- session can narrow controls below product baseline
- session cannot widen a denied product capability

------------------------------------------------------------------------

## 5. Descriptor Filtering Tests

Examples:
- hidden descriptors excluded from counts
- hidden descriptors excluded from index lookup
- hidden descriptors excluded from ID lookup
- hidden descriptors excluded from name lookup

------------------------------------------------------------------------

## 6. Probe Filtering Tests

Examples:
- hidden probe classes excluded from counts
- hidden probe classes excluded from enumeration
- hidden probe ID/name behave as absent on public surfaces
- privileged probe reads fail explicitly on denied paths

------------------------------------------------------------------------

## 7. Direct-Call Rejection Tests

Examples:
- denied native input submission returns forbidden/temporary stand-in
- denied native read returns forbidden/temporary stand-in
- denied extension entrypoint returns forbidden/temporary stand-in
- denied advanced control returns forbidden/temporary stand-in

------------------------------------------------------------------------

## 8. Artifact Admission Tests

Examples:
- valid compatible allowed artifact loads
- valid compatible product-denied artifact is rejected
- incompatible artifact still returns incompatible
- invalid artifact still fails before policy admission

------------------------------------------------------------------------

## 9. Result-Code Tests

Examples:
- invalid argument is not mistaken for forbidden
- invalid handle is not mistaken for unsupported
- forbidden is not mistaken for incompatible
- filtered lookup absence behaves as designed

------------------------------------------------------------------------

## 10. Audit Expectations

Future audits should verify:
- discovery reflects effective profile
- lookup surfaces reflect effective profile
- direct native paths enforce policy explicitly
- product policy and session policy are not accidentally widened
- `Breadboard` target policy does not contradict runtime admission

------------------------------------------------------------------------

## 11. High-Level Conclusion

Strata should treat profile enforcement as a cross-cutting verification problem,
not just a few runtime tests.

The system is only ready when:
- discovery is filtered,
- direct access is enforced,
- artifact admission is correct,
- and result behavior is stable and explainable.
