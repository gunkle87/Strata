#include "breadboard_api.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * test_breadboard_skeleton.c
 *
 * Smoke test verifying the initial Breadboard skeleton builds and that the
 * minimal API shapes map to their expected behaviors.
 */

static void print_result(const char* test_name, BreadboardResult res, BreadboardResult expected)
{
    if (res == expected)
    {
        printf("[PASS] %s\n", test_name);
    }
    else
    {
        printf("[FAIL] %s (Expected %d, got %d)\n", test_name, expected, res);
        exit(1);
    }
}

int main(void)
{
    printf("Starting Breadboard Skeleton Tests...\n");

    BreadboardModule* module = NULL;
    BreadboardResult res;

    /* 1. Test create */
    res = breadboard_module_create(&module);
    print_result("module_create", res, BREADBOARD_OK);

    if (!module)
    {
        printf("[FAIL] module pointer is NULL\n");
        exit(1);
    }

    /* 2. Test invalid arguments */
    res = breadboard_module_create(NULL);
    print_result("module_create(NULL)", res, BREADBOARD_ERR_INVALID_ARGUMENT);

    /* 3. Test target selection */
    res = breadboard_module_set_target(module, BREADBOARD_TARGET_FAST_4STATE);
    print_result("module_set_target(FAST_4STATE)", res, BREADBOARD_OK);

    res = breadboard_module_set_target(module, (BreadboardTarget)999);
    print_result("module_set_target(INVALID)", res, BREADBOARD_ERR_INVALID_TARGET);

    /* Test Target Policy Gating */
    res = breadboard_module_set_target_policy(module, BREADBOARD_TARGET_MASK_TEMPORAL);
    print_result("module_set_target_policy(TEMPORAL only)", res, BREADBOARD_OK);

    bool is_available = true;
    res = breadboard_module_query_target_availability(module, BREADBOARD_TARGET_FAST_4STATE, &is_available);
    print_result("module_query_target_availability(FAST_4STATE)", res, BREADBOARD_OK);
    if (is_available) { printf("[FAIL] FAST_4STATE should be denied\n"); exit(1); }

    res = breadboard_module_query_target_availability(module, BREADBOARD_TARGET_TEMPORAL, &is_available);
    print_result("module_query_target_availability(TEMPORAL)", res, BREADBOARD_OK);
    if (!is_available) { printf("[FAIL] TEMPORAL should be allowed\n"); exit(1); }

    /* Try to compile with a denied target (FAST_4STATE is active, but policy is TEMPORAL only) */
    BreadboardCompileOptions opts_compile_denied = { .allow_placeholders = true, .deny_approximation = false, .strict_projection = false };
    BreadboardArtifactDraft* draft = NULL;
    res = breadboard_module_compile(module, &opts_compile_denied, &draft);
    print_result("module_compile(denied target)", res, BREADBOARD_ERR_COMPILE_FAILED);

    res = breadboard_module_query_target_availability(module, (BreadboardTarget)999, &is_available);
    print_result("module_query_target_availability(INVALID)", res, BREADBOARD_ERR_INVALID_TARGET);

    res = breadboard_module_set_target_policy(module, BREADBOARD_TARGET_MASK_ALL | (1u << 31));
    print_result("module_set_target_policy(INVALID_MASK)", res, BREADBOARD_ERR_INVALID_ARGUMENT);

    /* Restore to temporal target just to be sure */
    breadboard_module_set_target(module, BREADBOARD_TARGET_TEMPORAL);

    /* Test get target */
    BreadboardTarget queried_target = BREADBOARD_TARGET_UNSPECIFIED;
    res = breadboard_module_get_target(module, &queried_target);
    print_result("module_get_target", res, BREADBOARD_OK);
    if (queried_target != BREADBOARD_TARGET_TEMPORAL)
    {
        printf("[FAIL] module target mismatch (Expected %d, got %d)\n",
               BREADBOARD_TARGET_TEMPORAL, queried_target);
        exit(1);
    }

    /* Test module query target info */
    BreadboardTargetInfo target_info;
    res = breadboard_module_query_target_info(module, &target_info);
    print_result("module_query_target_info", res, BREADBOARD_OK);
    if (target_info.target != BREADBOARD_TARGET_TEMPORAL)
    {
        printf("[FAIL] target info mismatch (Expected %d, got %d)\n",
               BREADBOARD_TARGET_TEMPORAL, target_info.target);
        exit(1);
    }

    /* 4. Test compilation refusal (no placeholder flag) */
    BreadboardCompileOptions opts_none = { .allow_placeholders = false, .deny_approximation = false, .strict_projection = false };
    res = breadboard_module_compile(module, &opts_none, &draft);
    print_result("module_compile(no_options)", res, BREADBOARD_ERR_COMPILE_FAILED);

    /* 5. Test invalid arguments for compile */
    res = breadboard_module_compile(NULL, &opts_none, &draft);
    print_result("module_compile(NULL, ...)", res, BREADBOARD_ERR_INVALID_ARGUMENT);

    /* 6. Test compilation with explicit placeholders allowed */
    BreadboardCompileOptions opts_allow = { .allow_placeholders = true, .deny_approximation = false, .strict_projection = false };
    res = breadboard_module_compile(module, &opts_allow, &draft);
    print_result("module_compile(allow_placeholders)", res, BREADBOARD_OK);

    if (!draft)
    {
        printf("[FAIL] draft pointer is NULL\n");
        exit(1);
    }

    /* 7. Artifact metadata check */
    queried_target = BREADBOARD_TARGET_UNSPECIFIED;
    res = breadboard_artifact_draft_query_metadata(draft, &queried_target);
    print_result("draft_query_metadata", res, BREADBOARD_OK);

    if (queried_target != BREADBOARD_TARGET_TEMPORAL)
    {
        printf("[FAIL] draft target mismatch (Expected %d, got %d)\n",
               BREADBOARD_TARGET_TEMPORAL, queried_target);
        exit(1);
    }

    /* 8. Artifact draft info check */
    BreadboardDraftInfo draft_info;
    res = breadboard_artifact_draft_query_info(draft, &draft_info);
    print_result("draft_query_info", res, BREADBOARD_OK);
    if (draft_info.target != BREADBOARD_TARGET_TEMPORAL || !draft_info.has_placeholders)
    {
        printf("[FAIL] draft info mismatch\n");
        exit(1);
    }

    /* 9. Test diagnostics */
    size_t diag_count = 99;
    res = breadboard_module_get_diagnostic_count(module, &diag_count);
    print_result("module_get_diagnostic_count", res, BREADBOARD_OK);
    /* We expect 3 diagnostics: denied target, compilation refusal without placeholders, and warning from successful compile */
    if (diag_count != 3)
    {
        printf("[FAIL] expected 3 diagnostics, got %zu\n", diag_count);
        exit(1);
    }

    BreadboardDiagnostic diag;
    res = breadboard_module_get_diagnostic(module, 0, &diag);
    print_result("module_get_diagnostic(0)", res, BREADBOARD_OK);
    if (diag.severity != BREADBOARD_DIAG_ERROR || diag.code != BREADBOARD_DIAG_CODE_TARGET_DENIED_BY_POLICY)
    {
        printf("[FAIL] diagnostic 0 mismatch\n");
        exit(1);
    }

    res = breadboard_module_get_diagnostic(module, 1, &diag);
    print_result("module_get_diagnostic(1)", res, BREADBOARD_OK);
    if (diag.severity != BREADBOARD_DIAG_ERROR || diag.code != BREADBOARD_DIAG_CODE_UNSUPPORTED_CONSTRUCT)
    {
        printf("[FAIL] diagnostic 1 mismatch\n");
        exit(1);
    }

    res = breadboard_module_get_last_diagnostic(module, &diag);
    print_result("module_get_last_diagnostic", res, BREADBOARD_OK);
    if (diag.severity != BREADBOARD_DIAG_WARNING || diag.code != BREADBOARD_DIAG_CODE_NONE)
    {
        printf("[FAIL] diagnostic last mismatch\n");
        exit(1);
    }

    /* 10. Descriptor count checks */
    size_t in_count = 0, out_count = 0, probe_count = 0;
    res = breadboard_draft_input_descriptor_count(draft, &in_count);
    print_result("draft_input_descriptor_count", res, BREADBOARD_OK);
    if (in_count != 2) { printf("[FAIL] Expected 2 input descriptors\n"); exit(1); }

    res = breadboard_draft_output_descriptor_count(draft, &out_count);
    print_result("draft_output_descriptor_count", res, BREADBOARD_OK);
    if (out_count != 2) { printf("[FAIL] Expected 2 output descriptors\n"); exit(1); }

    res = breadboard_draft_probe_descriptor_count(draft, &probe_count);
    print_result("draft_probe_descriptor_count", res, BREADBOARD_OK);
    if (probe_count != 1) { printf("[FAIL] Expected 1 probe descriptor\n"); exit(1); }

    /* 11. Descriptor querying by index checks */
    BreadboardDescriptor desc;
    res = breadboard_draft_input_descriptor_at(draft, 1, &desc);
    print_result("draft_input_descriptor_at(1)", res, BREADBOARD_OK);
    if (desc.id != 101 || desc.width != 8 || desc.class_type != BREADBOARD_DESC_INPUT || !desc.is_placeholder)
    {
        printf("[FAIL] Input descriptor 1 metadata mismatch\n");
        exit(1);
    }

    res = breadboard_draft_output_descriptor_at(draft, 0, &desc);
    print_result("draft_output_descriptor_at(0)", res, BREADBOARD_OK);
    if (desc.id != 200 || desc.width != 1 || desc.class_type != BREADBOARD_DESC_OUTPUT || !desc.is_placeholder)
    {
        printf("[FAIL] Output descriptor 0 metadata mismatch\n");
        exit(1);
    }

    res = breadboard_draft_probe_descriptor_at(draft, 0, &desc);
    print_result("draft_probe_descriptor_at(0)", res, BREADBOARD_OK);
    if (desc.id != 300 || desc.width != 1 || desc.class_type != BREADBOARD_DESC_PROBE || !desc.is_placeholder)
    {
        printf("[FAIL] Probe descriptor 0 metadata mismatch\n");
        exit(1);
    }

    /* 12. Lookup by ID and Name successful checks */
    BreadboardDescriptor lookup_desc;

    /* Input lookup success */
    res = breadboard_draft_input_descriptor_by_id(draft, 100, &lookup_desc);
    print_result("draft_input_descriptor_by_id(100)", res, BREADBOARD_OK);
    if (lookup_desc.id != 100 || lookup_desc.class_type != BREADBOARD_DESC_INPUT) { printf("[FAIL] Lookup mismatch\n"); exit(1); }

    res = breadboard_draft_input_descriptor_by_name(draft, "placeholder_in_1", &lookup_desc);
    print_result("draft_input_descriptor_by_name('placeholder_in_1')", res, BREADBOARD_OK);
    if (lookup_desc.id != 101 || lookup_desc.class_type != BREADBOARD_DESC_INPUT) { printf("[FAIL] Lookup mismatch\n"); exit(1); }

    /* Output lookup success */
    res = breadboard_draft_output_descriptor_by_id(draft, 201, &lookup_desc);
    print_result("draft_output_descriptor_by_id(201)", res, BREADBOARD_OK);
    if (lookup_desc.id != 201 || lookup_desc.class_type != BREADBOARD_DESC_OUTPUT) { printf("[FAIL] Lookup mismatch\n"); exit(1); }

    res = breadboard_draft_output_descriptor_by_name(draft, "placeholder_out_0", &lookup_desc);
    print_result("draft_output_descriptor_by_name('placeholder_out_0')", res, BREADBOARD_OK);
    if (lookup_desc.id != 200 || lookup_desc.class_type != BREADBOARD_DESC_OUTPUT) { printf("[FAIL] Lookup mismatch\n"); exit(1); }

    /* Probe lookup success */
    res = breadboard_draft_probe_descriptor_by_id(draft, 300, &lookup_desc);
    print_result("draft_probe_descriptor_by_id(300)", res, BREADBOARD_OK);
    if (lookup_desc.id != 300 || lookup_desc.class_type != BREADBOARD_DESC_PROBE) { printf("[FAIL] Lookup mismatch\n"); exit(1); }

    res = breadboard_draft_probe_descriptor_by_name(draft, "placeholder_probe_0", &lookup_desc);
    print_result("draft_probe_descriptor_by_name('placeholder_probe_0')", res, BREADBOARD_OK);
    if (lookup_desc.id != 300 || lookup_desc.class_type != BREADBOARD_DESC_PROBE) { printf("[FAIL] Lookup mismatch\n"); exit(1); }

    /* 13. Lookup by ID and Name failure paths */
    res = breadboard_draft_input_descriptor_by_id(draft, 999, &lookup_desc);
    print_result("draft_input_descriptor_by_id(999)", res, BREADBOARD_ERR_NOT_FOUND);

    res = breadboard_draft_output_descriptor_by_name(draft, "missing_name", &lookup_desc);
    print_result("draft_output_descriptor_by_name('missing_name')", res, BREADBOARD_ERR_NOT_FOUND);

    res = breadboard_draft_probe_descriptor_by_id(draft, 100, &lookup_desc); /* ID exists but wrong class */
    print_result("draft_probe_descriptor_by_id(100)", res, BREADBOARD_ERR_NOT_FOUND);

    /* 14. Out of bounds checking */
    res = breadboard_draft_input_descriptor_at(draft, 99, &desc);
    print_result("draft_input_descriptor_at(99)", res, BREADBOARD_ERR_OUT_OF_BOUNDS);

    /* 15. Invalid handles / argument paths */
    res = breadboard_draft_output_descriptor_count(NULL, &out_count);
    print_result("draft_output_descriptor_count(NULL)", res, BREADBOARD_ERR_INVALID_ARGUMENT);

    res = breadboard_draft_probe_descriptor_at(draft, 0, NULL);
    print_result("draft_probe_descriptor_at(..., NULL)", res, BREADBOARD_ERR_INVALID_ARGUMENT);

    res = breadboard_draft_input_descriptor_by_id(NULL, 100, &lookup_desc);
    print_result("draft_input_descriptor_by_id(NULL, ...)", res, BREADBOARD_ERR_INVALID_ARGUMENT);

    res = breadboard_draft_output_descriptor_by_name(draft, NULL, &lookup_desc);
    print_result("draft_output_descriptor_by_name(..., NULL, ...)", res, BREADBOARD_ERR_INVALID_ARGUMENT);

    /* 14. Free up */
    breadboard_artifact_draft_free(draft);
    breadboard_module_free(module);

    printf("All skeleton tests passed.\n");
    return 0;
}
