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
#include <string.h>
#include "../../include/forge_api.h"

typedef struct TestArtifactHeader
{
    unsigned char magic[4];
    unsigned short version_major;
    unsigned short version_minor;
    unsigned int target_backend_id;
    unsigned int payload_size;
}
TestArtifactHeader;

static void
fill_stub_artifact(unsigned char *buffer, unsigned int target_backend_id)
{
    TestArtifactHeader header;
    static const unsigned char stub_payload[] = { 0x53, 0x54, 0x42, 0x21 };

    header.magic[0] = 0x46;
    header.magic[1] = 0x41;
    header.magic[2] = 0x52;
    header.magic[3] = 0x54;
    header.version_major = 0;
    header.version_minor = 1;
    header.target_backend_id = target_backend_id;
    header.payload_size = (unsigned int)sizeof(stub_payload);

    memcpy(buffer, &header, sizeof(header));
    memcpy(buffer + sizeof(header), stub_payload, sizeof(stub_payload));
}

int main(void)
{
    ForgeBackendId  id      = FORGE_BACKEND_ID_INVALID;
    ForgeArtifact  *art     = NULL;
    ForgeSession   *session = NULL;
    ForgeSessionInfo session_info;
    ForgeResult     result;
    unsigned char   artifact_bytes[sizeof(TestArtifactHeader) + 4];

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
