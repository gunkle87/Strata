/*
 * test_profile_policy.c
 *
 * Verify that internal Forge policy objects narrow public discovery and
 * admission behavior without changing backend truth.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/forge_api.h"
#include "../../include/forge_capabilities.h"
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
    ForgeCapabilities caps;
    ForgeArtifact *artifact;
    ForgeSession *session;
    ForgeProbeValue probe_values[1];
    ForgeResult result;
    ForgeDescriptor descriptor;
    ForgeBackendInfo info;
    unsigned char artifact_bytes[strata_placeholder_artifact_size()];
    uint32_t count;

    if (forge_install_product_profile(FORGE_PRODUCT_PROFILE_UNRESTRICTED) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not install unrestricted profile\n");
        return 1;
    }

    lxs_id = find_backend_id_by_name("LXS");
    highz_id = find_backend_id_by_name("HighZ");

    if (lxs_id == FORGE_BACKEND_ID_INVALID || highz_id == FORGE_BACKEND_ID_INVALID)
    {
        fprintf(stderr, "FAIL: could not discover backend IDs under default profile\n");
        return 1;
    }

    if (forge_install_product_profile(FORGE_PRODUCT_PROFILE_COMMON_ONLY) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not install common-only product profile\n");
        return 1;
    }

    result = forge_backend_capabilities(highz_id, &caps);

    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: highz capabilities under common-only profile returned %d\n",
            (int)result);
        return 1;
    }

    if (caps.temporal_substep != 0u || caps.delta_phase_stepping != 0u)
    {
        fprintf(stderr, "FAIL: common-only profile did not disable advanced controls\n");
        return 1;
    }

    if (caps.extension_family_count != 1u ||
        caps.extension_families[0] != FORGE_EXT_RUNTIME_DIAGNOSTICS)
    {
        fprintf(stderr, "FAIL: common-only profile did not filter HighZ extensions correctly\n");
        return 1;
    }

    fill_stub_artifact(artifact_bytes, (unsigned int)highz_id);
    artifact = NULL;
    result = forge_artifact_load(highz_id, artifact_bytes, sizeof(artifact_bytes), &artifact);

    if (result != FORGE_OK || !artifact)
    {
        fprintf(stderr, "FAIL: HighZ artifact should still load under common-only profile\n");
        return 1;
    }

    result = forge_probe_descriptor_count(artifact, &count);

    if (result != FORGE_OK || count != 1u)
    {
        fprintf(stderr, "FAIL: probe descriptor filtering count expected 1, got %d / %u\n",
            (int)result, count);
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_probe_descriptor_by_id(artifact, 300u, &descriptor);

    if (result != FORGE_OK ||
        descriptor.id != 300u ||
        strcmp(descriptor.name, "placeholder_probe_0") != 0)
    {
        fprintf(stderr, "FAIL: visible placeholder probe metadata mismatch under common-only profile\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    if (forge_artifact_unload(artifact) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not unload HighZ artifact under common-only profile\n");
        return 1;
    }

    if (forge_install_product_profile(FORGE_PRODUCT_PROFILE_UNRESTRICTED) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not restore unrestricted profile before mutation test\n");
        return 1;
    }

    fill_stub_artifact(artifact_bytes, (unsigned int)highz_id);
    artifact = NULL;
    result = forge_artifact_load(highz_id, artifact_bytes, sizeof(artifact_bytes), &artifact);

    if (result != FORGE_OK || !artifact)
    {
        fprintf(stderr, "FAIL: HighZ artifact should load before profile mutation test\n");
        return 1;
    }

    if (forge_install_product_profile(FORGE_PRODUCT_PROFILE_LXS_ONLY) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not install LXS-only profile for mutation test\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_probe_descriptor_count(artifact, &count);

    if (result != FORGE_OK || count != 1u)
    {
        fprintf(stderr, "FAIL: loaded artifact should retain descriptor visibility snapshot, got %d / %u\n",
            (int)result, count);
        forge_artifact_unload(artifact);
        return 1;
    }

    session = NULL;
    result = forge_session_create(artifact, &session);

    if (result != FORGE_ERR_FORBIDDEN)
    {
        fprintf(stderr, "FAIL: session create should honor current product profile after mutation, got %d\n",
            (int)result);
        if (session)
        {
            forge_session_free(session);
        }
        forge_artifact_unload(artifact);
        return 1;
    }

    if (forge_artifact_unload(artifact) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not unload artifact after profile mutation test\n");
        return 1;
    }

    if (forge_install_product_profile(FORGE_PRODUCT_PROFILE_LXS_ONLY) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not install LXS-only product profile\n");
        return 1;
    }

    if (forge_backend_count() != 1u)
    {
        fprintf(stderr, "FAIL: LXS-only profile should expose exactly one backend\n");
        return 1;
    }

    result = forge_backend_info(highz_id, &info);

    if (result != FORGE_ERR_BACKEND_UNAVAILABLE)
    {
        fprintf(stderr, "FAIL: hidden HighZ backend should be unavailable, got %d\n",
            (int)result);
        return 1;
    }

    fill_stub_artifact(artifact_bytes, (unsigned int)highz_id);
    artifact = NULL;
    result = forge_artifact_load(highz_id, artifact_bytes, sizeof(artifact_bytes), &artifact);

    if (result != FORGE_ERR_FORBIDDEN)
    {
        fprintf(stderr, "FAIL: hidden HighZ artifact expected FORGE_ERR_FORBIDDEN, got %d\n",
            (int)result);
        return 1;
    }

    if (forge_install_product_profile(FORGE_PRODUCT_PROFILE_UNRESTRICTED) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not restore unrestricted product profile\n");
        return 1;
    }

    fill_stub_artifact(artifact_bytes, (unsigned int)lxs_id);
    artifact = NULL;
    result = forge_artifact_load(lxs_id, artifact_bytes, sizeof(artifact_bytes), &artifact);

    if (result != FORGE_OK || !artifact)
    {
        fprintf(stderr, "FAIL: LXS artifact should load under no-probe session profile\n");
        return 1;
    }

    session = NULL;
    result = forge_session_create_with_profile(
        artifact,
        FORGE_SESSION_PROFILE_NO_PROBES,
        &session);

    if (result != FORGE_OK || !session)
    {
        fprintf(stderr, "FAIL: session create with no-probes profile returned %d\n",
            (int)result);
        forge_artifact_unload(artifact);
        return 1;
    }

    probe_values[0].probe_id = 300u;
    probe_values[0].value = 0u;
    result = forge_read_probes(session, probe_values, 1u);

    if (result != FORGE_ERR_FORBIDDEN)
    {
        fprintf(stderr, "FAIL: no-probe session should deny probe reads, got %d\n",
            (int)result);
        forge_session_free(session);
        forge_artifact_unload(artifact);
        return 1;
    }

    if (forge_session_free(session) != FORGE_OK ||
        forge_artifact_unload(artifact) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: cleanup failed in profile policy test\n");
        return 1;
    }

    if (forge_install_product_profile(FORGE_PRODUCT_PROFILE_UNRESTRICTED) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not restore unrestricted profile at end of test\n");
        return 1;
    }

    printf("PASS: test_profile_policy\n");
    return 0;
}
