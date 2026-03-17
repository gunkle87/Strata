/*
 * test_invalid_handle.c
 *
 * Verify that all Forge API functions that accept handles reject NULL
 * with the correct error code, and that out-of-range backend IDs are
 * rejected properly.
 */

#include <stdio.h>
#include "../../include/forge_api.h"

int main(void)
{
    ForgeResult result;

    /* forge_artifact_unload: NULL artifact must return INVALID_HANDLE. */
    result = forge_artifact_unload(NULL);

    if (result != FORGE_ERR_INVALID_HANDLE)
    {
        fprintf(stderr,
            "FAIL: forge_artifact_unload(NULL) expected FORGE_ERR_INVALID_HANDLE, got %d\n",
            (int)result);
        return 1;
    }

    /* forge_artifact_info: NULL artifact must return INVALID_HANDLE. */
    {
        ForgeArtifactInfo info;

        result = forge_artifact_info(NULL, &info);

        if (result != FORGE_ERR_INVALID_HANDLE)
        {
            fprintf(stderr,
                "FAIL: forge_artifact_info(NULL, ...) expected FORGE_ERR_INVALID_HANDLE, "
                "got %d\n",
                (int)result);
            return 1;
        }
    }

    /* forge_artifact_info: NULL out_info must return INVALID_ARGUMENT. */
    result = forge_artifact_info((ForgeArtifact *)0x1, NULL);

    if (result != FORGE_ERR_INVALID_ARGUMENT)
    {
        fprintf(stderr,
            "FAIL: forge_artifact_info(..., NULL) expected FORGE_ERR_INVALID_ARGUMENT, "
            "got %d\n",
            (int)result);
        return 1;
    }

    /* forge_session_create: NULL artifact must return INVALID_HANDLE. */
    {
        ForgeSession *session = NULL;

        result = forge_session_create(NULL, &session);

        if (result != FORGE_ERR_INVALID_HANDLE)
        {
            fprintf(stderr,
                "FAIL: forge_session_create(NULL, ...) expected FORGE_ERR_INVALID_HANDLE, "
                "got %d\n",
                (int)result);
            return 1;
        }
    }

    /* forge_session_create: NULL out_session must return INVALID_ARGUMENT. */
    result = forge_session_create((ForgeArtifact *)0x1, NULL);

    if (result != FORGE_ERR_INVALID_ARGUMENT)
    {
        fprintf(stderr,
            "FAIL: forge_session_create(..., NULL) expected FORGE_ERR_INVALID_ARGUMENT, "
            "got %d\n",
            (int)result);
        return 1;
    }

    /* forge_session_reset: NULL session must return INVALID_HANDLE. */
    result = forge_session_reset(NULL);

    if (result != FORGE_ERR_INVALID_HANDLE)
    {
        fprintf(stderr,
            "FAIL: forge_session_reset(NULL) expected FORGE_ERR_INVALID_HANDLE, got %d\n",
            (int)result);
        return 1;
    }

    /* forge_session_info: NULL session must return INVALID_HANDLE. */
    {
        ForgeSessionInfo info;

        result = forge_session_info(NULL, &info);

        if (result != FORGE_ERR_INVALID_HANDLE)
        {
            fprintf(stderr,
                "FAIL: forge_session_info(NULL, ...) expected FORGE_ERR_INVALID_HANDLE, "
                "got %d\n",
                (int)result);
            return 1;
        }
    }

    /* forge_session_info: NULL out_info must return INVALID_ARGUMENT. */
    result = forge_session_info((ForgeSession *)0x1, NULL);

    if (result != FORGE_ERR_INVALID_ARGUMENT)
    {
        fprintf(stderr,
            "FAIL: forge_session_info(..., NULL) expected FORGE_ERR_INVALID_ARGUMENT, "
            "got %d\n",
            (int)result);
        return 1;
    }

    /* forge_session_free: NULL session must return INVALID_HANDLE. */
    result = forge_session_free(NULL);

    if (result != FORGE_ERR_INVALID_HANDLE)
    {
        fprintf(stderr,
            "FAIL: forge_session_free(NULL) expected FORGE_ERR_INVALID_HANDLE, got %d\n",
            (int)result);
        return 1;
    }

    /* forge_backend_info: out-of-range ID must be rejected. */
    {
        ForgeBackendInfo info;

        result = forge_backend_info((ForgeBackendId)0xFFFF, &info);

        if (result != FORGE_ERR_BACKEND_UNAVAILABLE)
        {
            fprintf(stderr,
                "FAIL: unknown ID for forge_backend_info expected "
                "FORGE_ERR_BACKEND_UNAVAILABLE, got %d\n",
                (int)result);
            return 1;
        }
    }

    /*
     * forge_artifact_load: NULL data must return INVALID_ARGUMENT.
     * Use a known backend ID to ensure we reach the data check.
     */
    {
        ForgeBackendId id = FORGE_BACKEND_ID_INVALID;
        ForgeArtifact *art = NULL;

        forge_backend_id_at(0, &id);

        result = forge_artifact_load(id, NULL, 0, &art);

        if (result != FORGE_ERR_INVALID_ARGUMENT)
        {
            fprintf(stderr,
                "FAIL: forge_artifact_load with NULL data expected "
                "FORGE_ERR_INVALID_ARGUMENT, got %d\n",
                (int)result);
            return 1;
        }
    }

    printf("PASS: test_invalid_handle\n");
    return 0;
}
