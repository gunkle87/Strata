/*
 * test_artifact_admission_policy.c
 *
 * Verify coarse artifact admission behavior for placeholder payload classes:
 * - valid and allowed artifacts load
 * - valid but policy-denied artifacts return FORGE_ERR_FORBIDDEN
 * - backend-incompatible artifacts return FORGE_ERR_ARTIFACT_INCOMPATIBLE
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
fill_stub_artifact(
    unsigned char *buffer,
    unsigned int target_backend_id,
    const unsigned char payload[4])
{
    TestArtifactHeader header;

    header.magic[0] = 0x46;
    header.magic[1] = 0x41;
    header.magic[2] = 0x52;
    header.magic[3] = 0x54;
    header.version_major = 0;
    header.version_minor = 1;
    header.target_backend_id = target_backend_id;
    header.payload_size = 4u;

    memcpy(buffer, &header, sizeof(header));
    memcpy(buffer + sizeof(header), payload, 4u);
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
    static const unsigned char k_basic_payload[4] = { 0x53, 0x54, 0x42, 0x21 };
    static const unsigned char k_advanced_payload[4] = { 0x41, 0x44, 0x56, 0x21 };
    static const unsigned char k_native_payload[4] = { 0x4E, 0x41, 0x54, 0x21 };
    ForgeBackendId lxs_id;
    ForgeBackendId highz_id;
    ForgeArtifact *artifact;
    ForgeArtifactInfo info;
    ForgeResult result;
    unsigned char artifact_bytes[sizeof(TestArtifactHeader) + 4];

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

    fill_stub_artifact(artifact_bytes, (unsigned int)highz_id, k_advanced_payload);
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

    if (!info.requires_advanced_controls ||
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

    if (forge_install_product_profile(FORGE_PRODUCT_PROFILE_COMMON_ONLY) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not install common-only profile\n");
        return 1;
    }

    fill_stub_artifact(artifact_bytes, (unsigned int)highz_id, k_advanced_payload);
    artifact = NULL;
    result = forge_artifact_load(highz_id, artifact_bytes, sizeof(artifact_bytes), &artifact);

    if (result != FORGE_ERR_FORBIDDEN)
    {
        fprintf(stderr, "FAIL: common-only profile should forbid advanced artifact, got %d\n",
            (int)result);
        return 1;
    }

    fill_stub_artifact(artifact_bytes, (unsigned int)highz_id, k_native_payload);
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

    fill_stub_artifact(artifact_bytes, (unsigned int)lxs_id, k_advanced_payload);
    artifact = NULL;
    result = forge_artifact_load(lxs_id, artifact_bytes, sizeof(artifact_bytes), &artifact);

    if (result != FORGE_ERR_ARTIFACT_INCOMPATIBLE)
    {
        fprintf(stderr, "FAIL: LXS advanced artifact should be incompatible, got %d\n",
            (int)result);
        return 1;
    }

    fill_stub_artifact(artifact_bytes, (unsigned int)lxs_id, k_basic_payload);
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

    if (info.required_extension_mask != 0u ||
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
