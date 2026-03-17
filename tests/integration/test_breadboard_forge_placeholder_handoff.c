/*
 * test_breadboard_forge_placeholder_handoff.c
 *
 * Verify the temporary placeholder export path from Breadboard into the
 * Forge stub artifact loader.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/breadboard_api.h"
#include "../../include/forge_api.h"
#include "../../include/strata_placeholder_artifact.h"

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

static int
build_and_export_draft(
    BreadboardTarget target,
    BreadboardDescriptor *out_input_descriptor,
    BreadboardDescriptor *out_output_descriptor,
    BreadboardDescriptor *out_probe_descriptor,
    unsigned char **out_bytes,
    size_t *out_size)
{
    BreadboardModule *module;
    BreadboardArtifactDraft *draft;
    BreadboardCompileOptions options;
    BreadboardResult bb_result;
    unsigned char *buffer;
    size_t size;

    module = NULL;
    draft = NULL;
    options.allow_placeholders = true;
    options.deny_approximation = false;
    options.strict_projection = false;

    if (breadboard_module_create(&module) != BREADBOARD_OK)
    {
        return 0;
    }

    if (breadboard_module_set_target(module, target) != BREADBOARD_OK)
    {
        breadboard_module_free(module);
        return 0;
    }

    if (breadboard_module_compile(module, &options, &draft) != BREADBOARD_OK || !draft)
    {
        breadboard_module_free(module);
        return 0;
    }

    if (out_input_descriptor &&
        breadboard_draft_input_descriptor_at(draft, 0u, out_input_descriptor) != BREADBOARD_OK)
    {
        breadboard_artifact_draft_free(draft);
        breadboard_module_free(module);
        return 0;
    }

    if (out_output_descriptor &&
        breadboard_draft_output_descriptor_at(draft, 0u, out_output_descriptor) != BREADBOARD_OK)
    {
        breadboard_artifact_draft_free(draft);
        breadboard_module_free(module);
        return 0;
    }

    if (out_probe_descriptor &&
        breadboard_draft_probe_descriptor_at(draft, 0u, out_probe_descriptor) != BREADBOARD_OK)
    {
        breadboard_artifact_draft_free(draft);
        breadboard_module_free(module);
        return 0;
    }

    bb_result = breadboard_artifact_draft_export_placeholder_size(draft, &size);
    if (bb_result != BREADBOARD_OK)
    {
        breadboard_artifact_draft_free(draft);
        breadboard_module_free(module);
        return 0;
    }

    buffer = (unsigned char*)malloc(size);
    if (!buffer)
    {
        breadboard_artifact_draft_free(draft);
        breadboard_module_free(module);
        return 0;
    }

    bb_result = breadboard_artifact_draft_export_placeholder(
        draft,
        buffer,
        size,
        out_size);

    breadboard_artifact_draft_free(draft);
    breadboard_module_free(module);

    if (bb_result != BREADBOARD_OK)
    {
        free(buffer);
        return 0;
    }

    *out_bytes = buffer;
    return 1;
}

int
main(void)
{
    ForgeBackendId lxs_id;
    ForgeBackendId highz_id;
    unsigned char *bytes;
    size_t size;
    ForgeArtifact *artifact;
    ForgeArtifactInfo info;
    ForgeDescriptor forge_descriptor;
    BreadboardDescriptor breadboard_input;
    BreadboardDescriptor breadboard_output;
    BreadboardDescriptor breadboard_probe;
    ForgeResult result;
    const StrataPlaceholderArtifactHeader *header;

    lxs_id = find_backend_id_by_name("LXS");
    highz_id = find_backend_id_by_name("HighZ");

    if (lxs_id == FORGE_BACKEND_ID_INVALID || highz_id == FORGE_BACKEND_ID_INVALID)
    {
        fprintf(stderr, "FAIL: could not resolve backend IDs for handoff test\n");
        return 1;
    }

    if (forge_install_product_profile(FORGE_PRODUCT_PROFILE_UNRESTRICTED) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not install unrestricted profile\n");
        return 1;
    }

    bytes = NULL;
    size = 0u;
    if (!build_and_export_draft(
        BREADBOARD_TARGET_FAST_4STATE,
        &breadboard_input,
        &breadboard_output,
        &breadboard_probe,
        &bytes,
        &size))
    {
        fprintf(stderr, "FAIL: could not build/export FAST_4STATE draft\n");
        return 1;
    }

    header = (const StrataPlaceholderArtifactHeader *)bytes;
    if (header->payload_kind != STRATA_PLACEHOLDER_PAYLOAD_BASELINE ||
        header->admission_info.requirement_flags != STRATA_PLACEHOLDER_REQUIREMENT_NONE)
    {
        free(bytes);
        fprintf(stderr, "FAIL: FAST_4STATE export manifest mismatch\n");
        return 1;
    }

    artifact = NULL;
    result = forge_artifact_load(lxs_id, bytes, size, &artifact);
    free(bytes);

    if (result != FORGE_OK || !artifact)
    {
        fprintf(stderr, "FAIL: FAST_4STATE placeholder handoff should load, got %d\n",
            (int)result);
        return 1;
    }

    result = forge_artifact_info(artifact, &info);
    if (result != FORGE_OK ||
        info.requires_advanced_controls != 0u ||
        info.requires_native_state_read != 0u ||
        info.requires_native_inputs != 0u)
    {
        fprintf(stderr, "FAIL: FAST_4STATE handoff metadata mismatch\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_input_descriptor_at(artifact, 0u, &forge_descriptor);
    if (result != FORGE_OK ||
        forge_descriptor.id != breadboard_input.id ||
        strcmp(forge_descriptor.name, breadboard_input.name) != 0 ||
        forge_descriptor.width != breadboard_input.width)
    {
        fprintf(stderr, "FAIL: FAST_4STATE input descriptor handoff mismatch\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_output_descriptor_at(artifact, 0u, &forge_descriptor);
    if (result != FORGE_OK ||
        forge_descriptor.id != breadboard_output.id ||
        strcmp(forge_descriptor.name, breadboard_output.name) != 0 ||
        forge_descriptor.width != breadboard_output.width)
    {
        fprintf(stderr, "FAIL: FAST_4STATE output descriptor handoff mismatch\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_probe_descriptor_at(artifact, 0u, &forge_descriptor);
    if (result != FORGE_OK ||
        forge_descriptor.id != breadboard_probe.id ||
        strcmp(forge_descriptor.name, breadboard_probe.name) != 0 ||
        forge_descriptor.width != breadboard_probe.width)
    {
        fprintf(stderr, "FAIL: FAST_4STATE probe descriptor handoff mismatch\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_output_descriptor_at(artifact, 0u, &forge_descriptor);
    if (result != FORGE_OK ||
        forge_descriptor.id != breadboard_output.id ||
        strcmp(forge_descriptor.name, breadboard_output.name) != 0 ||
        forge_descriptor.width != breadboard_output.width)
    {
        free(bytes);
        fprintf(stderr, "FAIL: TEMPORAL output descriptor handoff mismatch\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_probe_descriptor_at(artifact, 0u, &forge_descriptor);
    if (result != FORGE_OK ||
        forge_descriptor.id != breadboard_probe.id ||
        strcmp(forge_descriptor.name, breadboard_probe.name) != 0 ||
        forge_descriptor.width != breadboard_probe.width)
    {
        free(bytes);
        fprintf(stderr, "FAIL: TEMPORAL probe descriptor handoff mismatch\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    if (forge_artifact_unload(artifact) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not unload FAST_4STATE artifact\n");
        return 1;
    }

    bytes = NULL;
    size = 0u;
    if (!build_and_export_draft(
        BREADBOARD_TARGET_TEMPORAL,
        &breadboard_input,
        &breadboard_output,
        &breadboard_probe,
        &bytes,
        &size))
    {
        fprintf(stderr, "FAIL: could not build/export TEMPORAL draft\n");
        return 1;
    }

    header = (const StrataPlaceholderArtifactHeader *)bytes;
    if (header->payload_kind != STRATA_PLACEHOLDER_PAYLOAD_ADVANCED ||
        header->admission_info.requirement_flags !=
            STRATA_PLACEHOLDER_REQUIREMENT_ADVANCED_CONTROL ||
        !header->admission_info.requires_advanced_controls)
    {
        free(bytes);
        fprintf(stderr, "FAIL: TEMPORAL export manifest mismatch\n");
        return 1;
    }

    artifact = NULL;
    result = forge_artifact_load(highz_id, bytes, size, &artifact);

    if (result != FORGE_OK || !artifact)
    {
        free(bytes);
        fprintf(stderr, "FAIL: TEMPORAL placeholder handoff should load unrestricted, got %d\n",
            (int)result);
        return 1;
    }

    result = forge_artifact_info(artifact, &info);
    if (result != FORGE_OK || !info.requires_advanced_controls)
    {
        free(bytes);
        fprintf(stderr, "FAIL: TEMPORAL handoff metadata mismatch\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_input_descriptor_at(artifact, 0u, &forge_descriptor);
    if (result != FORGE_OK ||
        forge_descriptor.id != breadboard_input.id ||
        strcmp(forge_descriptor.name, breadboard_input.name) != 0 ||
        forge_descriptor.width != breadboard_input.width)
    {
        free(bytes);
        fprintf(stderr, "FAIL: TEMPORAL input descriptor handoff mismatch\n");
        forge_artifact_unload(artifact);
        return 1;
    }

    if (forge_artifact_unload(artifact) != FORGE_OK)
    {
        free(bytes);
        fprintf(stderr, "FAIL: could not unload TEMPORAL artifact\n");
        return 1;
    }

    if (forge_install_product_profile(FORGE_PRODUCT_PROFILE_COMMON_ONLY) != FORGE_OK)
    {
        free(bytes);
        fprintf(stderr, "FAIL: could not install common-only profile\n");
        return 1;
    }

    artifact = NULL;
    result = forge_artifact_load(highz_id, bytes, size, &artifact);
    free(bytes);

    if (result != FORGE_ERR_FORBIDDEN)
    {
        fprintf(stderr, "FAIL: TEMPORAL placeholder handoff should be forbidden under common-only, got %d\n",
            (int)result);
        if (artifact)
        {
            forge_artifact_unload(artifact);
        }
        return 1;
    }

    if (forge_install_product_profile(FORGE_PRODUCT_PROFILE_UNRESTRICTED) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not restore unrestricted profile at end of handoff test\n");
        return 1;
    }

    printf("PASS: test_breadboard_forge_placeholder_handoff\n");
    return 0;
}
