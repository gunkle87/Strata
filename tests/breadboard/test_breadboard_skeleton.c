#include "breadboard_api.h"
#include "../../include/strata_placeholder_artifact.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    BreadboardModuleIdentity module_identity = { 0x1234u, "temporal_demo" };
    BreadboardStructureSummary module_summary = { 7u, 11u, 2u };

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

    {
        BreadboardExecutableSubsetInfo subset_info;
        BreadboardPrimitiveSignature primitive_signature;

        res = breadboard_query_executable_subset_info(
            BREADBOARD_TARGET_FAST_4STATE,
            &subset_info);
        print_result("query_executable_subset_info(FAST_4STATE)", res, BREADBOARD_OK);
        if (subset_info.subset != BREADBOARD_EXECUTABLE_SUBSET_FAST_COMBINATIONAL_V1 ||
            !subset_info.has_real_executable_subset ||
            !subset_info.flat_only ||
            !subset_info.single_bit_only ||
            !subset_info.combinational_only ||
            subset_info.allows_stateful_components ||
            subset_info.allows_cycles ||
            !subset_info.real_path_hard_rejects_out_of_subset ||
            !subset_info.placeholder_fallback_requires_explicit_allowance ||
            subset_info.admitted_primitive_mask !=
                BREADBOARD_PRIMITIVE_MASK_ALL_FIRST_EXECUTABLE ||
            subset_info.module_input_endpoint_class !=
                BREADBOARD_ENDPOINT_MODULE_INPUT_SOURCE ||
            subset_info.component_output_endpoint_class !=
                BREADBOARD_ENDPOINT_COMPONENT_OUTPUT_SOURCE ||
            subset_info.component_input_endpoint_class !=
                BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK ||
            subset_info.module_output_endpoint_class !=
                BREADBOARD_ENDPOINT_MODULE_OUTPUT_SINK)
        {
            printf("[FAIL] FAST_4STATE executable subset info mismatch\n");
            exit(1);
        }

        res = breadboard_query_executable_subset_info(
            BREADBOARD_TARGET_TEMPORAL,
            &subset_info);
        print_result("query_executable_subset_info(TEMPORAL)", res, BREADBOARD_OK);
        if (subset_info.subset != BREADBOARD_EXECUTABLE_SUBSET_NONE ||
            subset_info.has_real_executable_subset ||
            subset_info.target != BREADBOARD_TARGET_TEMPORAL)
        {
            printf("[FAIL] TEMPORAL executable subset info mismatch\n");
            exit(1);
        }

        res = breadboard_query_executable_subset_info(
            (BreadboardTarget)999,
            &subset_info);
        print_result("query_executable_subset_info(INVALID)", res, BREADBOARD_ERR_INVALID_TARGET);

        res = breadboard_query_executable_subset_info(
            BREADBOARD_TARGET_FAST_4STATE,
            NULL);
        print_result("query_executable_subset_info(..., NULL)", res, BREADBOARD_ERR_INVALID_ARGUMENT);

        res = breadboard_query_primitive_signature(
            BREADBOARD_TARGET_FAST_4STATE,
            BREADBOARD_PRIMITIVE_BUF,
            &primitive_signature);
        print_result("query_primitive_signature(BUF)", res, BREADBOARD_OK);
        if (primitive_signature.primitive_kind != BREADBOARD_PRIMITIVE_BUF ||
            primitive_signature.target != BREADBOARD_TARGET_FAST_4STATE ||
            primitive_signature.input_count != 1u ||
            primitive_signature.output_count != 1u ||
            strcmp(primitive_signature.input_name_0, "in") != 0 ||
            primitive_signature.input_name_1 != NULL ||
            strcmp(primitive_signature.output_name_0, "out") != 0 ||
            primitive_signature.is_stateful ||
            !primitive_signature.single_bit_only)
        {
            printf("[FAIL] BUF primitive signature mismatch\n");
            exit(1);
        }

        res = breadboard_query_primitive_signature(
            BREADBOARD_TARGET_FAST_4STATE,
            BREADBOARD_PRIMITIVE_AND,
            &primitive_signature);
        print_result("query_primitive_signature(AND)", res, BREADBOARD_OK);
        if (primitive_signature.primitive_kind != BREADBOARD_PRIMITIVE_AND ||
            primitive_signature.input_count != 2u ||
            strcmp(primitive_signature.input_name_0, "a") != 0 ||
            strcmp(primitive_signature.input_name_1, "b") != 0 ||
            strcmp(primitive_signature.output_name_0, "out") != 0)
        {
            printf("[FAIL] AND primitive signature mismatch\n");
            exit(1);
        }

        res = breadboard_query_primitive_signature(
            BREADBOARD_TARGET_FAST_4STATE,
            BREADBOARD_PRIMITIVE_NOT,
            &primitive_signature);
        print_result("query_primitive_signature(NOT)", res, BREADBOARD_OK);
        if (primitive_signature.primitive_kind != BREADBOARD_PRIMITIVE_NOT ||
            primitive_signature.input_count != 1u ||
            strcmp(primitive_signature.input_name_0, "in") != 0 ||
            primitive_signature.input_name_1 != NULL ||
            strcmp(primitive_signature.output_name_0, "out") != 0 ||
            primitive_signature.is_stateful ||
            !primitive_signature.single_bit_only)
        {
            printf("[FAIL] NOT primitive signature mismatch\n");
            exit(1);
        }

        res = breadboard_query_primitive_signature(
            BREADBOARD_TARGET_FAST_4STATE,
            BREADBOARD_PRIMITIVE_OR,
            &primitive_signature);
        print_result("query_primitive_signature(OR)", res, BREADBOARD_OK);
        if (primitive_signature.primitive_kind != BREADBOARD_PRIMITIVE_OR ||
            primitive_signature.input_count != 2u ||
            strcmp(primitive_signature.input_name_0, "a") != 0 ||
            strcmp(primitive_signature.input_name_1, "b") != 0 ||
            strcmp(primitive_signature.output_name_0, "out") != 0 ||
            primitive_signature.is_stateful ||
            !primitive_signature.single_bit_only)
        {
            printf("[FAIL] OR primitive signature mismatch\n");
            exit(1);
        }

        res = breadboard_query_primitive_signature(
            BREADBOARD_TARGET_FAST_4STATE,
            BREADBOARD_PRIMITIVE_XOR,
            &primitive_signature);
        print_result("query_primitive_signature(XOR)", res, BREADBOARD_OK);
        if (primitive_signature.primitive_kind != BREADBOARD_PRIMITIVE_XOR ||
            primitive_signature.input_count != 2u ||
            strcmp(primitive_signature.input_name_0, "a") != 0 ||
            strcmp(primitive_signature.input_name_1, "b") != 0 ||
            strcmp(primitive_signature.output_name_0, "out") != 0 ||
            primitive_signature.is_stateful ||
            !primitive_signature.single_bit_only)
        {
            printf("[FAIL] XOR primitive signature mismatch\n");
            exit(1);
        }

        res = breadboard_query_primitive_signature(
            BREADBOARD_TARGET_TEMPORAL,
            BREADBOARD_PRIMITIVE_BUF,
            &primitive_signature);
        print_result("query_primitive_signature(TEMPORAL, BUF)", res, BREADBOARD_ERR_UNSUPPORTED);

        res = breadboard_query_primitive_signature(
            BREADBOARD_TARGET_FAST_4STATE,
            BREADBOARD_PRIMITIVE_INVALID,
            &primitive_signature);
        print_result("query_primitive_signature(INVALID primitive)", res, BREADBOARD_ERR_INVALID_ARGUMENT);

        res = breadboard_query_primitive_signature(
            (BreadboardTarget)999,
            BREADBOARD_PRIMITIVE_BUF,
            &primitive_signature);
        print_result("query_primitive_signature(INVALID target)", res, BREADBOARD_ERR_INVALID_TARGET);

        res = breadboard_query_primitive_signature(
            BREADBOARD_TARGET_FAST_4STATE,
            BREADBOARD_PRIMITIVE_BUF,
            NULL);
        print_result("query_primitive_signature(..., NULL)", res, BREADBOARD_ERR_INVALID_ARGUMENT);
    }

    {
        BreadboardModule* executable_module = NULL;
        BreadboardArtifactDraft* executable_draft = NULL;
        BreadboardExecutableAssessment assessment;
        BreadboardDraftInfo draft_info;
        BreadboardDraftAdmissionInfo admission_info;
        BreadboardDiagnostic diagnostic;
        BreadboardComponent component;
        BreadboardDescriptor descriptor;
        size_t diagnostic_count = 0u;
        size_t component_count = 0u;
        size_t connection_count = 0u;
        size_t export_size = 0u;
        size_t input_descriptor_count = 0u;
        size_t output_descriptor_count = 0u;
        size_t probe_descriptor_count = 0u;
        BreadboardDescriptorSpec input_a = { 401u, "a", 1u };
        BreadboardDescriptorSpec input_b = { 402u, "b", 1u };
        BreadboardDescriptorSpec output_y = { 403u, "y", 1u };
        BreadboardComponentSpec and_gate = { 501u, "AND", false };
        BreadboardComponentSpec not_gate = { 502u, "NOT", false };
        BreadboardExecutableConnectionSpec exec_ab = {
            { BREADBOARD_ENDPOINT_MODULE_INPUT_SOURCE, 401u, 0u, 0u },
            { BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK, 0u, 501u, 0u }
        };
        BreadboardExecutableConnectionSpec exec_bb = {
            { BREADBOARD_ENDPOINT_MODULE_INPUT_SOURCE, 402u, 0u, 0u },
            { BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK, 0u, 501u, 1u }
        };
        BreadboardExecutableConnectionSpec exec_chain = {
            { BREADBOARD_ENDPOINT_COMPONENT_OUTPUT_SOURCE, 0u, 501u, 0u },
            { BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK, 0u, 502u, 0u }
        };
        BreadboardExecutableConnectionSpec exec_out = {
            { BREADBOARD_ENDPOINT_COMPONENT_OUTPUT_SOURCE, 0u, 502u, 0u },
            { BREADBOARD_ENDPOINT_MODULE_OUTPUT_SINK, 403u, 0u, 0u }
        };
        BreadboardCompileOptions opts_real = {
            .allow_placeholders = false,
            .deny_approximation = false,
            .strict_projection = false,
            .require_real_executable = true
        };
        BreadboardCompileOptions opts_conflict = {
            .allow_placeholders = true,
            .deny_approximation = false,
            .strict_projection = false,
            .require_real_executable = true
        };

        res = breadboard_module_create(&executable_module);
        print_result("exec module_create", res, BREADBOARD_OK);
        res = breadboard_module_set_target(executable_module, BREADBOARD_TARGET_FAST_4STATE);
        print_result("exec module_set_target", res, BREADBOARD_OK);
        res = breadboard_module_add_input_descriptor(executable_module, &input_a);
        print_result("exec add_input_a", res, BREADBOARD_OK);
        res = breadboard_module_add_input_descriptor(executable_module, &input_b);
        print_result("exec add_input_b", res, BREADBOARD_OK);
        res = breadboard_module_add_output_descriptor(executable_module, &output_y);
        print_result("exec add_output_y", res, BREADBOARD_OK);
        res = breadboard_module_add_component_instance(executable_module, &and_gate);
        print_result("exec add_component_and", res, BREADBOARD_OK);
        res = breadboard_module_add_component_instance(executable_module, &not_gate);
        print_result("exec add_component_not", res, BREADBOARD_OK);
        res = breadboard_module_add_executable_connection(executable_module, &exec_ab);
        print_result("exec add_connection_ab", res, BREADBOARD_OK);
        res = breadboard_module_add_executable_connection(executable_module, &exec_bb);
        print_result("exec add_connection_bb", res, BREADBOARD_OK);
        res = breadboard_module_add_executable_connection(executable_module, &exec_chain);
        print_result("exec add_connection_chain", res, BREADBOARD_OK);
        res = breadboard_module_add_executable_connection(executable_module, &exec_out);
        print_result("exec add_connection_out", res, BREADBOARD_OK);

        res = breadboard_module_assess_executable_subset(executable_module, &assessment);
        print_result("exec assess_subset", res, BREADBOARD_OK);
        if (assessment.subset != BREADBOARD_EXECUTABLE_SUBSET_FAST_COMBINATIONAL_V1 ||
            assessment.status != BREADBOARD_EXECUTABLE_ASSESSMENT_EXECUTABLE ||
            assessment.reason != BREADBOARD_EXEC_REASON_NONE)
        {
            printf("[FAIL] executable assessment mismatch\n");
            exit(1);
        }

        res = breadboard_module_compile(executable_module, &opts_conflict, &executable_draft);
        print_result("exec compile(conflicting options)", res, BREADBOARD_ERR_INVALID_ARGUMENT);

        res = breadboard_module_compile(executable_module, &opts_real, &executable_draft);
        if (res == BREADBOARD_OK)
        {
            printf("[PASS] exec compile(require_real_executable)\n");
        }
        else
        {
            printf("[FAIL] exec compile(require_real_executable) (Expected 0, got %d)\n", res);
            res = breadboard_module_get_last_diagnostic(executable_module, &diagnostic);
            print_result("exec last_diagnostic(after real compile)", res, BREADBOARD_OK);
            if (res == BREADBOARD_OK)
            {
                printf("[FAIL] real compile diagnostic code=%d message=%s\n",
                    diagnostic.code,
                    diagnostic.message ? diagnostic.message : "(null)");
            }
        }

        res = breadboard_module_get_diagnostic_count(executable_module, &diagnostic_count);
        print_result("exec diagnostic_count", res, BREADBOARD_OK);
        if (diagnostic_count != 0u)
        {
            printf("[FAIL] executable lowering should not emit diagnostics on success\n");
            exit(1);
        }

        res = breadboard_artifact_draft_query_info(executable_draft, &draft_info);
        print_result("exec draft_query_info", res, BREADBOARD_OK);
        if (draft_info.target != BREADBOARD_TARGET_FAST_4STATE ||
            draft_info.has_placeholders ||
            draft_info.approximate_size_bytes !=
                sizeof(StrataPlaceholderArtifactHeader) +
                strata_placeholder_section_table_bytes(5u) +
                sizeof(StrataPlaceholderAdmissionInfo) +
                sizeof(StrataPlaceholderDraftSummary) +
                strata_placeholder_structure_bytes_for_counts(2u, 1u) +
                strata_placeholder_descriptor_bytes_for_counts(2u, 1u, 0u) +
                strata_placeholder_fast_payload_bytes_for_counts(2u, 4u, 2u, 1u) ||
            draft_info.source_module_id != 0u ||
            strcmp(draft_info.source_module_name, "") != 0 ||
            draft_info.declared_component_count != 2u ||
            draft_info.declared_connection_count != 1u ||
            draft_info.declared_stateful_node_count != 0u)
        {
            printf("[FAIL] executable draft info mismatch\n");
            exit(1);
        }

        res = breadboard_draft_input_descriptor_count(executable_draft, &input_descriptor_count);
        print_result("exec draft_input_descriptor_count", res, BREADBOARD_OK);
        res = breadboard_draft_output_descriptor_count(executable_draft, &output_descriptor_count);
        print_result("exec draft_output_descriptor_count", res, BREADBOARD_OK);
        res = breadboard_draft_probe_descriptor_count(executable_draft, &probe_descriptor_count);
        print_result("exec draft_probe_descriptor_count", res, BREADBOARD_OK);
        if (input_descriptor_count != 2u ||
            output_descriptor_count != 1u ||
            probe_descriptor_count != 0u)
        {
            printf("[FAIL] executable draft descriptor count mismatch\n");
            exit(1);
        }

        res = breadboard_draft_input_descriptor_by_id(executable_draft, 401u, &descriptor);
        print_result("exec draft_input_descriptor_by_id(401)", res, BREADBOARD_OK);
        if (descriptor.id != 401u || strcmp(descriptor.name, "a") != 0 || descriptor.width != 1u)
        {
            printf("[FAIL] executable draft input descriptor mismatch\n");
            exit(1);
        }

        res = breadboard_draft_output_descriptor_by_name(executable_draft, "y", &descriptor);
        print_result("exec draft_output_descriptor_by_name(y)", res, BREADBOARD_OK);
        if (descriptor.id != 403u || strcmp(descriptor.name, "y") != 0 || descriptor.width != 1u)
        {
            printf("[FAIL] executable draft output descriptor mismatch\n");
            exit(1);
        }

        res = breadboard_draft_query_admission_info(executable_draft, &admission_info);
        print_result("exec draft_query_admission_info", res, BREADBOARD_OK);
        if (admission_info.target != BREADBOARD_TARGET_FAST_4STATE ||
            admission_info.is_placeholder ||
            admission_info.extension_flags != 0u ||
            admission_info.requires_advanced_controls ||
            admission_info.requires_native_state_read ||
            admission_info.requires_native_inputs ||
            admission_info.native_only_behavior)
        {
            printf("[FAIL] executable admission info mismatch\n");
            exit(1);
        }

        res = breadboard_draft_component_count(executable_draft, &component_count);
        print_result("exec draft_component_count", res, BREADBOARD_OK);
        res = breadboard_draft_connection_count(executable_draft, &connection_count);
        print_result("exec draft_connection_count", res, BREADBOARD_OK);
        if (component_count != 2u || connection_count != 1u)
        {
            printf("[FAIL] executable draft structural count mismatch\n");
            exit(1);
        }

        res = breadboard_draft_component_at(executable_draft, 0u, &component);
        print_result("exec draft_component_at(0)", res, BREADBOARD_OK);
        if (component.id != 501u || strcmp(component.kind_name, "AND") != 0 || component.is_stateful)
        {
            printf("[FAIL] executable draft component 0 mismatch\n");
            exit(1);
        }

        res = breadboard_draft_component_at(executable_draft, 1u, &component);
        print_result("exec draft_component_at(1)", res, BREADBOARD_OK);
        if (component.id != 502u || strcmp(component.kind_name, "NOT") != 0 || component.is_stateful)
        {
            printf("[FAIL] executable draft component 1 mismatch\n");
            exit(1);
        }

        res = breadboard_draft_component_by_id(executable_draft, 501u, &component);
        print_result("exec draft_component_by_id(501)", res, BREADBOARD_OK);
        if (component.id != 501u || strcmp(component.kind_name, "AND") != 0 || component.is_stateful)
        {
            printf("[FAIL] executable draft component-by-id mismatch\n");
            exit(1);
        }

        res = breadboard_artifact_draft_export_placeholder_size(executable_draft, &export_size);
        print_result("exec draft_export_placeholder_size", res, BREADBOARD_ERR_UNSUPPORTED);

        res = breadboard_artifact_draft_export_fast_size(executable_draft, &export_size);
        print_result("exec draft_export_fast_size", res, BREADBOARD_OK);
        if (export_size == 0u)
        {
            printf("[FAIL] real fast export size should be non-zero\n");
            exit(1);
        }

        {
            unsigned char* export_bytes = (unsigned char*)malloc(export_size);
            const StrataPlaceholderArtifactHeader* export_header;

            if (!export_bytes)
            {
                printf("[FAIL] could not allocate real fast export bytes\n");
                exit(1);
            }

            res = breadboard_artifact_draft_export_fast(
                executable_draft,
                export_bytes,
                export_size,
                &export_size);
            print_result("exec draft_export_fast", res, BREADBOARD_OK);

            export_header = (const StrataPlaceholderArtifactHeader*)export_bytes;
            if (export_header->payload_kind != STRATA_PLACEHOLDER_PAYLOAD_FAST_EXECUTABLE_V1)
            {
                printf("[FAIL] real fast export payload kind mismatch\n");
                free(export_bytes);
                exit(1);
            }

            free(export_bytes);
        }

        res = breadboard_module_add_executable_connection(NULL, &exec_ab);
        print_result("add_executable_connection(NULL, ...)", res, BREADBOARD_ERR_INVALID_HANDLE);
        res = breadboard_module_add_executable_connection(executable_module, NULL);
        print_result("add_executable_connection(..., NULL)", res, BREADBOARD_ERR_INVALID_ARGUMENT);
        res = breadboard_module_assess_executable_subset(NULL, &assessment);
        print_result("assess_executable_subset(NULL, ...)", res, BREADBOARD_ERR_INVALID_HANDLE);
        res = breadboard_module_assess_executable_subset(executable_module, NULL);
        print_result("assess_executable_subset(..., NULL)", res, BREADBOARD_ERR_INVALID_ARGUMENT);

        breadboard_artifact_draft_free(executable_draft);
        breadboard_module_free(executable_module);
    }

    {
        BreadboardModule* unsupported_module = NULL;
        BreadboardArtifactDraft* unsupported_draft = NULL;
        BreadboardExecutableAssessment assessment;
        BreadboardDiagnostic diagnostic;
        BreadboardDescriptorSpec input_desc = { 601u, "in", 1u };
        BreadboardDescriptorSpec output_desc = { 602u, "out", 1u };
        BreadboardComponentSpec unsupported_component = { 603u, "NAND", false };
        BreadboardCompileOptions opts_real = {
            .allow_placeholders = false,
            .deny_approximation = false,
            .strict_projection = false,
            .require_real_executable = true
        };

        res = breadboard_module_create(&unsupported_module);
        print_result("unsupported exec module_create", res, BREADBOARD_OK);
        res = breadboard_module_set_target(unsupported_module, BREADBOARD_TARGET_FAST_4STATE);
        print_result("unsupported exec module_set_target", res, BREADBOARD_OK);
        res = breadboard_module_add_input_descriptor(unsupported_module, &input_desc);
        print_result("unsupported exec add_input", res, BREADBOARD_OK);
        res = breadboard_module_add_output_descriptor(unsupported_module, &output_desc);
        print_result("unsupported exec add_output", res, BREADBOARD_OK);
        res = breadboard_module_add_component_instance(unsupported_module, &unsupported_component);
        print_result("unsupported exec add_component", res, BREADBOARD_OK);
        res = breadboard_module_assess_executable_subset(unsupported_module, &assessment);
        print_result("unsupported exec assess_subset", res, BREADBOARD_OK);
        if (assessment.status != BREADBOARD_EXECUTABLE_ASSESSMENT_PLACEHOLDER_ONLY ||
            assessment.reason != BREADBOARD_EXEC_REASON_PRIMITIVE_UNSUPPORTED ||
            assessment.failing_component_id != 603u)
        {
            printf("[FAIL] unsupported primitive assessment mismatch\n");
            exit(1);
        }

        res = breadboard_module_compile(unsupported_module, &opts_real, &unsupported_draft);
        print_result("unsupported exec compile(require_real_executable)", res, BREADBOARD_ERR_COMPILE_FAILED);
        res = breadboard_module_get_last_diagnostic(unsupported_module, &diagnostic);
        print_result("unsupported exec last_diagnostic", res, BREADBOARD_OK);
        if (diagnostic.code != BREADBOARD_DIAG_CODE_EXECUTABLE_SUBSET_REQUIRED)
        {
            printf("[FAIL] unsupported primitive diagnostic mismatch\n");
            exit(1);
        }

        breadboard_artifact_draft_free(unsupported_draft);
        breadboard_module_free(unsupported_module);
    }

    {
        BreadboardModule* invalid_exec_module = NULL;
        BreadboardArtifactDraft* invalid_exec_draft = NULL;
        BreadboardExecutableAssessment assessment;
        BreadboardDiagnostic diagnostic;
        BreadboardDescriptorSpec input_a = { 701u, "a", 1u };
        BreadboardDescriptorSpec input_b = { 702u, "b", 1u };
        BreadboardDescriptorSpec output_y = { 703u, "y", 1u };
        BreadboardComponentSpec and_gate = { 704u, "AND", false };
        BreadboardExecutableConnectionSpec first_driver = {
            { BREADBOARD_ENDPOINT_MODULE_INPUT_SOURCE, 701u, 0u, 0u },
            { BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK, 0u, 704u, 0u }
        };
        BreadboardExecutableConnectionSpec duplicate_driver = {
            { BREADBOARD_ENDPOINT_MODULE_INPUT_SOURCE, 702u, 0u, 0u },
            { BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK, 0u, 704u, 0u }
        };
        BreadboardExecutableConnectionSpec output_driver = {
            { BREADBOARD_ENDPOINT_COMPONENT_OUTPUT_SOURCE, 0u, 704u, 0u },
            { BREADBOARD_ENDPOINT_MODULE_OUTPUT_SINK, 703u, 0u, 0u }
        };
        BreadboardCompileOptions opts_real = {
            .allow_placeholders = false,
            .deny_approximation = false,
            .strict_projection = false,
            .require_real_executable = true
        };

        res = breadboard_module_create(&invalid_exec_module);
        print_result("invalid exec module_create", res, BREADBOARD_OK);
        res = breadboard_module_set_target(invalid_exec_module, BREADBOARD_TARGET_FAST_4STATE);
        print_result("invalid exec module_set_target", res, BREADBOARD_OK);
        res = breadboard_module_add_input_descriptor(invalid_exec_module, &input_a);
        print_result("invalid exec add_input_a", res, BREADBOARD_OK);
        res = breadboard_module_add_input_descriptor(invalid_exec_module, &input_b);
        print_result("invalid exec add_input_b", res, BREADBOARD_OK);
        res = breadboard_module_add_output_descriptor(invalid_exec_module, &output_y);
        print_result("invalid exec add_output", res, BREADBOARD_OK);
        res = breadboard_module_add_component_instance(invalid_exec_module, &and_gate);
        print_result("invalid exec add_component", res, BREADBOARD_OK);
        res = breadboard_module_add_executable_connection(invalid_exec_module, &first_driver);
        print_result("invalid exec add_connection_first", res, BREADBOARD_OK);
        res = breadboard_module_add_executable_connection(invalid_exec_module, &duplicate_driver);
        print_result("invalid exec add_connection_duplicate", res, BREADBOARD_OK);
        res = breadboard_module_add_executable_connection(invalid_exec_module, &output_driver);
        print_result("invalid exec add_connection_output", res, BREADBOARD_OK);

        res = breadboard_module_assess_executable_subset(invalid_exec_module, &assessment);
        print_result("invalid exec assess_subset", res, BREADBOARD_OK);
        if (assessment.status != BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID ||
            assessment.reason != BREADBOARD_EXEC_REASON_DUPLICATE_SINK_DRIVER ||
            assessment.failing_component_id != 704u)
        {
            printf("[FAIL] invalid executable assessment mismatch\n");
            exit(1);
        }

        res = breadboard_module_compile(invalid_exec_module, &opts_real, &invalid_exec_draft);
        print_result("invalid exec compile(require_real_executable)", res, BREADBOARD_ERR_COMPILE_FAILED);
        res = breadboard_module_get_last_diagnostic(invalid_exec_module, &diagnostic);
        print_result("invalid exec last_diagnostic", res, BREADBOARD_OK);
        if (diagnostic.code != BREADBOARD_DIAG_CODE_EXECUTABLE_SUBSET_INVALID)
        {
            printf("[FAIL] invalid executable diagnostic mismatch\n");
            exit(1);
        }

        breadboard_artifact_draft_free(invalid_exec_draft);
        breadboard_module_free(invalid_exec_module);
    }

    {
        BreadboardModule* temporal_exec_module = NULL;
        BreadboardExecutableAssessment assessment;

        res = breadboard_module_create(&temporal_exec_module);
        print_result("temporal exec module_create", res, BREADBOARD_OK);
        res = breadboard_module_set_target(temporal_exec_module, BREADBOARD_TARGET_TEMPORAL);
        print_result("temporal exec module_set_target", res, BREADBOARD_OK);
        res = breadboard_module_assess_executable_subset(temporal_exec_module, &assessment);
        print_result("temporal exec assess_subset", res, BREADBOARD_OK);
        if (assessment.status != BREADBOARD_EXECUTABLE_ASSESSMENT_PLACEHOLDER_ONLY ||
            assessment.reason != BREADBOARD_EXEC_REASON_TARGET_UNSUPPORTED)
        {
            printf("[FAIL] temporal target assessment mismatch\n");
            exit(1);
        }

        breadboard_module_free(temporal_exec_module);
    }

    {
        BreadboardModule* invalid_endpoint_module = NULL;
        BreadboardExecutableAssessment assessment;
        BreadboardDescriptorSpec input_a = { 801u, "a", 1u };
        BreadboardDescriptorSpec output_y = { 802u, "y", 1u };
        BreadboardComponentSpec buf_gate = { 803u, "BUF", false };
        BreadboardExecutableConnectionSpec bad_slot = {
            { BREADBOARD_ENDPOINT_MODULE_INPUT_SOURCE, 801u, 0u, 0u },
            { BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK, 0u, 803u, 1u }
        };
        BreadboardExecutableConnectionSpec drive_output = {
            { BREADBOARD_ENDPOINT_COMPONENT_OUTPUT_SOURCE, 0u, 803u, 0u },
            { BREADBOARD_ENDPOINT_MODULE_OUTPUT_SINK, 802u, 0u, 0u }
        };

        res = breadboard_module_create(&invalid_endpoint_module);
        print_result("invalid endpoint module_create", res, BREADBOARD_OK);
        res = breadboard_module_set_target(invalid_endpoint_module, BREADBOARD_TARGET_FAST_4STATE);
        print_result("invalid endpoint module_set_target", res, BREADBOARD_OK);
        res = breadboard_module_add_input_descriptor(invalid_endpoint_module, &input_a);
        print_result("invalid endpoint add_input", res, BREADBOARD_OK);
        res = breadboard_module_add_output_descriptor(invalid_endpoint_module, &output_y);
        print_result("invalid endpoint add_output", res, BREADBOARD_OK);
        res = breadboard_module_add_component_instance(invalid_endpoint_module, &buf_gate);
        print_result("invalid endpoint add_component", res, BREADBOARD_OK);
        res = breadboard_module_add_executable_connection(invalid_endpoint_module, &bad_slot);
        print_result("invalid endpoint add_connection_bad_slot", res, BREADBOARD_OK);
        res = breadboard_module_add_executable_connection(invalid_endpoint_module, &drive_output);
        print_result("invalid endpoint add_connection_output", res, BREADBOARD_OK);
        res = breadboard_module_assess_executable_subset(invalid_endpoint_module, &assessment);
        print_result("invalid endpoint assess_subset", res, BREADBOARD_OK);
        if (assessment.status != BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID ||
            assessment.reason != BREADBOARD_EXEC_REASON_INVALID_ENDPOINT)
        {
            printf("[FAIL] invalid endpoint assessment mismatch\n");
            exit(1);
        }

        breadboard_module_free(invalid_endpoint_module);
    }

    {
        BreadboardModule* missing_driver_module = NULL;
        BreadboardExecutableAssessment assessment;
        BreadboardDescriptorSpec input_a = { 901u, "a", 1u };
        BreadboardDescriptorSpec output_y = { 902u, "y", 1u };
        BreadboardComponentSpec and_gate = { 903u, "AND", false };
        BreadboardExecutableConnectionSpec first_input = {
            { BREADBOARD_ENDPOINT_MODULE_INPUT_SOURCE, 901u, 0u, 0u },
            { BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK, 0u, 903u, 0u }
        };
        BreadboardExecutableConnectionSpec output_driver = {
            { BREADBOARD_ENDPOINT_COMPONENT_OUTPUT_SOURCE, 0u, 903u, 0u },
            { BREADBOARD_ENDPOINT_MODULE_OUTPUT_SINK, 902u, 0u, 0u }
        };

        res = breadboard_module_create(&missing_driver_module);
        print_result("missing driver module_create", res, BREADBOARD_OK);
        res = breadboard_module_set_target(missing_driver_module, BREADBOARD_TARGET_FAST_4STATE);
        print_result("missing driver module_set_target", res, BREADBOARD_OK);
        res = breadboard_module_add_input_descriptor(missing_driver_module, &input_a);
        print_result("missing driver add_input", res, BREADBOARD_OK);
        res = breadboard_module_add_output_descriptor(missing_driver_module, &output_y);
        print_result("missing driver add_output", res, BREADBOARD_OK);
        res = breadboard_module_add_component_instance(missing_driver_module, &and_gate);
        print_result("missing driver add_component", res, BREADBOARD_OK);
        res = breadboard_module_add_executable_connection(missing_driver_module, &first_input);
        print_result("missing driver add_connection_input", res, BREADBOARD_OK);
        res = breadboard_module_add_executable_connection(missing_driver_module, &output_driver);
        print_result("missing driver add_connection_output", res, BREADBOARD_OK);
        res = breadboard_module_assess_executable_subset(missing_driver_module, &assessment);
        print_result("missing driver assess_subset", res, BREADBOARD_OK);
        if (assessment.status != BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID ||
            assessment.reason != BREADBOARD_EXEC_REASON_MISSING_REQUIRED_DRIVER ||
            assessment.failing_component_id != 903u)
        {
            printf("[FAIL] missing driver assessment mismatch\n");
            exit(1);
        }

        breadboard_module_free(missing_driver_module);
    }

    {
        BreadboardModule* cycle_module = NULL;
        BreadboardExecutableAssessment assessment;
        BreadboardDescriptorSpec input_a = { 1001u, "a", 1u };
        BreadboardDescriptorSpec output_y = { 1002u, "y", 1u };
        BreadboardComponentSpec buf_gate = { 1003u, "BUF", false };
        BreadboardExecutableConnectionSpec self_cycle = {
            { BREADBOARD_ENDPOINT_COMPONENT_OUTPUT_SOURCE, 0u, 1003u, 0u },
            { BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK, 0u, 1003u, 0u }
        };
        BreadboardExecutableConnectionSpec output_driver = {
            { BREADBOARD_ENDPOINT_COMPONENT_OUTPUT_SOURCE, 0u, 1003u, 0u },
            { BREADBOARD_ENDPOINT_MODULE_OUTPUT_SINK, 1002u, 0u, 0u }
        };

        res = breadboard_module_create(&cycle_module);
        print_result("cycle module_create", res, BREADBOARD_OK);
        res = breadboard_module_set_target(cycle_module, BREADBOARD_TARGET_FAST_4STATE);
        print_result("cycle module_set_target", res, BREADBOARD_OK);
        res = breadboard_module_add_input_descriptor(cycle_module, &input_a);
        print_result("cycle add_input", res, BREADBOARD_OK);
        res = breadboard_module_add_output_descriptor(cycle_module, &output_y);
        print_result("cycle add_output", res, BREADBOARD_OK);
        res = breadboard_module_add_component_instance(cycle_module, &buf_gate);
        print_result("cycle add_component", res, BREADBOARD_OK);
        res = breadboard_module_add_executable_connection(cycle_module, &self_cycle);
        print_result("cycle add_connection_self", res, BREADBOARD_OK);
        res = breadboard_module_add_executable_connection(cycle_module, &output_driver);
        print_result("cycle add_connection_output", res, BREADBOARD_OK);
        res = breadboard_module_assess_executable_subset(cycle_module, &assessment);
        print_result("cycle assess_subset", res, BREADBOARD_OK);
        if (assessment.status != BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID ||
            assessment.reason != BREADBOARD_EXEC_REASON_CYCLE_DETECTED ||
            assessment.failing_component_id != 1003u)
        {
            printf("[FAIL] cycle assessment mismatch\n");
            exit(1);
        }

        breadboard_module_free(cycle_module);
    }

    res = breadboard_module_set_identity(module, &module_identity);
    print_result("module_set_identity", res, BREADBOARD_OK);
    res = breadboard_module_set_structure_summary(module, &module_summary);
    print_result("module_set_structure_summary", res, BREADBOARD_OK);

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
    if (draft_info.target != BREADBOARD_TARGET_TEMPORAL ||
        !draft_info.has_placeholders ||
        draft_info.approximate_size_bytes != strata_placeholder_artifact_size() ||
        draft_info.source_module_id != module_identity.module_id ||
        strcmp(draft_info.source_module_name, module_identity.module_name) != 0 ||
        draft_info.declared_component_count != module_summary.declared_component_count ||
        draft_info.declared_connection_count != module_summary.declared_connection_count ||
        draft_info.declared_stateful_node_count != module_summary.declared_stateful_node_count)
    {
        printf("[FAIL] draft info mismatch for TEMPORAL\n");
        exit(1);
    }

    /* 8b. Artifact admission info check (TEMPORAL) */
    BreadboardDraftAdmissionInfo admission_info;
    res = breadboard_draft_query_admission_info(draft, &admission_info);
    print_result("draft_query_admission_info(TEMPORAL)", res, BREADBOARD_OK);
    if (admission_info.target != BREADBOARD_TARGET_TEMPORAL || 
        !admission_info.is_placeholder ||
        !admission_info.requires_advanced_controls ||
        admission_info.requires_native_state_read ||
        admission_info.requires_native_inputs ||
        admission_info.native_only_behavior ||
        admission_info.extension_flags != 1)
    {
        printf("[FAIL] admission info mismatch for TEMPORAL\n");
        exit(1);
    }

    {
        size_t export_size = 0;
        unsigned char export_bytes[strata_placeholder_artifact_size()];
        const StrataPlaceholderArtifactHeader* export_header;
        const StrataPlaceholderSectionEntry* admission_section;
        const StrataPlaceholderSectionEntry* draft_summary_section;
        const StrataPlaceholderSectionEntry* structure_section;
        const StrataPlaceholderSectionEntry* descriptor_section;
        const StrataPlaceholderSectionEntry* payload_section;
        const StrataPlaceholderAdmissionInfo* export_admission_info;
        const StrataPlaceholderDraftSummary* export_draft_summary;

        res = breadboard_artifact_draft_export_placeholder_size(draft, &export_size);
        print_result("draft_export_placeholder_size(TEMPORAL)", res, BREADBOARD_OK);
        if (export_size != sizeof(export_bytes))
        {
            printf("[FAIL] export size mismatch for TEMPORAL\n");
            exit(1);
        }

        res = breadboard_artifact_draft_export_placeholder(
            draft,
            export_bytes,
            sizeof(export_bytes),
            &export_size);
        print_result("draft_export_placeholder(TEMPORAL)", res, BREADBOARD_OK);

        export_header = (const StrataPlaceholderArtifactHeader*)export_bytes;
        admission_section = strata_placeholder_find_section_entry(
            export_header, STRATA_PLACEHOLDER_SECTION_ADMISSION);
        draft_summary_section = strata_placeholder_find_section_entry(
            export_header, STRATA_PLACEHOLDER_SECTION_DRAFT_SUMMARY);
        structure_section = strata_placeholder_find_section_entry(
            export_header, STRATA_PLACEHOLDER_SECTION_STRUCTURE);
        descriptor_section = strata_placeholder_find_section_entry(
            export_header, STRATA_PLACEHOLDER_SECTION_DESCRIPTORS);
        payload_section = strata_placeholder_find_section_entry(
            export_header, STRATA_PLACEHOLDER_SECTION_PAYLOAD);
        export_admission_info = strata_placeholder_artifact_admission_info(export_header);
        export_draft_summary = strata_placeholder_artifact_draft_summary(export_header);
        if (export_header->target_backend_id != STRATA_PLACEHOLDER_BACKEND_ID_HIGHZ ||
            export_header->input_descriptor_count != 2u ||
            export_header->output_descriptor_count != 2u ||
            export_header->probe_descriptor_count != 1u ||
            export_header->section_table_offset != sizeof(StrataPlaceholderArtifactHeader) ||
            export_header->section_count != 5u ||
            !admission_section ||
            admission_section->section_offset !=
                sizeof(StrataPlaceholderArtifactHeader) +
                strata_placeholder_section_table_bytes(5u) ||
            admission_section->section_size != sizeof(StrataPlaceholderAdmissionInfo) ||
            !draft_summary_section ||
            draft_summary_section->section_offset !=
                sizeof(StrataPlaceholderArtifactHeader) +
                strata_placeholder_section_table_bytes(5u) +
                sizeof(StrataPlaceholderAdmissionInfo) ||
            draft_summary_section->section_size != sizeof(StrataPlaceholderDraftSummary) ||
            !structure_section ||
            structure_section->section_offset !=
                sizeof(StrataPlaceholderArtifactHeader) +
                strata_placeholder_section_table_bytes(5u) +
                sizeof(StrataPlaceholderAdmissionInfo) +
                sizeof(StrataPlaceholderDraftSummary) ||
            structure_section->section_size != sizeof(StrataPlaceholderStructureSummary) ||
            export_header->descriptor_offset !=
                sizeof(StrataPlaceholderArtifactHeader) +
                strata_placeholder_section_table_bytes(5u) +
                sizeof(StrataPlaceholderAdmissionInfo) +
                sizeof(StrataPlaceholderDraftSummary) +
                sizeof(StrataPlaceholderStructureSummary) ||
            export_header->payload_offset !=
                sizeof(StrataPlaceholderArtifactHeader) +
                strata_placeholder_section_table_bytes(5u) +
                sizeof(StrataPlaceholderAdmissionInfo) +
                sizeof(StrataPlaceholderDraftSummary) +
                sizeof(StrataPlaceholderStructureSummary) +
                export_header->descriptor_bytes ||
            !descriptor_section ||
            descriptor_section->section_offset != export_header->descriptor_offset ||
            descriptor_section->section_size != export_header->descriptor_bytes ||
            !payload_section ||
            payload_section->section_offset != export_header->payload_offset ||
            payload_section->section_size != export_header->payload_size ||
            export_header->payload_size != STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN ||
            export_header->payload_kind != STRATA_PLACEHOLDER_PAYLOAD_ADVANCED ||
            !export_admission_info ||
            memcmp(export_admission_info,
                &export_header->admission_info,
                sizeof(*export_admission_info)) != 0 ||
            !export_draft_summary ||
            export_draft_summary->source_target_value != BREADBOARD_TARGET_TEMPORAL ||
            export_draft_summary->has_placeholders != 1u ||
            export_draft_summary->approximate_size_bytes != strata_placeholder_artifact_size() ||
            export_draft_summary->source_module_id != module_identity.module_id ||
            strcmp(export_draft_summary->source_module_name, module_identity.module_name) != 0 ||
            export_draft_summary->declared_component_count != module_summary.declared_component_count ||
            export_draft_summary->declared_connection_count != module_summary.declared_connection_count ||
            export_draft_summary->declared_stateful_node_count != module_summary.declared_stateful_node_count ||
            export_header->admission_info.requirement_flags !=
                STRATA_PLACEHOLDER_REQUIREMENT_ADVANCED_CONTROL ||
            !export_header->admission_info.requires_advanced_controls ||
            export_header->admission_info.requires_native_state_read ||
            export_header->admission_info.requires_native_inputs ||
            !strata_placeholder_payload_matches(
                strata_placeholder_artifact_payload(export_header),
                STRATA_PLACEHOLDER_PAYLOAD_ADVANCED))
        {
            printf("[FAIL] exported TEMPORAL placeholder bytes mismatch\n");
            exit(1);
        }
    }

    /* 8c. Artifact admission info check (FAST_4STATE) */
    breadboard_module_set_target_policy(module, BREADBOARD_TARGET_MASK_ALL);
    breadboard_module_set_target(module, BREADBOARD_TARGET_FAST_4STATE);
    BreadboardArtifactDraft* draft_fast = NULL;
    res = breadboard_module_compile(module, &opts_allow, &draft_fast);
    print_result("module_compile(FAST_4STATE)", res, BREADBOARD_OK);

    res = breadboard_artifact_draft_query_info(draft_fast, &draft_info);
    print_result("draft_query_info(FAST_4STATE)", res, BREADBOARD_OK);
    if (draft_info.target != BREADBOARD_TARGET_FAST_4STATE ||
        !draft_info.has_placeholders ||
        draft_info.approximate_size_bytes != strata_placeholder_artifact_size() ||
        draft_info.source_module_id != module_identity.module_id ||
        strcmp(draft_info.source_module_name, module_identity.module_name) != 0 ||
        draft_info.declared_component_count != module_summary.declared_component_count ||
        draft_info.declared_connection_count != module_summary.declared_connection_count ||
        draft_info.declared_stateful_node_count != module_summary.declared_stateful_node_count)
    {
        printf("[FAIL] draft info mismatch for FAST_4STATE\n");
        exit(1);
    }

    BreadboardDraftAdmissionInfo admission_info_fast;
    res = breadboard_draft_query_admission_info(draft_fast, &admission_info_fast);
    print_result("draft_query_admission_info(FAST_4STATE)", res, BREADBOARD_OK);
    if (admission_info_fast.target != BREADBOARD_TARGET_FAST_4STATE || 
        !admission_info_fast.is_placeholder ||
        admission_info_fast.requires_advanced_controls ||
        admission_info_fast.requires_native_state_read ||
        admission_info_fast.requires_native_inputs ||
        admission_info_fast.native_only_behavior ||
        admission_info_fast.extension_flags != 0)
    {
        printf("[FAIL] admission info mismatch for FAST_4STATE\n");
        exit(1);
    }

    {
        size_t export_size = 0;
        unsigned char export_bytes[strata_placeholder_artifact_size()];
        const StrataPlaceholderArtifactHeader* export_header;
        const StrataPlaceholderSectionEntry* admission_section;
        const StrataPlaceholderSectionEntry* draft_summary_section;
        const StrataPlaceholderSectionEntry* structure_section;
        const StrataPlaceholderSectionEntry* descriptor_section;
        const StrataPlaceholderSectionEntry* payload_section;
        const StrataPlaceholderAdmissionInfo* export_admission_info;
        const StrataPlaceholderDraftSummary* export_draft_summary;

        res = breadboard_artifact_draft_export_placeholder_size(draft_fast, &export_size);
        print_result("draft_export_placeholder_size(FAST_4STATE)", res, BREADBOARD_OK);
        if (export_size != sizeof(export_bytes))
        {
            printf("[FAIL] export size mismatch for FAST_4STATE\n");
            exit(1);
        }

        res = breadboard_artifact_draft_export_placeholder(
            draft_fast,
            export_bytes,
            sizeof(export_bytes),
            &export_size);
        print_result("draft_export_placeholder(FAST_4STATE)", res, BREADBOARD_OK);

        export_header = (const StrataPlaceholderArtifactHeader*)export_bytes;
        admission_section = strata_placeholder_find_section_entry(
            export_header, STRATA_PLACEHOLDER_SECTION_ADMISSION);
        draft_summary_section = strata_placeholder_find_section_entry(
            export_header, STRATA_PLACEHOLDER_SECTION_DRAFT_SUMMARY);
        structure_section = strata_placeholder_find_section_entry(
            export_header, STRATA_PLACEHOLDER_SECTION_STRUCTURE);
        descriptor_section = strata_placeholder_find_section_entry(
            export_header, STRATA_PLACEHOLDER_SECTION_DESCRIPTORS);
        payload_section = strata_placeholder_find_section_entry(
            export_header, STRATA_PLACEHOLDER_SECTION_PAYLOAD);
        export_admission_info = strata_placeholder_artifact_admission_info(export_header);
        export_draft_summary = strata_placeholder_artifact_draft_summary(export_header);
        if (export_header->target_backend_id != STRATA_PLACEHOLDER_BACKEND_ID_LXS ||
            export_header->input_descriptor_count != 2u ||
            export_header->output_descriptor_count != 2u ||
            export_header->probe_descriptor_count != 1u ||
            export_header->section_table_offset != sizeof(StrataPlaceholderArtifactHeader) ||
            export_header->section_count != 5u ||
            !admission_section ||
            admission_section->section_offset !=
                sizeof(StrataPlaceholderArtifactHeader) +
                strata_placeholder_section_table_bytes(5u) ||
            admission_section->section_size != sizeof(StrataPlaceholderAdmissionInfo) ||
            !draft_summary_section ||
            draft_summary_section->section_offset !=
                sizeof(StrataPlaceholderArtifactHeader) +
                strata_placeholder_section_table_bytes(5u) +
                sizeof(StrataPlaceholderAdmissionInfo) ||
            draft_summary_section->section_size != sizeof(StrataPlaceholderDraftSummary) ||
            !structure_section ||
            structure_section->section_offset !=
                sizeof(StrataPlaceholderArtifactHeader) +
                strata_placeholder_section_table_bytes(5u) +
                sizeof(StrataPlaceholderAdmissionInfo) +
                sizeof(StrataPlaceholderDraftSummary) ||
            structure_section->section_size != sizeof(StrataPlaceholderStructureSummary) ||
            export_header->descriptor_offset !=
                sizeof(StrataPlaceholderArtifactHeader) +
                strata_placeholder_section_table_bytes(5u) +
                sizeof(StrataPlaceholderAdmissionInfo) +
                sizeof(StrataPlaceholderDraftSummary) +
                sizeof(StrataPlaceholderStructureSummary) ||
            export_header->payload_offset !=
                sizeof(StrataPlaceholderArtifactHeader) +
                strata_placeholder_section_table_bytes(5u) +
                sizeof(StrataPlaceholderAdmissionInfo) +
                sizeof(StrataPlaceholderDraftSummary) +
                sizeof(StrataPlaceholderStructureSummary) +
                export_header->descriptor_bytes ||
            !descriptor_section ||
            descriptor_section->section_offset != export_header->descriptor_offset ||
            descriptor_section->section_size != export_header->descriptor_bytes ||
            !payload_section ||
            payload_section->section_offset != export_header->payload_offset ||
            payload_section->section_size != export_header->payload_size ||
            export_header->payload_size != STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN ||
            export_header->payload_kind != STRATA_PLACEHOLDER_PAYLOAD_BASELINE ||
            !export_admission_info ||
            memcmp(export_admission_info,
                &export_header->admission_info,
                sizeof(*export_admission_info)) != 0 ||
            !export_draft_summary ||
            export_draft_summary->source_target_value != BREADBOARD_TARGET_FAST_4STATE ||
            export_draft_summary->has_placeholders != 1u ||
            export_draft_summary->approximate_size_bytes != strata_placeholder_artifact_size() ||
            export_draft_summary->source_module_id != module_identity.module_id ||
            strcmp(export_draft_summary->source_module_name, module_identity.module_name) != 0 ||
            export_draft_summary->declared_component_count != module_summary.declared_component_count ||
            export_draft_summary->declared_connection_count != module_summary.declared_connection_count ||
            export_draft_summary->declared_stateful_node_count != module_summary.declared_stateful_node_count ||
            export_header->admission_info.requirement_flags !=
                STRATA_PLACEHOLDER_REQUIREMENT_NONE ||
            export_header->admission_info.requires_advanced_controls ||
            export_header->admission_info.requires_native_state_read ||
            export_header->admission_info.requires_native_inputs ||
            !strata_placeholder_payload_matches(
                strata_placeholder_artifact_payload(export_header),
                STRATA_PLACEHOLDER_PAYLOAD_BASELINE))
        {
            printf("[FAIL] exported FAST_4STATE placeholder bytes mismatch\n");
            exit(1);
        }
    }

    breadboard_module_set_target_policy(module, BREADBOARD_TARGET_MASK_TEMPORAL);
    breadboard_module_set_target(module, BREADBOARD_TARGET_TEMPORAL); /* Restore */

    /* 9. Test diagnostics */
    size_t diag_count = 99;
    res = breadboard_module_get_diagnostic_count(module, &diag_count);
    print_result("module_get_diagnostic_count", res, BREADBOARD_OK);
    /* We expect 4 diagnostics: denied target, compilation refusal without placeholders, and two warnings from successful compiles */
    if (diag_count != 4)
    {
        printf("[FAIL] expected 4 diagnostics, got %zu\n", diag_count);
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

    /* 12b. Authored descriptor declarations should drive draft structure when present */
    {
        BreadboardModule* authored_module = NULL;
        BreadboardArtifactDraft* authored_draft = NULL;
        BreadboardDescriptorSpec input_spec = { 1000u, "user_in", 4u };
        BreadboardDescriptorSpec output_spec = { 2000u, "user_out", 2u };
        BreadboardDescriptorSpec probe_spec = { 3000u, "user_probe", 1u };
        BreadboardModuleIdentity authored_identity = { 0xBEEFu, "authored_temporal" };
        BreadboardStructureSummary authored_summary = { 3u, 2u, 1u };

        res = breadboard_module_create(&authored_module);
        print_result("authored module_create", res, BREADBOARD_OK);
        res = breadboard_module_set_target(authored_module, BREADBOARD_TARGET_TEMPORAL);
        print_result("authored module_set_target", res, BREADBOARD_OK);
        res = breadboard_module_set_identity(authored_module, &authored_identity);
        print_result("authored module_set_identity", res, BREADBOARD_OK);
        res = breadboard_module_set_structure_summary(authored_module, &authored_summary);
        print_result("authored module_set_structure_summary", res, BREADBOARD_OK);
        res = breadboard_module_add_input_descriptor(authored_module, &input_spec);
        print_result("authored add_input", res, BREADBOARD_OK);
        res = breadboard_module_add_output_descriptor(authored_module, &output_spec);
        print_result("authored add_output", res, BREADBOARD_OK);
        res = breadboard_module_add_probe_descriptor(authored_module, &probe_spec);
        print_result("authored add_probe", res, BREADBOARD_OK);
        res = breadboard_module_compile(authored_module, &opts_allow, &authored_draft);
        print_result("authored module_compile", res, BREADBOARD_OK);

        res = breadboard_artifact_draft_query_info(authored_draft, &draft_info);
        print_result("authored draft_query_info", res, BREADBOARD_OK);
        if (draft_info.target != BREADBOARD_TARGET_TEMPORAL ||
            draft_info.has_placeholders ||
            draft_info.approximate_size_bytes !=
                strata_placeholder_artifact_size_for_counts(1u, 1u, 1u) ||
            draft_info.source_module_id != authored_identity.module_id ||
            strcmp(draft_info.source_module_name, authored_identity.module_name) != 0 ||
            draft_info.declared_component_count != authored_summary.declared_component_count ||
            draft_info.declared_connection_count != authored_summary.declared_connection_count ||
            draft_info.declared_stateful_node_count != authored_summary.declared_stateful_node_count)
        {
            printf("[FAIL] authored draft info mismatch\n");
            exit(1);
        }

        res = breadboard_draft_query_admission_info(authored_draft, &admission_info);
        print_result("authored draft_query_admission_info", res, BREADBOARD_OK);
        if (!admission_info.is_placeholder || !admission_info.requires_advanced_controls)
        {
            printf("[FAIL] authored admission info mismatch\n");
            exit(1);
        }

        res = breadboard_draft_input_descriptor_count(authored_draft, &in_count);
        print_result("authored input_count", res, BREADBOARD_OK);
        res = breadboard_draft_output_descriptor_count(authored_draft, &out_count);
        print_result("authored output_count", res, BREADBOARD_OK);
        res = breadboard_draft_probe_descriptor_count(authored_draft, &probe_count);
        print_result("authored probe_count", res, BREADBOARD_OK);
        if (in_count != 1u || out_count != 1u || probe_count != 1u)
        {
            printf("[FAIL] authored descriptor counts mismatch\n");
            exit(1);
        }

        res = breadboard_draft_input_descriptor_by_name(authored_draft, "user_in", &lookup_desc);
        print_result("authored input by name", res, BREADBOARD_OK);
        if (lookup_desc.id != 1000u || lookup_desc.is_placeholder)
        {
            printf("[FAIL] authored input descriptor mismatch\n");
            exit(1);
        }

        res = breadboard_draft_output_descriptor_by_id(authored_draft, 2000u, &lookup_desc);
        print_result("authored output by id", res, BREADBOARD_OK);
        if (lookup_desc.width != 2u || lookup_desc.is_placeholder)
        {
            printf("[FAIL] authored output descriptor mismatch\n");
            exit(1);
        }

        res = breadboard_draft_probe_descriptor_by_name(authored_draft, "user_probe", &lookup_desc);
        print_result("authored probe by name", res, BREADBOARD_OK);
        if (lookup_desc.id != 3000u || lookup_desc.is_placeholder)
        {
            printf("[FAIL] authored probe descriptor mismatch\n");
            exit(1);
        }

        breadboard_artifact_draft_free(authored_draft);
        breadboard_module_free(authored_module);
    }

    /* 12c. Authored requirement profile should drive native placeholder payloads */
    {
        BreadboardModule* native_module = NULL;
        BreadboardArtifactDraft* native_draft = NULL;
        BreadboardRequirementProfile native_profile = {
            1u, false, true, true
        };
        size_t export_size = 0;
        unsigned char export_bytes[strata_placeholder_artifact_size()];
        const StrataPlaceholderArtifactHeader* export_header;

        res = breadboard_module_create(&native_module);
        print_result("native module_create", res, BREADBOARD_OK);
        res = breadboard_module_set_target(native_module, BREADBOARD_TARGET_TEMPORAL);
        print_result("native module_set_target", res, BREADBOARD_OK);
        res = breadboard_module_set_requirement_profile(native_module, &native_profile);
        print_result("native module_set_requirement_profile", res, BREADBOARD_OK);
        res = breadboard_module_compile(native_module, &opts_allow, &native_draft);
        print_result("native module_compile", res, BREADBOARD_OK);

        res = breadboard_draft_query_admission_info(native_draft, &admission_info);
        print_result("native draft_query_admission_info", res, BREADBOARD_OK);
        if (admission_info.requires_advanced_controls ||
            !admission_info.requires_native_state_read ||
            !admission_info.requires_native_inputs ||
            !admission_info.native_only_behavior)
        {
            printf("[FAIL] native authored requirement profile mismatch\n");
            exit(1);
        }

        res = breadboard_artifact_draft_export_placeholder_size(native_draft, &export_size);
        print_result("native draft_export_placeholder_size", res, BREADBOARD_OK);
        res = breadboard_artifact_draft_export_placeholder(
            native_draft,
            export_bytes,
            sizeof(export_bytes),
            &export_size);
        print_result("native draft_export_placeholder", res, BREADBOARD_OK);
        export_header = (const StrataPlaceholderArtifactHeader*)export_bytes;
        if (export_header->payload_kind != STRATA_PLACEHOLDER_PAYLOAD_NATIVE ||
            !export_header->admission_info.requires_native_state_read ||
            !export_header->admission_info.requires_native_inputs)
        {
            printf("[FAIL] native payload export mismatch\n");
            exit(1);
        }

        breadboard_artifact_draft_free(native_draft);
        breadboard_module_free(native_module);
    }

    /* 12d. Authored structural declarations should derive coarse structure summary */
    {
        BreadboardModule* structured_module = NULL;
        BreadboardArtifactDraft* structured_draft = NULL;
        BreadboardModuleIdentity structured_identity = { 0xCAFEu, "structured_temporal" };
        BreadboardStructureSummary misleading_summary = { 99u, 77u, 66u };
        BreadboardComponentSpec component_a = { 10u, "and_gate", false };
        BreadboardComponentSpec component_b = { 11u, "register", true };
        BreadboardComponentSpec component_c = { 12u, "probe_tap", false };
        BreadboardConnectionSpec connection_ab = { 10u, 11u };
        BreadboardConnectionSpec connection_bc = { 11u, 12u };
        BreadboardComponent component;
        BreadboardConnection connection;
        size_t component_count = 0u;
        size_t connection_count = 0u;

        res = breadboard_module_create(&structured_module);
        print_result("structured module_create", res, BREADBOARD_OK);
        res = breadboard_module_set_target(structured_module, BREADBOARD_TARGET_TEMPORAL);
        print_result("structured module_set_target", res, BREADBOARD_OK);
        res = breadboard_module_set_identity(structured_module, &structured_identity);
        print_result("structured module_set_identity", res, BREADBOARD_OK);
        res = breadboard_module_set_structure_summary(structured_module, &misleading_summary);
        print_result("structured module_set_structure_summary", res, BREADBOARD_OK);
        res = breadboard_module_add_component_instance(structured_module, &component_a);
        print_result("structured add_component_a", res, BREADBOARD_OK);
        res = breadboard_module_add_component_instance(structured_module, &component_b);
        print_result("structured add_component_b", res, BREADBOARD_OK);
        res = breadboard_module_add_component_instance(structured_module, &component_c);
        print_result("structured add_component_c", res, BREADBOARD_OK);
        res = breadboard_module_add_connection(structured_module, &connection_ab);
        print_result("structured add_connection_ab", res, BREADBOARD_OK);
        res = breadboard_module_add_connection(structured_module, &connection_bc);
        print_result("structured add_connection_bc", res, BREADBOARD_OK);
        res = breadboard_module_compile(structured_module, &opts_allow, &structured_draft);
        print_result("structured module_compile", res, BREADBOARD_OK);

        res = breadboard_artifact_draft_query_info(structured_draft, &draft_info);
        print_result("structured draft_query_info", res, BREADBOARD_OK);
        if (draft_info.declared_component_count != 3u ||
            draft_info.declared_connection_count != 2u ||
            draft_info.declared_stateful_node_count != 1u)
        {
            printf("[FAIL] structured draft info summary mismatch\n");
            exit(1);
        }

        res = breadboard_draft_component_count(structured_draft, &component_count);
        print_result("structured draft_component_count", res, BREADBOARD_OK);
        res = breadboard_draft_connection_count(structured_draft, &connection_count);
        print_result("structured draft_connection_count", res, BREADBOARD_OK);
        if (component_count != 3u || connection_count != 2u)
        {
            printf("[FAIL] structured draft count query mismatch\n");
            exit(1);
        }

        res = breadboard_draft_component_at(structured_draft, 1u, &component);
        print_result("structured draft_component_at(1)", res, BREADBOARD_OK);
        if (component.id != 11u ||
            strcmp(component.kind_name, "register") != 0 ||
            !component.is_stateful)
        {
            printf("[FAIL] structured draft component query mismatch\n");
            exit(1);
        }

        res = breadboard_draft_connection_at(structured_draft, 1u, &connection);
        print_result("structured draft_connection_at(1)", res, BREADBOARD_OK);
        if (connection.source_component_id != 11u ||
            connection.sink_component_id != 12u)
        {
            printf("[FAIL] structured draft connection query mismatch\n");
            exit(1);
        }

        res = breadboard_draft_component_by_id(structured_draft, 12u, &component);
        print_result("structured draft_component_by_id(12)", res, BREADBOARD_OK);
        if (component.id != 12u ||
            strcmp(component.kind_name, "probe_tap") != 0 ||
            component.is_stateful)
        {
            printf("[FAIL] structured draft component-by-id query mismatch\n");
            exit(1);
        }

        breadboard_artifact_draft_free(structured_draft);
        breadboard_module_free(structured_module);
    }

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

    res = breadboard_draft_query_admission_info(NULL, &admission_info);
    print_result("draft_query_admission_info(NULL, ...)", res, BREADBOARD_ERR_INVALID_ARGUMENT);

    res = breadboard_draft_query_admission_info(draft, NULL);
    print_result("draft_query_admission_info(..., NULL)", res, BREADBOARD_ERR_INVALID_ARGUMENT);

    res = breadboard_artifact_draft_query_info(NULL, &draft_info);
    print_result("draft_query_info(NULL, ...)", res, BREADBOARD_ERR_INVALID_ARGUMENT);

    res = breadboard_artifact_draft_query_info(draft, NULL);
    print_result("draft_query_info(..., NULL)", res, BREADBOARD_ERR_INVALID_ARGUMENT);

    {
        BreadboardComponent component;
        BreadboardConnection connection;
        size_t count = 0u;

        res = breadboard_draft_component_count(NULL, &count);
        print_result("draft_component_count(NULL, ...)", res, BREADBOARD_ERR_INVALID_ARGUMENT);
        res = breadboard_draft_component_count(draft, NULL);
        print_result("draft_component_count(..., NULL)", res, BREADBOARD_ERR_INVALID_ARGUMENT);
        res = breadboard_draft_component_at(NULL, 0u, &component);
        print_result("draft_component_at(NULL, ...)", res, BREADBOARD_ERR_INVALID_ARGUMENT);
        res = breadboard_draft_component_at(draft, 0u, NULL);
        print_result("draft_component_at(..., NULL)", res, BREADBOARD_ERR_INVALID_ARGUMENT);
        res = breadboard_draft_component_at(draft, 0u, &component);
        print_result("draft_component_at(placeholder draft, 0)", res, BREADBOARD_ERR_OUT_OF_BOUNDS);
        res = breadboard_draft_component_by_id(NULL, 1u, &component);
        print_result("draft_component_by_id(NULL, ...)", res, BREADBOARD_ERR_INVALID_ARGUMENT);
        res = breadboard_draft_component_by_id(draft, 1u, NULL);
        print_result("draft_component_by_id(..., NULL)", res, BREADBOARD_ERR_INVALID_ARGUMENT);
        res = breadboard_draft_component_by_id(draft, 1u, &component);
        print_result("draft_component_by_id(placeholder draft, 1)", res, BREADBOARD_ERR_NOT_FOUND);

        res = breadboard_draft_connection_count(NULL, &count);
        print_result("draft_connection_count(NULL, ...)", res, BREADBOARD_ERR_INVALID_ARGUMENT);
        res = breadboard_draft_connection_count(draft, NULL);
        print_result("draft_connection_count(..., NULL)", res, BREADBOARD_ERR_INVALID_ARGUMENT);
        res = breadboard_draft_connection_at(NULL, 0u, &connection);
        print_result("draft_connection_at(NULL, ...)", res, BREADBOARD_ERR_INVALID_ARGUMENT);
        res = breadboard_draft_connection_at(draft, 0u, NULL);
        print_result("draft_connection_at(..., NULL)", res, BREADBOARD_ERR_INVALID_ARGUMENT);
        res = breadboard_draft_connection_at(draft, 0u, &connection);
        print_result("draft_connection_at(placeholder draft, 0)", res, BREADBOARD_ERR_OUT_OF_BOUNDS);
    }

    res = breadboard_module_set_structure_summary(module, NULL);
    print_result("module_set_structure_summary(NULL)", res, BREADBOARD_ERR_INVALID_ARGUMENT);

    {
        BreadboardModule* invalid_structure_module = NULL;
        BreadboardComponentSpec duplicate_component = { 1u, "gate", false };
        BreadboardConnectionSpec missing_connection = { 1u, 2u };

        res = breadboard_module_create(&invalid_structure_module);
        print_result("invalid structured module_create", res, BREADBOARD_OK);
        res = breadboard_module_set_target(invalid_structure_module, BREADBOARD_TARGET_TEMPORAL);
        print_result("invalid structured module_set_target", res, BREADBOARD_OK);
        res = breadboard_module_add_component_instance(invalid_structure_module, &duplicate_component);
        print_result("invalid structured add_component first", res, BREADBOARD_OK);
        res = breadboard_module_add_component_instance(invalid_structure_module, &duplicate_component);
        print_result("invalid structured add_component duplicate", res, BREADBOARD_ERR_INVALID_ARGUMENT);
        res = breadboard_module_add_connection(invalid_structure_module, &missing_connection);
        print_result("invalid structured add_connection missing endpoint", res, BREADBOARD_ERR_INVALID_ARGUMENT);
        breadboard_module_free(invalid_structure_module);
    }

    {
        BreadboardRequirementProfile invalid_profile = { 0u, false, true, true };
        BreadboardModule* fast_module = NULL;
        res = breadboard_module_create(&fast_module);
        print_result("fast module_create(for invalid profile)", res, BREADBOARD_OK);
        res = breadboard_module_set_target(fast_module, BREADBOARD_TARGET_FAST_4STATE);
        print_result("fast module_set_target(for invalid profile)", res, BREADBOARD_OK);
        res = breadboard_module_set_requirement_profile(fast_module, &invalid_profile);
        print_result("module_set_requirement_profile(invalid FAST profile)", res, BREADBOARD_ERR_INVALID_ARGUMENT);
        breadboard_module_free(fast_module);
    }

    {
        size_t export_size = 0;
        unsigned char export_bytes[strata_placeholder_artifact_size()];

        res = breadboard_artifact_draft_export_placeholder_size(NULL, &export_size);
        print_result("draft_export_placeholder_size(NULL, ...)", res, BREADBOARD_ERR_INVALID_ARGUMENT);

        res = breadboard_artifact_draft_export_placeholder(draft, NULL, sizeof(export_bytes), &export_size);
        print_result("draft_export_placeholder(NULL buffer)", res, BREADBOARD_ERR_INVALID_ARGUMENT);

        res = breadboard_artifact_draft_export_placeholder(draft, export_bytes, sizeof(export_bytes) - 1u, &export_size);
        print_result("draft_export_placeholder(short buffer)", res, BREADBOARD_ERR_INVALID_ARGUMENT);
    }

    /* 16. Free up */
    breadboard_artifact_draft_free(draft_fast);
    breadboard_artifact_draft_free(draft);
    breadboard_module_free(module);

    printf("All skeleton tests passed.\n");
    return 0;
}
