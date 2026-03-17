/*
 * test_profile_policy.c
 *
 * Verify that internal Forge policy objects narrow public discovery and
 * admission behavior without changing backend truth.
 */

#include <stdio.h>
#include <string.h>
#include "../../include/forge_api.h"
#include "../../include/forge_capabilities.h"
#include "../../src/forge/forge_policy.h"

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
    ForgeProductExposureProfile common_only_profile;
    ForgeProductExposureProfile lxs_only_profile;
    ForgeSessionRestrictionProfile no_probe_session;
    ForgeBackendId lxs_id;
    ForgeBackendId highz_id;
    ForgeCapabilities caps;
    ForgeArtifact *artifact;
    ForgeSession *session;
    ForgeProbeValue probe_values[1];
    ForgeResult result;
    ForgeDescriptor descriptor;
    ForgeBackendInfo info;
    unsigned char artifact_bytes[sizeof(TestArtifactHeader) + 4];
    uint32_t count;

    forge_policy_reset_defaults();

    lxs_id = find_backend_id_by_name("LXS");
    highz_id = find_backend_id_by_name("HighZ");

    if (lxs_id == FORGE_BACKEND_ID_INVALID || highz_id == FORGE_BACKEND_ID_INVALID)
    {
        fprintf(stderr, "FAIL: could not discover backend IDs under default profile\n");
        return 1;
    }

    common_only_profile.visible_backend_mask = FORGE_BACKEND_MASK_ALL;
    common_only_profile.allowed_extension_mask = (1u << 0) | (1u << 3);
    common_only_profile.visible_probe_class_mask =
        FORGE_PROBE_VISIBILITY_MASK_COMMON_PUBLIC;
    common_only_profile.allow_common_observation = 1u;
    common_only_profile.allow_native_state_read = 0u;
    common_only_profile.allow_common_inputs = 1u;
    common_only_profile.allow_native_inputs = 0u;
    common_only_profile.allow_common_probes = 1u;
    common_only_profile.allow_advanced_controls = 0u;

    forge_policy_install_product_profile_for_test(&common_only_profile);

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

    result = forge_probe_descriptor_by_id(artifact, 1002u, &descriptor);

    if (result != FORGE_ERR_OUT_OF_BOUNDS)
    {
        fprintf(stderr, "FAIL: hidden probe ID should behave as absent, got %d\n",
            (int)result);
        forge_artifact_unload(artifact);
        return 1;
    }

    if (forge_artifact_unload(artifact) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not unload HighZ artifact under common-only profile\n");
        return 1;
    }

    lxs_only_profile.visible_backend_mask = FORGE_BACKEND_MASK_LXS;
    lxs_only_profile.allowed_extension_mask = (1u << 0) | (1u << 3);
    lxs_only_profile.visible_probe_class_mask = FORGE_PROBE_VISIBILITY_MASK_COMMON_PUBLIC;
    lxs_only_profile.allow_common_observation = 1u;
    lxs_only_profile.allow_native_state_read = 0u;
    lxs_only_profile.allow_common_inputs = 1u;
    lxs_only_profile.allow_native_inputs = 0u;
    lxs_only_profile.allow_common_probes = 1u;
    lxs_only_profile.allow_advanced_controls = 0u;

    forge_policy_install_product_profile_for_test(&lxs_only_profile);

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

    forge_policy_reset_defaults();

    no_probe_session.visible_backend_mask = FORGE_BACKEND_MASK_ALL;
    no_probe_session.allowed_extension_mask = (1u << 0) | (1u << 1) | (1u << 2) | (1u << 3);
    no_probe_session.visible_probe_class_mask = 0u;
    no_probe_session.allow_common_observation = 1u;
    no_probe_session.allow_native_state_read = 1u;
    no_probe_session.allow_common_inputs = 1u;
    no_probe_session.allow_native_inputs = 1u;
    no_probe_session.allow_common_probes = 0u;
    no_probe_session.allow_advanced_controls = 1u;

    forge_policy_install_session_profile_for_test(&no_probe_session);

    fill_stub_artifact(artifact_bytes, (unsigned int)lxs_id);
    artifact = NULL;
    result = forge_artifact_load(lxs_id, artifact_bytes, sizeof(artifact_bytes), &artifact);

    if (result != FORGE_OK || !artifact)
    {
        fprintf(stderr, "FAIL: LXS artifact should load under no-probe session profile\n");
        return 1;
    }

    session = NULL;
    result = forge_session_create(artifact, &session);

    if (result != FORGE_OK || !session)
    {
        fprintf(stderr, "FAIL: session create under no-probe session profile returned %d\n",
            (int)result);
        forge_artifact_unload(artifact);
        return 1;
    }

    probe_values[0].probe_id = 1001u;
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

    forge_policy_reset_defaults();

    printf("PASS: test_profile_policy\n");
    return 0;
}
