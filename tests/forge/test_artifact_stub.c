/*
 * test_artifact_stub.c
 *
 * Verify the stub artifact load path:
 * - Known backend + stub magic bytes -> FORGE_OK, non-NULL handle.
 * - forge_artifact_unload frees it cleanly.
 * - Unknown backend ID is rejected.
 * - Non-stub data is rejected with FORGE_ERR_UNSUPPORTED.
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
    ForgeBackendId  id   = FORGE_BACKEND_ID_INVALID;
    ForgeBackendId  other_id = FORGE_BACKEND_ID_INVALID;
    ForgeArtifact  *art  = NULL;
    ForgeArtifactInfo info;
    ForgeResult     result;
    unsigned char   artifact_bytes[strata_placeholder_artifact_size()];
    unsigned char   bad_magic[strata_placeholder_artifact_size()];
    unsigned char   backend_mismatch[strata_placeholder_artifact_size()];
    unsigned char   unsupported_payload[strata_placeholder_artifact_size()];
    unsigned char   bad_descriptor_bytes[strata_placeholder_artifact_size()];
    unsigned char   bad_descriptor_class[strata_placeholder_artifact_size()];
    unsigned char   bad_descriptor_name[strata_placeholder_artifact_size()];
    StrataPlaceholderArtifactHeader *bad_header;
    StrataPlaceholderSerializedDescriptor *bad_descriptors;

    result = forge_backend_id_at(0, &id);

    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not get backend id at index 0: %d\n", (int)result);
        return 1;
    }

    result = forge_backend_id_at(1, &other_id);

    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not get backend id at index 1: %d\n", (int)result);
        return 1;
    }

    fill_stub_artifact(artifact_bytes, (unsigned int)id);

    /* Valid header + stub payload should return FORGE_OK and a non-NULL handle. */
    result = forge_artifact_load(id, artifact_bytes, sizeof(artifact_bytes), &art);

    if (result != FORGE_OK)
    {
        fprintf(stderr,
            "FAIL: stub load expected FORGE_OK, got %d (%s)\n",
            (int)result, forge_last_error_string());
        return 1;
    }

    if (!art)
    {
        fprintf(stderr, "FAIL: stub load returned FORGE_OK but out_artifact is NULL\n");
        return 1;
    }

    result = forge_artifact_info(art, &info);

    if (result != FORGE_OK)
    {
        fprintf(stderr,
            "FAIL: forge_artifact_info expected FORGE_OK, got %d\n",
            (int)result);
        forge_artifact_unload(art);
        return 1;
    }

    if (info.backend_id != id ||
        info.format_version_major != 0 ||
        info.format_version_minor != 3 ||
        info.payload_size != 4 ||
        info.placeholder_flags != 1 ||
        info.required_extension_mask != 0u ||
        info.requires_advanced_controls != 0u ||
        info.requires_native_state_read != 0u ||
        info.requires_native_inputs != 0u ||
        info.source_size != sizeof(artifact_bytes))
    {
        fprintf(stderr, "FAIL: forge_artifact_info returned unexpected metadata\n");
        forge_artifact_unload(art);
        return 1;
    }

    /* Unload must succeed. */
    result = forge_artifact_unload(art);
    art = NULL;

    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: forge_artifact_unload returned %d\n", (int)result);
        return 1;
    }

    /* Unknown backend ID must be rejected before reaching the data check. */
    result = forge_artifact_load(
        (ForgeBackendId)0xFFFF, artifact_bytes, sizeof(artifact_bytes), &art);

    if (result != FORGE_ERR_BACKEND_UNAVAILABLE)
    {
        fprintf(stderr,
            "FAIL: unknown backend expected FORGE_ERR_BACKEND_UNAVAILABLE, got %d\n",
            (int)result);
        return 1;
    }

    memcpy(bad_magic, artifact_bytes, sizeof(artifact_bytes));
    bad_magic[0] = 0x00;
    result = forge_artifact_load(id, bad_magic, sizeof(bad_magic), &art);
    if (result != FORGE_ERR_ARTIFACT_INCOMPATIBLE)
    {
        fprintf(stderr,
            "FAIL: bad magic expected FORGE_ERR_ARTIFACT_INCOMPATIBLE, got %d\n",
            (int)result);
        return 1;
    }

    memcpy(backend_mismatch, artifact_bytes, sizeof(artifact_bytes));
    fill_stub_artifact(backend_mismatch, (unsigned int)other_id);
    result = forge_artifact_load(id, backend_mismatch, sizeof(backend_mismatch), &art);
    if (result != FORGE_ERR_ARTIFACT_INCOMPATIBLE)
    {
        fprintf(stderr,
            "FAIL: backend mismatch expected FORGE_ERR_ARTIFACT_INCOMPATIBLE, got %d\n",
            (int)result);
        return 1;
    }

    memcpy(unsupported_payload, artifact_bytes, sizeof(artifact_bytes));
    unsupported_payload[sizeof(StrataPlaceholderArtifactHeader) +
        ((StrataPlaceholderArtifactHeader*)unsupported_payload)->descriptor_bytes] = 0x00;
    unsupported_payload[sizeof(StrataPlaceholderArtifactHeader) +
        ((StrataPlaceholderArtifactHeader*)unsupported_payload)->descriptor_bytes + 1] = 0x01;
    unsupported_payload[sizeof(StrataPlaceholderArtifactHeader) +
        ((StrataPlaceholderArtifactHeader*)unsupported_payload)->descriptor_bytes + 2] = 0x02;
    unsupported_payload[sizeof(StrataPlaceholderArtifactHeader) +
        ((StrataPlaceholderArtifactHeader*)unsupported_payload)->descriptor_bytes + 3] = 0x03;
    result = forge_artifact_load(id, unsupported_payload, sizeof(unsupported_payload), &art);
    if (result != FORGE_ERR_UNSUPPORTED)
    {
        fprintf(stderr,
            "FAIL: unsupported payload expected FORGE_ERR_UNSUPPORTED, got %d\n",
            (int)result);
        return 1;
    }

    memcpy(bad_magic, artifact_bytes, sizeof(artifact_bytes));
    bad_header = (StrataPlaceholderArtifactHeader *)bad_magic;
    bad_header->payload_size = 99;
    result = forge_artifact_load(id, bad_magic, sizeof(bad_magic), &art);
    if (result != FORGE_ERR_ARTIFACT_INCOMPATIBLE)
    {
        fprintf(stderr,
            "FAIL: payload size mismatch expected FORGE_ERR_ARTIFACT_INCOMPATIBLE, got %d\n",
            (int)result);
        return 1;
    }

    memcpy(bad_magic, artifact_bytes, sizeof(artifact_bytes));
    bad_header = (StrataPlaceholderArtifactHeader *)bad_magic;
    bad_header->admission_info.requires_advanced_controls = 1u;
    result = forge_artifact_load(id, bad_magic, sizeof(bad_magic), &art);
    if (result != FORGE_ERR_ARTIFACT_INCOMPATIBLE)
    {
        fprintf(stderr,
            "FAIL: mismatched admission manifest expected FORGE_ERR_ARTIFACT_INCOMPATIBLE, got %d\n",
            (int)result);
        return 1;
    }

    memcpy(bad_descriptor_bytes, artifact_bytes, sizeof(artifact_bytes));
    bad_header = (StrataPlaceholderArtifactHeader *)bad_descriptor_bytes;
    bad_header->descriptor_bytes -= sizeof(StrataPlaceholderSerializedDescriptor);
    result = forge_artifact_load(id, bad_descriptor_bytes, sizeof(bad_descriptor_bytes), &art);
    if (result != FORGE_ERR_ARTIFACT_INCOMPATIBLE)
    {
        fprintf(stderr,
            "FAIL: descriptor_bytes mismatch expected FORGE_ERR_ARTIFACT_INCOMPATIBLE, got %d\n",
            (int)result);
        return 1;
    }

    memcpy(bad_descriptor_class, artifact_bytes, sizeof(artifact_bytes));
    bad_descriptors = (StrataPlaceholderSerializedDescriptor*)
        (((unsigned char*)bad_descriptor_class) + sizeof(StrataPlaceholderArtifactHeader));
    bad_descriptors[0].class_type = FORGE_DESCRIPTOR_CLASS_OUTPUT;
    result = forge_artifact_load(id, bad_descriptor_class, sizeof(bad_descriptor_class), &art);
    if (result != FORGE_ERR_ARTIFACT_INCOMPATIBLE)
    {
        fprintf(stderr,
            "FAIL: malformed descriptor class expected FORGE_ERR_ARTIFACT_INCOMPATIBLE, got %d\n",
            (int)result);
        return 1;
    }

    memcpy(bad_descriptor_name, artifact_bytes, sizeof(artifact_bytes));
    bad_descriptors = (StrataPlaceholderSerializedDescriptor*)
        (((unsigned char*)bad_descriptor_name) + sizeof(StrataPlaceholderArtifactHeader));
    memset(bad_descriptors[0].name, 'A', sizeof(bad_descriptors[0].name));
    result = forge_artifact_load(id, bad_descriptor_name, sizeof(bad_descriptor_name), &art);
    if (result != FORGE_ERR_ARTIFACT_INCOMPATIBLE)
    {
        fprintf(stderr,
            "FAIL: unterminated descriptor name expected FORGE_ERR_ARTIFACT_INCOMPATIBLE, got %d\n",
            (int)result);
        return 1;
    }

    printf("PASS: test_artifact_stub\n");
    return 0;
}
