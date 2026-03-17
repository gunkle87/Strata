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

    /* Restore to temporal target just to be sure */
    breadboard_module_set_target(module, BREADBOARD_TARGET_TEMPORAL);

    /* 4. Test compilation refusal (no placeholder flag) */
    BreadboardArtifactDraft* draft = NULL;
    res = breadboard_module_compile(module, NULL, &draft);
    print_result("module_compile(no_options)", res, BREADBOARD_ERR_UNSUPPORTED);

    /* 5. Test compilation with explicit placeholders allowed */
    BreadboardCompileOptions opts = { .allow_placeholders = true };
    res = breadboard_module_compile(module, &opts, &draft);
    print_result("module_compile(allow_placeholders)", res, BREADBOARD_OK);

    if (!draft)
    {
        printf("[FAIL] draft pointer is NULL\n");
        exit(1);
    }

    /* 6. Artifact metadata check */
    BreadboardTarget queried_target = BREADBOARD_TARGET_UNSPECIFIED;
    res = breadboard_artifact_draft_query_metadata(draft, &queried_target);
    print_result("draft_query_metadata", res, BREADBOARD_OK);

    if (queried_target != BREADBOARD_TARGET_TEMPORAL)
    {
        printf("[FAIL] draft target mismatch (Expected %d, got %d)\n",
               BREADBOARD_TARGET_TEMPORAL, queried_target);
        exit(1);
    }

    /* 7. Test diagnostics */
    size_t diag_count = 99;
    res = breadboard_module_get_diagnostic_count(module, &diag_count);
    print_result("module_get_diagnostic_count", res, BREADBOARD_OK);
    if (diag_count != 0)
    {
        printf("[FAIL] expected 0 diagnostics, got %zu\n", diag_count);
        exit(1);
    }

    BreadboardDiagnostic diag;
    res = breadboard_module_get_diagnostic(module, 0, &diag);
    print_result("module_get_diagnostic(0)", res, BREADBOARD_ERR_UNSUPPORTED);

    /* 8. Free up */
    breadboard_artifact_draft_free(draft);
    breadboard_module_free(module);

    printf("All skeleton tests passed.\n");
    return 0;
}
