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
    printf("Starting UNINIT Handling Tests...\n");

    BreadboardModule* module = NULL;
    BreadboardResult res;
    BreadboardArtifactDraft* draft = NULL;
    BreadboardDraftAdmissionInfo admission_info;

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
    req_profile.required_projection_families_mask = (1u << STRATA_PROJECTION_FAMILY_INITIALIZATION);

    res = breadboard_module_set_requirement_profile(module, &req_profile);
    print_result("set_requirement_profile", res, BREADBOARD_OK);

    BreadboardComponentSpec uninit_spec;
    uninit_spec.id = 100u;
    uninit_spec.kind_name = "UNINIT";
    uninit_spec.is_stateful = false;

    res = breadboard_module_add_component_instance(module, &uninit_spec);
    print_result("add_UNINIT_component", res, BREADBOARD_OK);

    BreadboardCompileOptions options;
    options.allow_placeholders = false;
    options.deny_approximation = false;
    options.strict_projection = false;
    options.require_real_executable = true;
    options.allowed_projection_families_mask = (1u << STRATA_PROJECTION_FAMILY_INITIALIZATION);
    options.generate_projection_report = false;

    res = breadboard_module_compile(module, &options, &draft);
    print_result("compile fails for steady-state fast execution", res, BREADBOARD_ERR_COMPILE_FAILED);

    BreadboardDiagnostic diag;
    res = breadboard_module_get_last_diagnostic(module, &diag);
    print_result("get_last_diagnostic", res, BREADBOARD_OK);

    if (diag.code != BREADBOARD_DIAG_CODE_STATE_DISTINCTION_UNSUPPORTED)
    {
        printf("[FAIL] Expected diagnostic code %d, got %d\n", BREADBOARD_DIAG_CODE_STATE_DISTINCTION_UNSUPPORTED, diag.code);
        return 1;
    }
    printf("[PASS] Diagnostic code is STATE_DISTINCTION_UNSUPPORTED\n");

    /* Also verify that when allowed, UNINIT is explicitly routed to native-only placeholder fallback. */
    options.require_real_executable = false;
    options.allow_placeholders = true;
    res = breadboard_module_compile(module, &options, &draft);
    print_result("compile succeeds with placeholder fallback", res, BREADBOARD_OK);

    res = breadboard_draft_query_admission_info(draft, &admission_info);
    print_result("draft_query_admission_info", res, BREADBOARD_OK);

    if (!admission_info.is_placeholder)
    {
        printf("[FAIL] Expected placeholder draft for UNINIT fallback.\n");
        return 1;
    }

    if (!admission_info.requires_native_state_read)
    {
        printf("[FAIL] Expected UNINIT fallback to require native state handling.\n");
        return 1;
    }

    if (!admission_info.native_only_behavior)
    {
        printf("[FAIL] Expected UNINIT fallback to be marked native-only.\n");
        return 1;
    }

    printf("[PASS] UNINIT placeholder fallback is marked native-only\n");

    if (draft)
    {
        breadboard_artifact_draft_free(draft);
        draft = NULL;
    }

    breadboard_module_free(module);
    printf("All UNINIT Handling Tests passed.\n");
    return 0;
}
