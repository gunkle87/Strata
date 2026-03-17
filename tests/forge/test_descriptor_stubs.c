/*
 * test_descriptor_stubs.c
 *
 * Verify that the descriptor-first Forge surface exists and returns stable
 * placeholder metadata from a loaded stub artifact, while probe reads on the
 * live session still fail honestly with FORGE_ERR_UNSUPPORTED.
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
    ForgeDescriptor descriptor;
    ForgeProbeValue probe_values[1];
    unsigned char artifact_bytes[sizeof(TestArtifactHeader) + 4];
    uint32_t count;
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

    result = forge_output_descriptor_count(artifact, &count);

    if (result != FORGE_OK || count != 2u)
    {
        fprintf(stderr,
            "FAIL: forge_output_descriptor_count expected FORGE_OK and count 2, got %d / %u\n",
            (int)result, count);
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_output_descriptor_at(artifact, 0u, &descriptor);

    if (result != FORGE_OK ||
        descriptor.descriptor_class != FORGE_DESCRIPTOR_CLASS_OUTPUT ||
        descriptor.id != 1u ||
        descriptor.width != 1u)
    {
        fprintf(stderr, "FAIL: forge_output_descriptor_at returned unexpected descriptor\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_output_descriptor_by_id(artifact, 2u, &descriptor);

    if (result != FORGE_OK ||
        descriptor.descriptor_class != FORGE_DESCRIPTOR_CLASS_OUTPUT ||
        strcmp(descriptor.name, "out1") != 0)
    {
        fprintf(stderr, "FAIL: forge_output_descriptor_by_id returned unexpected descriptor\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_output_descriptor_by_name(artifact, "out0", &descriptor);

    if (result != FORGE_OK ||
        descriptor.id != 1u ||
        descriptor.descriptor_class != FORGE_DESCRIPTOR_CLASS_OUTPUT)
    {
        fprintf(stderr, "FAIL: forge_output_descriptor_by_name returned unexpected descriptor\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_output_descriptor_at(artifact, count, &descriptor);

    if (result != FORGE_ERR_OUT_OF_BOUNDS)
    {
        fprintf(stderr,
            "FAIL: forge_output_descriptor_at expected FORGE_ERR_OUT_OF_BOUNDS, got %d\n",
            (int)result);
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_output_descriptor_by_id(artifact, 999u, &descriptor);

    if (result != FORGE_ERR_OUT_OF_BOUNDS)
    {
        fprintf(stderr,
            "FAIL: forge_output_descriptor_by_id expected FORGE_ERR_OUT_OF_BOUNDS, got %d\n",
            (int)result);
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_output_descriptor_by_name(artifact, "missing_output", &descriptor);

    if (result != FORGE_ERR_OUT_OF_BOUNDS)
    {
        fprintf(stderr,
            "FAIL: forge_output_descriptor_by_name expected FORGE_ERR_OUT_OF_BOUNDS, got %d\n",
            (int)result);
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_probe_descriptor_count(artifact, &count);

    if (result != FORGE_OK || count != 1u)
    {
        fprintf(stderr,
            "FAIL: forge_probe_descriptor_count expected FORGE_OK and count 1, got %d / %u\n",
            (int)result, count);
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_probe_descriptor_at(artifact, 0u, &descriptor);

    if (result != FORGE_OK ||
        descriptor.descriptor_class != FORGE_DESCRIPTOR_CLASS_PROBE ||
        descriptor.id != 1001u ||
        descriptor.width != 64u)
    {
        fprintf(stderr, "FAIL: forge_probe_descriptor_at returned unexpected descriptor\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_probe_descriptor_by_id(artifact, 1001u, &descriptor);

    if (result != FORGE_OK ||
        descriptor.descriptor_class != FORGE_DESCRIPTOR_CLASS_PROBE ||
        strcmp(descriptor.name, "probe_step_count") != 0)
    {
        fprintf(stderr, "FAIL: forge_probe_descriptor_by_id returned unexpected descriptor\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_probe_descriptor_by_name(artifact, "probe_step_count", &descriptor);

    if (result != FORGE_OK ||
        descriptor.id != 1001u ||
        descriptor.descriptor_class != FORGE_DESCRIPTOR_CLASS_PROBE)
    {
        fprintf(stderr, "FAIL: forge_probe_descriptor_by_name returned unexpected descriptor\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_probe_descriptor_by_id(artifact, 9999u, &descriptor);

    if (result != FORGE_ERR_OUT_OF_BOUNDS)
    {
        fprintf(stderr,
            "FAIL: forge_probe_descriptor_by_id expected FORGE_ERR_OUT_OF_BOUNDS, got %d\n",
            (int)result);
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_probe_descriptor_by_name(artifact, "missing_probe", &descriptor);

    if (result != FORGE_ERR_OUT_OF_BOUNDS)
    {
        fprintf(stderr,
            "FAIL: forge_probe_descriptor_by_name expected FORGE_ERR_OUT_OF_BOUNDS, got %d\n",
            (int)result);
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_session_create(artifact, &session);

    if (result != FORGE_OK || !session)
    {
        fprintf(stderr, "FAIL: could not create stub session: %d\n", (int)result);
        forge_artifact_unload(artifact);
        return 1;
    }

    probe_values[0].probe_id = 1001u;
    probe_values[0].value = 0u;

    result = forge_read_probes(session, probe_values, 1u);

    if (result != FORGE_ERR_UNSUPPORTED)
    {
        fprintf(stderr,
            "FAIL: forge_read_probes expected FORGE_ERR_UNSUPPORTED, got %d\n",
            (int)result);
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

    printf("PASS: test_descriptor_stubs\n");
    return 0;
}
