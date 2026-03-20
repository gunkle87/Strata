/*
 * test_projection_contract.c
 *
 * Validate both the projection data contracts and the Forge-visible reporting
 * surface that exposes Breadboard-emitted projection metadata.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/breadboard_api.h"
#include "../../include/forge_api.h"
#include "../../include/strata_projection.h"

static void
fail(const char* message)
{
    fprintf(stderr, "FAIL: %s\n", message);
    exit(1);
}

static BreadboardModule*
build_plain_fast_module(void)
{
    BreadboardModule* module = NULL;
    BreadboardDescriptorSpec input_spec = { 101u, "plain_in", 1u };
    BreadboardDescriptorSpec output_spec = { 102u, "plain_out", 1u };
    BreadboardComponentSpec buf_spec = { 103u, "BUF", false };
    BreadboardExecutableConnectionSpec in_to_buf = {
        { BREADBOARD_ENDPOINT_MODULE_INPUT_SOURCE, 101u, 0u, 0u },
        { BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK, 0u, 103u, 0u }
    };
    BreadboardExecutableConnectionSpec buf_to_out = {
        { BREADBOARD_ENDPOINT_COMPONENT_OUTPUT_SOURCE, 0u, 103u, 0u },
        { BREADBOARD_ENDPOINT_MODULE_OUTPUT_SINK, 102u, 0u, 0u }
    };

    if (breadboard_module_create(&module) != BREADBOARD_OK ||
        breadboard_module_set_target(module, BREADBOARD_TARGET_FAST_4STATE) != BREADBOARD_OK ||
        breadboard_module_add_input_descriptor(module, &input_spec) != BREADBOARD_OK ||
        breadboard_module_add_output_descriptor(module, &output_spec) != BREADBOARD_OK ||
        breadboard_module_add_component_instance(module, &buf_spec) != BREADBOARD_OK ||
        breadboard_module_add_executable_connection(module, &in_to_buf) != BREADBOARD_OK ||
        breadboard_module_add_executable_connection(module, &buf_to_out) != BREADBOARD_OK)
    {
        breadboard_module_free(module);
        fail("could not build plain fast Breadboard module");
    }

    return module;
}

static BreadboardModule*
build_strength_projection_module(void)
{
    BreadboardModule* module = build_plain_fast_module();
    BreadboardRequirementProfile profile;
    BreadboardProjectionPolicy policy;

    memset(&profile, 0, sizeof(profile));
    profile.required_projection_families_mask =
        (1u << STRATA_PROJECTION_FAMILY_STRENGTH_DISTINCTION);
    if (breadboard_module_set_requirement_profile(module, &profile) != BREADBOARD_OK)
    {
        breadboard_module_free(module);
        fail("could not set strength projection requirement profile");
    }

    memset(&policy, 0, sizeof(policy));
    policy.allowed_families_mask =
        (1u << STRATA_PROJECTION_FAMILY_STRENGTH_DISTINCTION);
    if (breadboard_module_set_projection_policy(module, &policy) != BREADBOARD_OK)
    {
        breadboard_module_free(module);
        fail("could not set strength projection policy");
    }

    return module;
}

static void
expect_projection_metadata_matches(
    const BreadboardDraftInfo* draft_info,
    const ForgeArtifactInfo* artifact_info)
{
    if (!draft_info || !artifact_info)
    {
        fail("projection metadata comparison received NULL input");
    }

    if (artifact_info->projection_metadata.required_projection_families_mask !=
            draft_info->projection_metadata.required_projection_families_mask ||
        artifact_info->projection_metadata.lowered_projection_families_mask !=
            draft_info->projection_metadata.lowered_projection_families_mask ||
        artifact_info->projection_metadata.projection_occurred !=
            (draft_info->projection_metadata.projection_occurred ? 1u : 0u) ||
        artifact_info->projection_metadata.approximation_occurred !=
            (draft_info->projection_metadata.approximation_occurred ? 1u : 0u))
    {
        fail("Forge projection metadata did not match Breadboard draft metadata");
    }
}

static void
exercise_projection_visibility(
    BreadboardModule* module,
    uint32_t expected_required_mask,
    uint32_t expected_lowered_mask,
    uint32_t expected_projection_occurred,
    uint32_t expected_approximation_occurred)
{
    BreadboardArtifactDraft* draft = NULL;
    BreadboardDraftInfo draft_info;
    BreadboardCompileOptions options;
    unsigned char* bytes = NULL;
    size_t size = 0u;
    ForgeArtifact* artifact = NULL;
    ForgeArtifactInfo artifact_info;
    ForgeBackendId backend_id = FORGE_BACKEND_ID_INVALID;

    memset(&options, 0, sizeof(options));
    options.require_real_executable = true;

    if (breadboard_module_compile(module, &options, &draft) != BREADBOARD_OK || !draft)
    {
        fail("could not compile Breadboard draft for Forge projection visibility test");
    }

    if (breadboard_artifact_draft_query_info(draft, &draft_info) != BREADBOARD_OK)
    {
        breadboard_artifact_draft_free(draft);
        fail("could not query Breadboard draft info");
    }

    if (draft_info.projection_metadata.required_projection_families_mask != expected_required_mask ||
        draft_info.projection_metadata.lowered_projection_families_mask != expected_lowered_mask ||
        draft_info.projection_metadata.projection_occurred !=
            (expected_projection_occurred ? true : false) ||
        draft_info.projection_metadata.approximation_occurred !=
            (expected_approximation_occurred ? true : false))
    {
        breadboard_artifact_draft_free(draft);
        fail("Breadboard draft metadata mismatch before Forge load");
    }

    if (breadboard_artifact_draft_export_fast_size(draft, &size) != BREADBOARD_OK)
    {
        breadboard_artifact_draft_free(draft);
        fail("could not size real fast draft export");
    }

    bytes = (unsigned char*)malloc(size);
    if (!bytes)
    {
        breadboard_artifact_draft_free(draft);
        fail("could not allocate exported artifact bytes");
    }

    if (breadboard_artifact_draft_export_fast(draft, bytes, size, &size) != BREADBOARD_OK)
    {
        free(bytes);
        breadboard_artifact_draft_free(draft);
        fail("could not export real fast draft");
    }

    breadboard_artifact_draft_free(draft);

    if (forge_backend_id_at(0u, &backend_id) != FORGE_OK ||
        backend_id == FORGE_BACKEND_ID_INVALID)
    {
        free(bytes);
        fail("could not resolve Forge backend id");
    }

    if (forge_artifact_load(backend_id, bytes, size, &artifact) != FORGE_OK || !artifact)
    {
        free(bytes);
        fail("Forge failed to load exported artifact");
    }

    free(bytes);

    if (forge_artifact_info(artifact, &artifact_info) != FORGE_OK)
    {
        forge_artifact_unload(artifact);
        fail("Forge failed to query artifact info");
    }

    if (artifact_info.projection_metadata.required_projection_families_mask != expected_required_mask ||
        artifact_info.projection_metadata.lowered_projection_families_mask != expected_lowered_mask ||
        artifact_info.projection_metadata.projection_occurred != expected_projection_occurred ||
        artifact_info.projection_metadata.approximation_occurred != expected_approximation_occurred)
    {
        forge_artifact_unload(artifact);
        fail("Forge artifact info projection metadata mismatch");
    }

    expect_projection_metadata_matches(&draft_info, &artifact_info);

    if (forge_artifact_unload(artifact) != FORGE_OK)
    {
        fail("Forge failed to unload artifact");
    }
}

int main(void)
{
    BreadboardModule* no_projection_module = NULL;
    BreadboardModule* projected_module = NULL;

    /* Verify enums have distinct values. */
    if (STRATA_PROJECTION_FAMILY_INITIALIZATION != 1)
    {
        fprintf(stderr, "FAIL: STRATA_PROJECTION_FAMILY_INITIALIZATION != 1\n");
        return 1;
    }

    if (STRATA_PROJECTION_FAMILY_STRENGTH_DISTINCTION != 2)
    {
        fprintf(stderr, "FAIL: STRATA_PROJECTION_FAMILY_STRENGTH_DISTINCTION != 2\n");
        return 1;
    }

    if (STRATA_PROJECTION_FAMILY_BACKEND_SPECIFIC != 3)
    {
        fprintf(stderr, "FAIL: STRATA_PROJECTION_FAMILY_BACKEND_SPECIFIC != 3\n");
        return 1;
    }

    if (STRATA_APPROX_CATEGORY_PROJECTION != 1)
    {
        fprintf(stderr, "FAIL: STRATA_APPROX_CATEGORY_PROJECTION != 1\n");
        return 1;
    }

    if (STRATA_APPROX_CATEGORY_COLLAPSE != 2)
    {
        fprintf(stderr, "FAIL: STRATA_APPROX_CATEGORY_COLLAPSE != 2\n");
        return 1;
    }

    if (STRATA_APPROX_CATEGORY_RESOLUTION != 3)
    {
        fprintf(stderr, "FAIL: STRATA_APPROX_CATEGORY_RESOLUTION != 3\n");
        return 1;
    }

    if (STRATA_PORTABLE_STATE_0 != 0 ||
        STRATA_PORTABLE_STATE_1 != 1 ||
        STRATA_PORTABLE_STATE_X != 2 ||
        STRATA_PORTABLE_STATE_Z != 3)
    {
        fprintf(stderr, "FAIL: portable state enum values mismatch\n");
        return 1;
    }

    if (sizeof(StrataProjectionOutcome) == 0 || sizeof(StrataProjectionReport) == 0)
    {
        fprintf(stderr, "FAIL: projection data contract struct size is zero\n");
        return 1;
    }

    if (offsetof(StrataProjectionOutcome, target_backend_id) != 0)
    {
        fprintf(stderr, "FAIL: target_backend_id offset != 0\n");
        return 1;
    }

    if (sizeof(((StrataProjectionReport*)0)->outcomes) / sizeof(StrataProjectionOutcome) != 8)
    {
        fprintf(stderr, "FAIL: outcomes array size mismatch\n");
        return 1;
    }

    if (STRATA_PROJECTION_SUMMARY_INITIALIZATION_PROJECTED != (1u << 0) ||
        STRATA_PROJECTION_SUMMARY_STRENGTH_COLLAPSED != (1u << 1) ||
        STRATA_PROJECTION_SUMMARY_BACKEND_SPECIFIC_LOST != (1u << 2) ||
        STRATA_PROJECTION_SUMMARY_ANY_SEMANTIC_LOSS != (1u << 3))
    {
        fprintf(stderr, "FAIL: summary flag values mismatch\n");
        return 1;
    }

    if (forge_install_product_profile(FORGE_PRODUCT_PROFILE_UNRESTRICTED) != FORGE_OK)
    {
        fail("could not install unrestricted Forge profile");
    }

    no_projection_module = build_plain_fast_module();
    exercise_projection_visibility(no_projection_module, 0u, 0u, 0u, 0u);
    breadboard_module_free(no_projection_module);

    projected_module = build_strength_projection_module();
    exercise_projection_visibility(
        projected_module,
        (1u << STRATA_PROJECTION_FAMILY_STRENGTH_DISTINCTION),
        (1u << STRATA_PROJECTION_FAMILY_STRENGTH_DISTINCTION),
        1u,
        1u);
    breadboard_module_free(projected_module);

    printf("PASS: projection data contracts and Forge visibility compile correctly\n");
    return 0;
}