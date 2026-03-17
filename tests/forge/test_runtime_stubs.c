/*
 * test_runtime_stubs.c
 *
 * Verify that the common runtime surface exists at the public Forge boundary
 * and fails honestly with FORGE_ERR_UNSUPPORTED after lifecycle validation.
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
    ForgeBackendId id = FORGE_BACKEND_ID_INVALID;
    ForgeArtifact *artifact = NULL;
    ForgeSession *session = NULL;
    ForgeSignalValue values[2];
    unsigned char artifact_bytes[sizeof(TestArtifactHeader) + 4];
    ForgeResult result;

    result = forge_backend_id_at(0, &id);

    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not get backend id at index 0: %d\n", (int)result);
        return 1;
    }

    fill_stub_artifact(artifact_bytes, (unsigned int)id);

    result = forge_artifact_load(id, artifact_bytes, sizeof(artifact_bytes), &artifact);

    if (result != FORGE_OK || !artifact)
    {
        fprintf(stderr, "FAIL: could not load stub artifact: %d\n", (int)result);
        return 1;
    }

    result = forge_session_create(artifact, &session);

    if (result != FORGE_OK || !session)
    {
        fprintf(stderr, "FAIL: could not create stub session: %d\n", (int)result);
        forge_artifact_unload(artifact);
        return 1;
    }

    values[0].signal_id = 1;
    values[0].value = FORGE_LOGIC_1;
    values[1].signal_id = 2;
    values[1].value = FORGE_LOGIC_0;

    result = forge_apply_inputs(session, values, 2);

    if (result != FORGE_ERR_UNSUPPORTED)
    {
        fprintf(stderr,
            "FAIL: forge_apply_inputs expected FORGE_ERR_UNSUPPORTED, got %d\n",
            (int)result);
        forge_session_free(session);
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_step(session, 1);

    if (result != FORGE_ERR_UNSUPPORTED)
    {
        fprintf(stderr,
            "FAIL: forge_step expected FORGE_ERR_UNSUPPORTED, got %d\n",
            (int)result);
        forge_session_free(session);
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_read_outputs(session, values, 2);

    if (result != FORGE_ERR_UNSUPPORTED)
    {
        fprintf(stderr,
            "FAIL: forge_read_outputs expected FORGE_ERR_UNSUPPORTED, got %d\n",
            (int)result);
        forge_session_free(session);
        forge_artifact_unload(artifact);
        return 1;
    }

    if (!forge_last_error_string())
    {
        fprintf(stderr, "FAIL: forge_last_error_string returned NULL after stub failures\n");
        forge_session_free(session);
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_session_free(session);

    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: forge_session_free returned %d\n", (int)result);
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_artifact_unload(artifact);

    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: forge_artifact_unload returned %d\n", (int)result);
        return 1;
    }

    printf("PASS: test_runtime_stubs\n");
    return 0;
}
