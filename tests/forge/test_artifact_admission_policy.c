/*
 * test_artifact_admission_policy.c
 *
 * Verify coarse artifact admission behavior for placeholder payload classes:
 * - valid and allowed artifacts load
 * - valid but policy-denied artifacts return FORGE_ERR_FORBIDDEN
 * - backend-incompatible artifacts return FORGE_ERR_ARTIFACT_INCOMPATIBLE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/forge_api.h"
#include "../../include/strata_placeholder_artifact.h"

static void
fill_stub_artifact(
    unsigned char *buffer,
    unsigned int target_backend_id,
    StrataPlaceholderPayloadKind payload_kind)
{
    StrataPlaceholderAdmissionInfo admission_info;
    size_t out_size;

    if (!strata_placeholder_expected_admission_info(payload_kind, &admission_info))
    {
        fprintf(stderr, "FAIL: could not build placeholder admission info\n");
        exit(1);
    }

    (void)strata_placeholder_artifact_write(
        buffer,
        strata_placeholder_artifact_size(),
        target_backend_id,
        payload_kind,
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

int
main(void)
{
    ForgeBackendId lxs_id;
    ForgeBackendId highz_id;
    ForgeArtifact *artifact;
    ForgeArtifactInfo info;
    ForgeResult result;
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
        fprintf(stderr, "FAIL: could not resolve backend IDs\n");
        return 1;
    }

    fill_stub_artifact(
        artifact_bytes,
        (unsigned int)highz_id,
        STRATA_PLACEHOLDER_PAYLOAD_ADVANCED);
    artifact = NULL;
    result = forge_artifact_load(highz_id, artifact_bytes, sizeof(artifact_bytes), &artifact);

    if (result != FORGE_OK || !artifact)
    {
        fprintf(stderr, "FAIL: unrestricted advanced HighZ artifact should load, got %d\n",
            (int)result);
        return 1;
    }

    result = forge_artifact_info(artifact, &info);
    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: forge_artifact_info failed for advanced payload, got %d\n",
            (int)result);
        forge_artifact_unload(artifact);
        return 1;
    }

    if (info.source_target_value != 2u ||
        info.source_has_placeholders != 1u ||
        info.source_approximate_size_bytes != 1024u ||
        !info.requires_advanced_controls ||
        info.requires_native_state_read ||
        info.requires_native_inputs ||
        info.required_extension_mask == 0u)
    {
        fprintf(stderr, "FAIL: advanced payload metadata was not populated correctly\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    if (forge_artifact_unload(artifact) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not unload advanced artifact\n");
        return 1;
    }

    fill_stub_artifact(
        artifact_bytes,
        (unsigned int)highz_id,
        STRATA_PLACEHOLDER_PAYLOAD_ADVANCED);
    ((StrataPlaceholderArtifactHeader*)artifact_bytes)->admission_info.requires_advanced_controls = 0u;
    artifact = NULL;
    result = forge_artifact_load(highz_id, artifact_bytes, sizeof(artifact_bytes), &artifact);

    if (result != FORGE_ERR_ARTIFACT_INCOMPATIBLE)
    {
        fprintf(stderr, "FAIL: mismatched advanced admission manifest should be incompatible, got %d\n",
            (int)result);
        return 1;
    }

    if (forge_install_product_profile(FORGE_PRODUCT_PROFILE_COMMON_ONLY) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not install common-only profile\n");
        return 1;
    }

    fill_stub_artifact(
        artifact_bytes,
        (unsigned int)highz_id,
        STRATA_PLACEHOLDER_PAYLOAD_ADVANCED);
    artifact = NULL;
    result = forge_artifact_load(highz_id, artifact_bytes, sizeof(artifact_bytes), &artifact);

    if (result != FORGE_ERR_FORBIDDEN)
    {
        fprintf(stderr, "FAIL: common-only profile should forbid advanced artifact, got %d\n",
            (int)result);
        return 1;
    }

    fill_stub_artifact(
        artifact_bytes,
        (unsigned int)highz_id,
        STRATA_PLACEHOLDER_PAYLOAD_NATIVE);
    artifact = NULL;
    result = forge_artifact_load(highz_id, artifact_bytes, sizeof(artifact_bytes), &artifact);

    if (result != FORGE_ERR_FORBIDDEN)
    {
        fprintf(stderr, "FAIL: common-only profile should forbid native artifact, got %d\n",
            (int)result);
        return 1;
    }

    if (forge_install_product_profile(FORGE_PRODUCT_PROFILE_UNRESTRICTED) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not restore unrestricted profile\n");
        return 1;
    }

    fill_stub_artifact(
        artifact_bytes,
        (unsigned int)lxs_id,
        STRATA_PLACEHOLDER_PAYLOAD_ADVANCED);
    artifact = NULL;
    result = forge_artifact_load(lxs_id, artifact_bytes, sizeof(artifact_bytes), &artifact);

    if (result != FORGE_ERR_ARTIFACT_INCOMPATIBLE)
    {
        fprintf(stderr, "FAIL: LXS advanced artifact should be incompatible, got %d\n",
            (int)result);
        return 1;
    }

    fill_stub_artifact(
        artifact_bytes,
        (unsigned int)lxs_id,
        STRATA_PLACEHOLDER_PAYLOAD_BASELINE);
    artifact = NULL;
    result = forge_artifact_load(lxs_id, artifact_bytes, sizeof(artifact_bytes), &artifact);

    if (result != FORGE_OK || !artifact)
    {
        fprintf(stderr, "FAIL: basic LXS artifact should still load, got %d\n",
            (int)result);
        return 1;
    }

    result = forge_artifact_info(artifact, &info);
    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: forge_artifact_info failed for basic payload, got %d\n",
            (int)result);
        forge_artifact_unload(artifact);
        return 1;
    }

    if (info.source_target_value != 1u ||
        info.source_has_placeholders != 1u ||
        info.source_approximate_size_bytes != 1024u ||
        info.required_extension_mask != 0u ||
        info.requires_advanced_controls != 0u ||
        info.requires_native_state_read != 0u ||
        info.requires_native_inputs != 0u)
    {
        fprintf(stderr, "FAIL: basic payload should not report extra requirements\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    if (forge_artifact_unload(artifact) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not unload basic artifact\n");
        return 1;
    }

    if (forge_install_product_profile(FORGE_PRODUCT_PROFILE_UNRESTRICTED) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not restore unrestricted profile at end of test\n");
        return 1;
    }

    printf("PASS: test_artifact_admission_policy\n");
    return 0;
}
