#ifndef FORGE_RESULT_H
#define FORGE_RESULT_H

/*
 * forge_result.h
 *
 * Result codes for the Forge public API.
 * Every Forge API function that can fail returns a ForgeResult.
 * Callers must check the result before using any out-parameters.
 */

typedef enum ForgeResult
{
    /* Operation completed successfully. */
    FORGE_OK = 0,

    /* A required argument was NULL or out of range. */
    FORGE_ERR_INVALID_ARGUMENT = 1,

    /* A handle argument was NULL or does not refer to a live object. */
    FORGE_ERR_INVALID_HANDLE = 2,

    /* The call was made in the wrong lifecycle order. */
    FORGE_ERR_INVALID_LIFECYCLE = 3,

    /* The requested backend exists in the registry but is not available. */
    FORGE_ERR_BACKEND_UNAVAILABLE = 4,

    /* The artifact is not compatible with the selected backend. */
    FORGE_ERR_ARTIFACT_INCOMPATIBLE = 5,

    /* The requested operation is not supported by this backend or this build. */
    FORGE_ERR_UNSUPPORTED = 6,

    /* An index or ID was outside the valid range. */
    FORGE_ERR_OUT_OF_BOUNDS = 7,

    /* An internal error that should not occur under normal conditions. */
    FORGE_ERR_INTERNAL = 8,

    /* The runtime could support the call, but active policy forbids it. */
    FORGE_ERR_FORBIDDEN = 9

}
ForgeResult;

#endif /* FORGE_RESULT_H */
