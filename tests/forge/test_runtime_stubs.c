/*
 * test_runtime_stubs.c
 *
 * Verify that the common runtime surface exists at the public Forge boundary
 * and fails honestly with FORGE_ERR_UNSUPPORTED after lifecycle validation.
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
    ForgeBackendId id = FORGE_BACKEND_ID_INVALID;
    ForgeArtifact *artifact = NULL;
    ForgeSession *session = NULL;
    ForgeSignalValue values[2];
    unsigned char artifact_bytes[strata_placeholder_artifact_size()];
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
