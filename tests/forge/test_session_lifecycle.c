/*
 * test_session_lifecycle.c
 *
 * Verify the stub session lifecycle:
 * - forge_session_create on a stub artifact returns FORGE_ERR_UNSUPPORTED.
 * - forge_session_reset and forge_session_free with NULL return INVALID_HANDLE.
 * - forge_last_error_string is non-NULL after a failure.
 */

#include <stdio.h>
#include "../../include/forge_api.h"

static const unsigned char k_stub_magic[] = { 0x53, 0x54, 0x42, 0x21 };

int main(void)
{
    ForgeBackendId  id      = FORGE_BACKEND_ID_INVALID;
    ForgeArtifact  *art     = NULL;
    ForgeSession   *session = NULL;
    ForgeResult     result;

    result = forge_backend_id_at(0, &id);

    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not get backend id at index 0: %d\n", (int)result);
        return 1;
    }

    /* Load a stub artifact so we have a valid handle to pass. */
    result = forge_artifact_load(id, k_stub_magic, sizeof(k_stub_magic), &art);

    if (result != FORGE_OK || !art)
    {
        fprintf(stderr, "FAIL: could not load stub artifact: %d\n", (int)result);
        return 1;
    }

    /*
     * Session creation must return FORGE_ERR_UNSUPPORTED in this skeleton.
     * No real execution backend is wired up.
     */
    result = forge_session_create(art, &session);

    if (result != FORGE_ERR_UNSUPPORTED)
    {
        fprintf(stderr,
            "FAIL: forge_session_create expected FORGE_ERR_UNSUPPORTED, got %d\n",
            (int)result);
        forge_artifact_unload(art);
        return 1;
    }

    if (session != NULL)
    {
        fprintf(stderr,
            "FAIL: session handle should remain NULL after FORGE_ERR_UNSUPPORTED\n");
        forge_artifact_unload(art);
        return 1;
    }

    /* forge_last_error_string must be non-NULL after failure. */
    if (!forge_last_error_string())
    {
        fprintf(stderr, "FAIL: forge_last_error_string returned NULL after failure\n");
        forge_artifact_unload(art);
        return 1;
    }

    /* Null-handle paths must still return INVALID_HANDLE. */
    result = forge_session_reset(NULL);

    if (result != FORGE_ERR_INVALID_HANDLE)
    {
        fprintf(stderr,
            "FAIL: forge_session_reset(NULL) expected FORGE_ERR_INVALID_HANDLE, got %d\n",
            (int)result);
        forge_artifact_unload(art);
        return 1;
    }

    result = forge_session_free(NULL);

    if (result != FORGE_ERR_INVALID_HANDLE)
    {
        fprintf(stderr,
            "FAIL: forge_session_free(NULL) expected FORGE_ERR_INVALID_HANDLE, got %d\n",
            (int)result);
        forge_artifact_unload(art);
        return 1;
    }

    /* Clean up. */
    result = forge_artifact_unload(art);

    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: forge_artifact_unload returned %d\n", (int)result);
        return 1;
    }

    printf("PASS: test_session_lifecycle\n");
    return 0;
}
