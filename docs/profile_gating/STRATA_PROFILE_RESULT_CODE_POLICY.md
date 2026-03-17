# Strata Profile Result Code Policy

Purpose
- Define how `Forge` should report failures related to product profiles,
  feature gating, and runtime clipping.
- Prevent inconsistent use of `unsupported`, `not found`, and future
  forbidden/access-denied behavior.
- Give future implementation slices a stable decision rule for result codes.

Status
- Planning and implementation-facing policy document.
- This file defines intended result semantics for profile and gating behavior.
- It does not itself modify `forge_result.h`.

Core doctrine
- discovery should hide denied features
- direct access to denied features should fail explicitly
- result codes should explain why the call failed

------------------------------------------------------------------------

## 1. Why This Policy Exists

Strata is moving toward profile-driven feature clipping.

That means `Forge` will increasingly need to distinguish between these very
different realities:
- the caller asked for something invalid,
- the caller asked for something that does not exist,
- the caller asked for something the runtime cannot support,
- the caller asked for something the product intentionally forbids.

If those are all reported through the same result path, the API becomes:
- harder to reason about,
- harder to test,
- harder to document,
- and easier to clip in patchy, inconsistent ways.

This policy exists to stop that before profile enforcement lands in code.

------------------------------------------------------------------------

## 2. Current Forge Result Surface

The current public `ForgeResult` enum includes:
- `FORGE_OK`
- `FORGE_ERR_INVALID_ARGUMENT`
- `FORGE_ERR_INVALID_HANDLE`
- `FORGE_ERR_INVALID_LIFECYCLE`
- `FORGE_ERR_BACKEND_UNAVAILABLE`
- `FORGE_ERR_ARTIFACT_INCOMPATIBLE`
- `FORGE_ERR_UNSUPPORTED`
- `FORGE_ERR_OUT_OF_BOUNDS`
- `FORGE_ERR_INTERNAL`

This is enough for early skeleton work, but not ideal for full profile
enforcement.

The main gap is:
- no dedicated forbidden/access-denied style result

------------------------------------------------------------------------

## 3. Core Rules

1. Result codes should describe the reason for failure, not only that failure
   happened.
2. Discovery filtering and direct-call rejection must be treated differently.
3. A product-forbidden feature is not the same as an unsupported feature.
4. A missing descriptor is not the same as a forbidden descriptor.
5. Artifact incompatibility is not the same as product rejection.
6. Result behavior must stay stable once profile enforcement starts landing.

------------------------------------------------------------------------

## 4. Discovery Versus Direct Access

Strata should use two complementary mechanisms:

### 4.1 Discovery Filtering

When a capability is denied by build, product, or session policy, it should
often disappear from normal discovery.

Examples:
- hidden backends absent from enumeration
- denied extension families absent from capability discovery
- forbidden probes absent from descriptor lists

### 4.2 Direct Access Rejection

If the caller still reaches a denied path directly, `Forge` must reject it
explicitly.

Examples:
- guessed probe ID
- guessed probe name
- direct native-state read call
- direct native input submission call
- direct backend-native extension call

Important rule
- discovery filtering does not remove the need for explicit result behavior

------------------------------------------------------------------------

## 5. Recommended Semantic Meanings

The following semantic meanings should remain stable.

### 5.1 `FORGE_ERR_INVALID_ARGUMENT`

Use when:
- a required pointer is `NULL`
- a numeric argument is malformed
- a string argument is `NULL` where required
- the call shape itself is invalid before any policy or runtime lookup happens

Do not use for:
- forbidden features
- not-found descriptors
- unsupported backend capabilities

### 5.2 `FORGE_ERR_INVALID_HANDLE`

Use when:
- a handle is `NULL`
- a handle does not refer to a live object
- a handle belongs to the wrong runtime state for the call

Do not use for:
- product policy denial
- missing descriptors within a valid object

### 5.3 `FORGE_ERR_INVALID_LIFECYCLE`

Use when:
- the call is made in the wrong lifecycle order
- the handle is valid, but the operation is not legal in the current state

Examples:
- reading from a session before a valid artifact is loaded
- advancing a session before required setup is complete

### 5.4 `FORGE_ERR_BACKEND_UNAVAILABLE`

Use when:
- the backend exists conceptually but is not available to this runtime path

Near-term guidance:
- this may cover "compiled out" or "not currently available" conditions
- it should not become the general result for all profile gating

### 5.5 `FORGE_ERR_ARTIFACT_INCOMPATIBLE`

Use when:
- an artifact cannot be legally loaded because its target or required runtime
  assumptions do not match the selected backend/runtime compatibility contract

Use for:
- target mismatch
- artifact format/version mismatch where it is truly an artifact compatibility
  issue
- incompatible capability assumptions encoded into the artifact

Do not use for:
- a product policy that intentionally forbids an otherwise valid artifact class

### 5.6 `FORGE_ERR_UNSUPPORTED`

Use when:
- the runtime/backend/build genuinely does not support the requested behavior
- the operation is outside actual implemented capability
- the feature is not present in this build in a structural sense

Examples:
- unimplemented skeleton path
- runtime path not yet built
- feature not supported by the backend design

Do not use as the final long-term result for:
- product policy denial

### 5.7 `FORGE_ERR_OUT_OF_BOUNDS`

Use when:
- an index is outside the visible valid range
- a visible-ID lookup space treats the requested ID as invalid/out of range

Use carefully:
- for profile-filtered lookups, once filtering is applied, out-of-bounds should
  refer only to the visible space

Do not use when:
- the object exists but is intentionally forbidden and hidden by policy
- a name lookup fails because a name is absent

### 5.8 `FORGE_ERR_INTERNAL`

Use only for:
- unexpected internal invariants breaking
- impossible state under the documented public contract

Do not use as a fallback for policy cases.

------------------------------------------------------------------------

## 6. The Missing Result: Forbidden Or Access Denied

Strata should add a dedicated public result code for profile denial before the
profile system is considered complete.

Recommended conceptual name:
- `FORGE_ERR_FORBIDDEN`

Possible alternate name:
- `FORGE_ERR_ACCESS_DENIED`

This result should mean:
- the runtime could support the request in principle,
- but the effective build/product/session profile intentionally forbids it.

This is the correct result for cases such as:
- denied native-state reads
- denied native input submission
- denied probe classes reached by direct call
- denied backend-native extension entrypoints
- denied advanced runtime controls
- artifact types denied by product policy, when the artifact is otherwise valid

------------------------------------------------------------------------

## 7. Why Forbidden Must Be Separate From Unsupported

These are different truths:

- unsupported
  - the runtime/backend/build cannot do this

- forbidden
  - the runtime/backend/build could do this, but this product/session may not

If Strata merges those concepts:
- tests become weaker,
- capability policy becomes harder to verify,
- product restrictions become less auditable,
- logs become less useful,
- later cleanup becomes more painful.

So the direction should be:
- early skeletons may temporarily use `FORGE_ERR_UNSUPPORTED`
- final profile enforcement should use a dedicated forbidden result

------------------------------------------------------------------------

## 8. Not Found Versus Forbidden

This distinction matters especially for descriptors and probes.

### 8.1 Not Found

Use a not-found style outcome when:
- the requested object does not exist in the visible allowed space
- and there is no need to disclose whether it exists outside that space

### 8.2 Forbidden

Use forbidden when:
- the caller is reaching a feature family or entrypoint that the profile
  explicitly denies,
- and the API contract is already at a privileged or backend-native boundary
  where policy denial should be explicit

Practical guidance:
- normal filtered enumeration and ordinary descriptor lookup should usually make
  forbidden items look absent
- direct privileged entrypoints should fail as forbidden

This gives the cleanest split between:
- safe public discovery
- explicit denial at privileged call boundaries

------------------------------------------------------------------------

## 9. Artifact Rejection Semantics

Artifact rejection needs two distinct outcomes.

### 9.1 Incompatible Artifact

Use `FORGE_ERR_ARTIFACT_INCOMPATIBLE` when:
- the artifact cannot run on the selected backend/runtime contract

### 9.2 Forbidden Artifact

Use forbidden when:
- the artifact is technically valid and compatible,
- but the effective profile denies loading that artifact class or required
  extension family

This distinction will matter for product-profile debugging and supportability.

------------------------------------------------------------------------

## 10. Recommended Near-Term Policy Before Enum Expansion

Until `forge_result.h` is expanded, Strata should apply these temporary rules:

1. Use discovery filtering first.
2. Use `FORGE_ERR_UNSUPPORTED` for profile-denied direct calls only as a
   temporary stand-in.
3. Document clearly when `FORGE_ERR_UNSUPPORTED` is carrying policy denial
   instead of true lack of backend capability.
4. Avoid spreading this ambiguity across more call sites than necessary.

Important rule
- this temporary compromise should not be treated as the final architecture

------------------------------------------------------------------------

## 11. Recommended Future Enum Change

At the right implementation slice, Strata should update `ForgeResult` to add a
dedicated forbidden result.

Recommended shape:
- keep existing values stable where practical
- add one new explicit policy-denial result
- update docs and tests together

When this happens, the following areas should be updated in the same pass:
- `forge_result.h`
- `Forge` public API comments
- profile enforcement code
- artifact admission code
- probe filtering tests
- native extension tests

------------------------------------------------------------------------

## 12. Required Test Families

When profile enforcement lands, tests should verify:
- denied feature absent from discovery
- denied direct native read returns forbidden
- denied direct native input returns forbidden
- denied privileged probe read returns forbidden
- valid but product-denied artifact returns forbidden
- genuinely unsupported feature still returns unsupported
- incompatible artifact still returns incompatible
- filtered public lookup still behaves like not found or out-of-bounds where
  intended

------------------------------------------------------------------------

## 13. High-Level Conclusion

Strata should not let profile enforcement blur into generic unsupported/error
handling.

The correct path is:
- stable meanings for existing result codes,
- filtered discovery for denied features,
- explicit direct-call rejection for forbidden access,
- and a future dedicated forbidden result code in `Forge`.

That will keep product clipping understandable, testable, and non-patchy as
the runtime grows.
