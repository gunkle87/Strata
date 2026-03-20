#include "breadboard_api.h"
#include <stdio.h>
#include <stdlib.h>

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
    printf("Starting State Legality Checks Tests...\n");

    BreadboardModule* module = NULL;
    BreadboardResult res;
    BreadboardArtifactDraft* draft = NULL;

    res = breadboard_module_create(&module);
    if (res != BREADBOARD_OK || !module)
    {
        printf("Failed to create module.\n");
        return 1;
    }

    res = breadboard_module_set_target(module, BREADBOARD_TARGET_FAST_4STATE);
    print_result("set_target", res, BREADBOARD_OK);

    BreadboardRequirementProfile req_profile;
    req_profile.extension_flags = 0;
    req_profile.requires_advanced_controls = false;
    req_profile.requires_native_state_read = false;
    req_profile.requires_native_inputs = false;
    req_profile.required_projection_families_mask = (1u << STRATA_PROJECTION_FAMILY_BACKEND_SPECIFIC);

    res = breadboard_module_set_requirement_profile(module, &req_profile);
    print_result("set_requirement_profile", res, BREADBOARD_OK);

    BreadboardCompileOptions options;
    options.allow_placeholders = true;
    options.deny_approximation = false;
    options.strict_projection = false;
    options.require_real_executable = false;
    options.allowed_projection_families_mask = 0;
    options.generate_projection_report = false;

    res = breadboard_module_compile(module, &options, &draft);
    print_result("compile fails due to unsupported state distinction", res, BREADBOARD_ERR_COMPILE_FAILED);

    BreadboardDiagnostic diag;
    res = breadboard_module_get_last_diagnostic(module, &diag);
    print_result("get_last_diagnostic", res, BREADBOARD_OK);

    if (diag.code != BREADBOARD_DIAG_CODE_STATE_DISTINCTION_UNSUPPORTED)
    {
        printf("[FAIL] Expected diagnostic code %d, got %d\n", BREADBOARD_DIAG_CODE_STATE_DISTINCTION_UNSUPPORTED, diag.code);
        return 1;
    }
    printf("[PASS] Diagnostic code is STATE_DISTINCTION_UNSUPPORTED\n");

    /* Test allowing the projection policy explicitly */
    options.allowed_projection_families_mask = (1u << STRATA_PROJECTION_FAMILY_BACKEND_SPECIFIC) | (1u << STRATA_PROJECTION_FAMILY_INITIALIZATION) | (1u << STRATA_PROJECTION_FAMILY_STRENGTH_DISTINCTION);
    res = breadboard_module_compile(module, &options, &draft);
    print_result("compile succeeds with allowed projection families", res, BREADBOARD_OK);
    if (draft)
    {
        breadboard_artifact_draft_free(draft);
        draft = NULL;
    }

    /* Test strict projection blocking lossy backend-specific projection */
    options.strict_projection = true;
    res = breadboard_module_compile(module, &options, &draft);
    print_result("compile fails due to strict projection on backend specific", res, BREADBOARD_ERR_COMPILE_FAILED);
    
    res = breadboard_module_get_last_diagnostic(module, &diag);
    if (diag.code != BREADBOARD_DIAG_CODE_STATE_DISTINCTION_UNSUPPORTED)
    {
        printf("[FAIL] Expected diagnostic code %d, got %d\n", BREADBOARD_DIAG_CODE_STATE_DISTINCTION_UNSUPPORTED, diag.code);
        return 1;
    }
    printf("[PASS] Diagnostic code is STATE_DISTINCTION_UNSUPPORTED for strict projection\n");

    /* Test deny_approximation */
    options.strict_projection = false;
    options.deny_approximation = true;
    res = breadboard_module_compile(module, &options, &draft);
    print_result("compile fails due to deny approximation", res, BREADBOARD_ERR_COMPILE_FAILED);

    res = breadboard_module_get_last_diagnostic(module, &diag);
    if (diag.code != BREADBOARD_DIAG_CODE_STATE_DISTINCTION_UNSUPPORTED)
    {
        printf("[FAIL] Expected diagnostic code %d, got %d\n", BREADBOARD_DIAG_CODE_STATE_DISTINCTION_UNSUPPORTED, diag.code);
        return 1;
    }
    printf("[PASS] Diagnostic code is STATE_DISTINCTION_UNSUPPORTED for deny approximation\n");

    breadboard_module_free(module);
    printf("All State Legality Checks Tests passed.\n");
    return 0;
}