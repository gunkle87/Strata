/*
 * test_session_lifecycle.c
 *
 * Verify the stub session lifecycle:
 * - forge_session_create on a stub artifact returns FORGE_OK and a live handle.
 * - forge_session_reset succeeds on the live handle.
 * - forge_session_reset and forge_session_free with NULL return INVALID_HANDLE.
 * - forge_last_error_string is non-NULL after a failure.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/forge_api.h"
#include "../../include/strata_placeholder_artifact.h"

static void
fill_stub_artifact(unsigned char *buffer, unsigned int target_backend_id)
{
    StrataPlaceholderAdmissionInfo admission_info;
    size_t out_size;

    if (!strata_placeholder_expected_admission_info(
        STRATA_PLACEHOLDER_PAYLOAD_BASELINE,
        &admission_info))
    {
        fprintf(stderr, "FAIL: could not build baseline placeholder admission info\n");
        exit(1);
    }

    (void)strata_placeholder_artifact_write(
        buffer,
        strata_placeholder_artifact_size(),
        target_backend_id,
        STRATA_PLACEHOLDER_PAYLOAD_BASELINE,
        &admission_info,
        &out_size);
}

int main(void)
{
    ForgeBackendId  id      = FORGE_BACKEND_ID_INVALID;
    ForgeArtifact  *art     = NULL;
    ForgeSession   *session = NULL;
    ForgeSessionInfo session_info;
    ForgeResult     result;
    unsigned char   artifact_bytes[strata_placeholder_artifact_size()];

    result = forge_backend_id_at(0, &id);

    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not get backend id at index 0: %d\n", (int)result);
        return 1;
    }

    fill_stub_artifact(artifact_bytes, (unsigned int)id);

    /* Load a stub artifact so we have a valid handle to pass. */
    result = forge_artifact_load(id, artifact_bytes, sizeof(artifact_bytes), &art);

    if (result != FORGE_OK || !art)
    {
        fprintf(stderr, "FAIL: could not load stub artifact: %d\n", (int)result);
        return 1;
    }

    /* Session creation now succeeds for the stub lifecycle path. */
    result = forge_session_create(art, &session);

    if (result != FORGE_OK)
    {
        fprintf(stderr,
            "FAIL: forge_session_create expected FORGE_OK, got %d\n",
            (int)result);
        forge_artifact_unload(art);
        return 1;
    }

    if (session == NULL)
    {
        fprintf(stderr,
            "FAIL: session handle should be non-NULL after FORGE_OK\n");
        forge_artifact_unload(art);
        return 1;
    }

    result = forge_session_info(session, &session_info);

    if (result != FORGE_OK)
    {
        fprintf(stderr,
            "FAIL: forge_session_info expected FORGE_OK, got %d\n",
            (int)result);
        forge_session_free(session);
        forge_artifact_unload(art);
        return 1;
    }

    if (session_info.backend_id != id ||
        session_info.lifecycle_state != FORGE_SESSION_STATE_READY ||
        session_info.placeholder_state != 0)
    {
        fprintf(stderr, "FAIL: forge_session_info returned unexpected metadata\n");
        forge_session_free(session);
        forge_artifact_unload(art);
        return 1;
    }

    result = forge_session_reset(session);

    if (result != FORGE_OK)
    {
        fprintf(stderr,
            "FAIL: forge_session_reset expected FORGE_OK, got %d\n",
            (int)result);
        forge_session_free(session);
        forge_artifact_unload(art);
        return 1;
    }

    result = forge_session_free(session);

    if (result != FORGE_OK)
    {
        fprintf(stderr,
            "FAIL: forge_session_free expected FORGE_OK, got %d\n",
            (int)result);
        forge_artifact_unload(art);
        return 1;
    }

    session = NULL;

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

    if (!forge_last_error_string())
    {
        fprintf(stderr, "FAIL: forge_last_error_string returned NULL after failure\n");
        forge_artifact_unload(art);
        return 1;
    }

    /* Clean up artifact after session release. */
    result = forge_artifact_unload(art);

    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: forge_artifact_unload returned %d\n", (int)result);
        return 1;
    }

    printf("PASS: test_session_lifecycle\n");
    return 0;
}
