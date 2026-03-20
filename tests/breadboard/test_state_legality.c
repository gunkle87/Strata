#include "breadboard_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static void expect_last_diagnostic(
    BreadboardModule* module,
    BreadboardDiagnosticCode expected_code,
    const char* required_fragment)
{
    BreadboardResult res;
    BreadboardDiagnostic diag;

    res = breadboard_module_get_last_diagnostic(module, &diag);
    print_result("get_last_diagnostic", res, BREADBOARD_OK);

    if (diag.code != expected_code)
    {
        printf("[FAIL] Expected diagnostic code %d, got %d\n", expected_code, diag.code);
        exit(1);
    }

    if (required_fragment &&
        (!diag.message || strstr(diag.message, required_fragment) == NULL))
    {
        printf("[FAIL] Expected diagnostic message to contain '%s', got '%s'\n",
               required_fragment,
               diag.message ? diag.message : "(null)");
        exit(1);
    }

    printf("[PASS] Diagnostic code/message matched expectation\n");
}

static BreadboardModule* build_strength_projection_module(void)
{
    BreadboardModule* module = NULL;
    BreadboardResult res;
    BreadboardDescriptorSpec input_spec = { 1u, "in", 1u };
    BreadboardDescriptorSpec output_spec = { 2u, "out", 1u };
    BreadboardComponentSpec buf_spec = { 3u, "BUF", false };
    BreadboardExecutableConnectionSpec in_to_buf = {
        { BREADBOARD_ENDPOINT_MODULE_INPUT_SOURCE, 1u, 0u, 0u },
        { BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK, 0u, 3u, 0u }
    };
    BreadboardExecutableConnectionSpec buf_to_out = {
        { BREADBOARD_ENDPOINT_COMPONENT_OUTPUT_SOURCE, 0u, 3u, 0u },
        { BREADBOARD_ENDPOINT_MODULE_OUTPUT_SINK, 2u, 0u, 0u }
    };
    BreadboardRequirementProfile req_profile;

    res = breadboard_module_create(&module);
    print_result("module_create(strength projection)", res, BREADBOARD_OK);
    res = breadboard_module_set_target(module, BREADBOARD_TARGET_FAST_4STATE);
    print_result("set_target(FAST_4STATE)", res, BREADBOARD_OK);
    res = breadboard_module_add_input_descriptor(module, &input_spec);
    print_result("add_input_descriptor", res, BREADBOARD_OK);
    res = breadboard_module_add_output_descriptor(module, &output_spec);
    print_result("add_output_descriptor", res, BREADBOARD_OK);
    res = breadboard_module_add_component_instance(module, &buf_spec);
    print_result("add_component_instance(BUF)", res, BREADBOARD_OK);
    res = breadboard_module_add_executable_connection(module, &in_to_buf);
    print_result("add_executable_connection(in_to_buf)", res, BREADBOARD_OK);
    res = breadboard_module_add_executable_connection(module, &buf_to_out);
    print_result("add_executable_connection(buf_to_out)", res, BREADBOARD_OK);

    req_profile.extension_flags = 0u;
    req_profile.requires_advanced_controls = false;
    req_profile.requires_native_state_read = false;
    req_profile.requires_native_inputs = false;
    req_profile.required_projection_families_mask =
        (1u << STRATA_PROJECTION_FAMILY_STRENGTH_DISTINCTION);

    res = breadboard_module_set_requirement_profile(module, &req_profile);
    print_result("set_requirement_profile(strength)", res, BREADBOARD_OK);
    return module;
}

static BreadboardModule* build_plain_fast_module(void)
{
    BreadboardModule* module = NULL;
    BreadboardResult res;
    BreadboardDescriptorSpec input_spec = { 11u, "plain_in", 1u };
    BreadboardDescriptorSpec output_spec = { 12u, "plain_out", 1u };
    BreadboardComponentSpec buf_spec = { 13u, "BUF", false };
    BreadboardExecutableConnectionSpec in_to_buf = {
        { BREADBOARD_ENDPOINT_MODULE_INPUT_SOURCE, 11u, 0u, 0u },
        { BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK, 0u, 13u, 0u }
    };
    BreadboardExecutableConnectionSpec buf_to_out = {
        { BREADBOARD_ENDPOINT_COMPONENT_OUTPUT_SOURCE, 0u, 13u, 0u },
        { BREADBOARD_ENDPOINT_MODULE_OUTPUT_SINK, 12u, 0u, 0u }
    };

    res = breadboard_module_create(&module);
    print_result("module_create(plain fast)", res, BREADBOARD_OK);
    res = breadboard_module_set_target(module, BREADBOARD_TARGET_FAST_4STATE);
    print_result("set_target(plain fast)", res, BREADBOARD_OK);
    res = breadboard_module_add_input_descriptor(module, &input_spec);
    print_result("add_input_descriptor(plain fast)", res, BREADBOARD_OK);
    res = breadboard_module_add_output_descriptor(module, &output_spec);
    print_result("add_output_descriptor(plain fast)", res, BREADBOARD_OK);
    res = breadboard_module_add_component_instance(module, &buf_spec);
    print_result("add_component_instance(BUF plain fast)", res, BREADBOARD_OK);
    res = breadboard_module_add_executable_connection(module, &in_to_buf);
    print_result("add_executable_connection(in_to_buf plain fast)", res, BREADBOARD_OK);
    res = breadboard_module_add_executable_connection(module, &buf_to_out);
    print_result("add_executable_connection(buf_to_out plain fast)", res, BREADBOARD_OK);

    return module;
}

int main(void)
{
    printf("Starting State Legality Checks Tests...\n");

    {
        BreadboardModule* module = build_strength_projection_module();
        BreadboardResult res;
        BreadboardArtifactDraft* draft = NULL;
        BreadboardDraftAdmissionInfo admission_info;
        BreadboardProjectionPolicy policy;
        BreadboardCompileOptions options;

        policy.allowed_families_mask =
            (1u << STRATA_PROJECTION_FAMILY_STRENGTH_DISTINCTION);
        policy.deny_approximation = false;
        policy.strict_projection = false;
        policy.generate_report = false;
        memset(policy.reserved, 0, sizeof(policy.reserved));
        res = breadboard_module_set_projection_policy(module, &policy);
        print_result("set_projection_policy(allow strength)", res, BREADBOARD_OK);

        options.allow_placeholders = false;
        options.deny_approximation = false;
        options.strict_projection = false;
        options.require_real_executable = true;
        options.allowed_projection_families_mask = 0u;
        options.generate_projection_report = false;

        res = breadboard_module_compile(module, &options, &draft);
        print_result("compile succeeds with strength projection allowed", res, BREADBOARD_OK);
        res = breadboard_draft_query_admission_info(draft, &admission_info);
        print_result("draft_query_admission_info(real executable)", res, BREADBOARD_OK);
        if (admission_info.is_placeholder)
        {
            printf("[FAIL] Expected real executable draft after strength projection lowering.\n");
            return 1;
        }

        breadboard_artifact_draft_free(draft);
        breadboard_module_free(module);
    }

    {
        BreadboardModule* module = build_strength_projection_module();
        BreadboardResult res;
        BreadboardArtifactDraft* draft = NULL;
        BreadboardProjectionPolicy policy;
        BreadboardCompileOptions options;

        policy.allowed_families_mask = 0u;
        policy.deny_approximation = false;
        policy.strict_projection = false;
        policy.generate_report = false;
        memset(policy.reserved, 0, sizeof(policy.reserved));
        res = breadboard_module_set_projection_policy(module, &policy);
        print_result("set_projection_policy(deny strength)", res, BREADBOARD_OK);

        options.allow_placeholders = false;
        options.deny_approximation = false;
        options.strict_projection = false;
        options.require_real_executable = true;
        options.allowed_projection_families_mask = 0u;
        options.generate_projection_report = false;

        res = breadboard_module_compile(module, &options, &draft);
        print_result("compile fails when strength projection denied", res, BREADBOARD_ERR_COMPILE_FAILED);
        expect_last_diagnostic(
            module,
            BREADBOARD_DIAG_CODE_STATE_DISTINCTION_UNSUPPORTED,
            "projection policy denies");

        breadboard_artifact_draft_free(draft);
        breadboard_module_free(module);
    }

    {
        BreadboardModule* module = build_strength_projection_module();
        BreadboardResult res;
        BreadboardArtifactDraft* draft = NULL;
        BreadboardProjectionPolicy policy;
        BreadboardCompileOptions options;

        policy.allowed_families_mask =
            (1u << STRATA_PROJECTION_FAMILY_STRENGTH_DISTINCTION);
        policy.deny_approximation = false;
        policy.strict_projection = false;
        policy.generate_report = false;
        memset(policy.reserved, 0, sizeof(policy.reserved));
        res = breadboard_module_set_projection_policy(module, &policy);
        print_result("set_projection_policy(allow strength for deny approximation)", res, BREADBOARD_OK);

        options.allow_placeholders = false;
        options.deny_approximation = true;
        options.strict_projection = false;
        options.require_real_executable = true;
        options.allowed_projection_families_mask = 0u;
        options.generate_projection_report = false;

        res = breadboard_module_compile(module, &options, &draft);
        print_result("compile fails due to deny approximation on projected strength", res, BREADBOARD_ERR_COMPILE_FAILED);
        expect_last_diagnostic(
            module,
            BREADBOARD_DIAG_CODE_STATE_DISTINCTION_UNSUPPORTED,
            "approximation is denied");

        breadboard_artifact_draft_free(draft);
        breadboard_module_free(module);
    }

    {
        BreadboardModule* module = NULL;
        BreadboardResult res;
        BreadboardArtifactDraft* draft = NULL;
        BreadboardRequirementProfile req_profile;
        BreadboardCompileOptions options;

        res = breadboard_module_create(&module);
        print_result("module_create(backend-specific)", res, BREADBOARD_OK);
        res = breadboard_module_set_target(module, BREADBOARD_TARGET_FAST_4STATE);
        print_result("set_target(backend-specific)", res, BREADBOARD_OK);

        req_profile.extension_flags = 0u;
        req_profile.requires_advanced_controls = false;
        req_profile.requires_native_state_read = false;
        req_profile.requires_native_inputs = false;
        req_profile.required_projection_families_mask =
            (1u << STRATA_PROJECTION_FAMILY_BACKEND_SPECIFIC);
        res = breadboard_module_set_requirement_profile(module, &req_profile);
        print_result("set_requirement_profile(backend-specific)", res, BREADBOARD_OK);

        options.allow_placeholders = true;
        options.deny_approximation = false;
        options.strict_projection = false;
        options.require_real_executable = false;
        options.allowed_projection_families_mask =
            (1u << STRATA_PROJECTION_FAMILY_INITIALIZATION) |
            (1u << STRATA_PROJECTION_FAMILY_STRENGTH_DISTINCTION) |
            (1u << STRATA_PROJECTION_FAMILY_BACKEND_SPECIFIC);
        options.generate_projection_report = false;

        res = breadboard_module_compile(module, &options, &draft);
        print_result("compile still fails for backend-specific family on FAST_4STATE", res, BREADBOARD_ERR_COMPILE_FAILED);
        expect_last_diagnostic(
            module,
            BREADBOARD_DIAG_CODE_STATE_DISTINCTION_UNSUPPORTED,
            "cannot lower");

        breadboard_artifact_draft_free(draft);
        breadboard_module_free(module);
    }

    {
        BreadboardModule* module = build_plain_fast_module();
        BreadboardResult res;
        BreadboardArtifactDraft* draft = NULL;
        BreadboardCompileOptions options;
        BreadboardDraftAdmissionInfo admission_info;

        options.allow_placeholders = false;
        options.deny_approximation = true;
        options.strict_projection = true;
        options.require_real_executable = true;
        options.allowed_projection_families_mask = 0u;
        options.generate_projection_report = false;

        res = breadboard_module_compile(module, &options, &draft);
        print_result("compile succeeds with strict/deny flags when no projection is needed", res, BREADBOARD_OK);

        res = breadboard_draft_query_admission_info(draft, &admission_info);
        print_result("draft_query_admission_info(no projection needed)", res, BREADBOARD_OK);
        if (admission_info.is_placeholder)
        {
            printf("[FAIL] Expected real executable draft when no projection is needed.\n");
            return 1;
        }

        breadboard_artifact_draft_free(draft);
        breadboard_module_free(module);
    }

    printf("All State Legality Checks Tests passed.\n");
    return 0;
}
