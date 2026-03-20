/*
 * test_refusal_paths.c
 *
 * Verify explicit refusal behavior for unsupported common and extension
 * requests through the public Forge boundary.
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

static ForgeBackendId
find_backend_id_by_name(const char *name)
{
    uint32_t count;
    uint32_t index;
    ForgeBackendId id;
    ForgeBackendInfo info;

    count = forge_backend_count();

    for (index = 0u; index < count; ++index)
    {
        if (forge_backend_id_at(index, &id) != FORGE_OK)
        {
            continue;
        }

        if (forge_backend_info(id, &info) != FORGE_OK)
        {
            continue;
        }

        if (strcmp(info.name, name) == 0)
        {
            return id;
        }
    }

    return FORGE_BACKEND_ID_INVALID;
}

int main(void)
{
    ForgeBackendId lxs_id;
    ForgeBackendId highz_id;
    ForgeArtifact *artifact;
    ForgeSession *session;
    ForgeResult result;
    ForgeProbeValue probe_value;
    ForgeSignalValue output_value;
    unsigned char artifact_bytes[strata_placeholder_artifact_size()];

    if (forge_install_product_profile(FORGE_PRODUCT_PROFILE_UNRESTRICTED) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not install unrestricted profile\n");
        return 1;
    }

    lxs_id = find_backend_id_by_name("LXS");
    highz_id = find_backend_id_by_name("HighZ");

    if (lxs_id == FORGE_BACKEND_ID_INVALID || highz_id == FORGE_BACKEND_ID_INVALID)
    {
        fprintf(stderr, "FAIL: could not discover backend IDs\n");
        return 1;
    }

    fill_stub_artifact(artifact_bytes, (unsigned int)lxs_id);
    artifact = NULL;
    result = forge_artifact_load(lxs_id, artifact_bytes, sizeof(artifact_bytes), &artifact);

    if (result != FORGE_OK || !artifact)
    {
        fprintf(stderr, "FAIL: could not load LXS stub artifact\n");
        return 1;
    }

    session = NULL;
    result = forge_session_create(artifact, &session);

    if (result != FORGE_OK || !session)
    {
        fprintf(stderr, "FAIL: could not create LXS session\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    output_value.signal_id = 0u;
    output_value.value = FORGE_LOGIC_0;
    result = forge_read_outputs(session, &output_value, 1u);

    if (result != FORGE_ERR_UNSUPPORTED)
    {
        fprintf(stderr, "FAIL: placeholder output read should be unsupported, got %d\n",
            (int)result);
        forge_session_free(session);
        forge_artifact_unload(artifact);
        return 1;
    }

    probe_value.probe_id = 0u;
    probe_value.value = 0u;
    result = forge_read_probes(session, &probe_value, 1u);

    if (result != FORGE_ERR_UNSUPPORTED)
    {
        fprintf(stderr, "FAIL: probe reads should be unsupported, got %d\n",
            (int)result);
        forge_session_free(session);
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_session_extension_command(
        session,
        FORGE_EXT_TEMPORAL_CONTROL,
        0u,
        NULL);

    if (result != FORGE_ERR_UNSUPPORTED)
    {
        fprintf(stderr, "FAIL: LXS temporal extension should be unsupported, got %d\n",
            (int)result);
        forge_session_free(session);
        forge_artifact_unload(artifact);
        return 1;
    }

    if (forge_session_free(session) != FORGE_OK ||
        forge_artifact_unload(artifact) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: cleanup failed for LXS refusal checks\n");
        return 1;
    }

    if (forge_install_product_profile(FORGE_PRODUCT_PROFILE_UNRESTRICTED) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not restore unrestricted profile\n");
        return 1;
    }

    fill_stub_artifact(artifact_bytes, (unsigned int)highz_id);
    artifact = NULL;
    result = forge_artifact_load(highz_id, artifact_bytes, sizeof(artifact_bytes), &artifact);

    if (result != FORGE_OK || !artifact)
    {
        fprintf(stderr, "FAIL: could not load HighZ stub artifact under unrestricted profile\n");
        return 1;
    }

    session = NULL;
    result = forge_session_create_with_profile(
        artifact,
        FORGE_SESSION_PROFILE_COMMON_ONLY,
        &session);

    if (result != FORGE_OK || !session)
    {
        fprintf(stderr, "FAIL: could not create HighZ session under common-only session profile\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_session_extension_command(
        session,
        FORGE_EXT_TEMPORAL_CONTROL,
        0u,
        NULL);

    if (result != FORGE_ERR_FORBIDDEN)
    {
        fprintf(stderr, "FAIL: common-only session profile should forbid temporal extension, got %d\n",
            (int)result);
        forge_session_free(session);
        forge_artifact_unload(artifact);
        return 1;
    }

    if (forge_session_free(session) != FORGE_OK ||
        forge_artifact_unload(artifact) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: cleanup failed for common-only HighZ refusal checks\n");
        return 1;
    }

    fill_stub_artifact(artifact_bytes, (unsigned int)highz_id);
    artifact = NULL;
    result = forge_artifact_load(highz_id, artifact_bytes, sizeof(artifact_bytes), &artifact);

    if (result != FORGE_OK || !artifact)
    {
        fprintf(stderr, "FAIL: could not load HighZ stub artifact under unrestricted profile\n");
        return 1;
    }

    session = NULL;
    result = forge_session_create(artifact, &session);

    if (result != FORGE_OK || !session)
    {
        fprintf(stderr, "FAIL: could not create HighZ session under unrestricted profile\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_session_extension_command(
        session,
        FORGE_EXT_TEMPORAL_CONTROL,
        0u,
        NULL);

    if (result != FORGE_ERR_UNSUPPORTED)
    {
        fprintf(stderr,
            "FAIL: supported-but-unimplemented temporal extension should be unsupported, got %d\n",
            (int)result);
        forge_session_free(session);
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_session_extension_command(
        session,
        FORGE_EXT_NONE,
        0u,
        NULL);

    if (result != FORGE_ERR_INVALID_ARGUMENT)
    {
        fprintf(stderr, "FAIL: invalid extension family should be rejected, got %d\n",
            (int)result);
        forge_session_free(session);
        forge_artifact_unload(artifact);
        return 1;
    }

    if (forge_session_free(session) != FORGE_OK ||
        forge_artifact_unload(artifact) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: cleanup failed for unrestricted HighZ refusal checks\n");
        return 1;
    }

    printf("PASS: test_refusal_paths\n");
    return 0;
}
