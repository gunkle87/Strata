#include "breadboard_api.h"
#include "breadboard_internal.h"
#include "../../include/strata_placeholder_artifact.h"
#include <stdlib.h>
#include <string.h>

static BreadboardResult
resolve_placeholder_backend_id(
    BreadboardTarget target,
    uint32_t* out_backend_id)
{
    if (!out_backend_id)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    switch (target)
    {
        case BREADBOARD_TARGET_FAST_4STATE:
            *out_backend_id = STRATA_PLACEHOLDER_BACKEND_ID_LXS;
            return BREADBOARD_OK;
        case BREADBOARD_TARGET_TEMPORAL:
            *out_backend_id = STRATA_PLACEHOLDER_BACKEND_ID_HIGHZ;
            return BREADBOARD_OK;
        default:
            return BREADBOARD_ERR_UNSUPPORTED;
    }
}

static int
fill_placeholder_admission_info(
    const BreadboardArtifactDraft* draft,
    StrataPlaceholderPayloadKind payload_kind,
    StrataPlaceholderAdmissionInfo* out_info)
{
    if (!draft || !out_info)
    {
        return 0;
    }

    if (!strata_placeholder_expected_admission_info(payload_kind, out_info))
    {
        return 0;
    }

    if (!draft->admission_info.is_placeholder)
    {
        return 0;
    }

    return 1;
}

static int
copy_export_admission_info(
    const BreadboardDraftAdmissionInfo* source,
    StrataPlaceholderAdmissionInfo* out_info)
{
    if (!source || !out_info)
    {
        return 0;
    }

    out_info->requirement_flags = source->extension_flags;
    out_info->requires_advanced_controls =
        source->requires_advanced_controls ? 1u : 0u;
    out_info->requires_native_state_read =
        source->requires_native_state_read ? 1u : 0u;
    out_info->requires_native_inputs =
        source->requires_native_inputs ? 1u : 0u;
    return 1;
}

static void
copy_placeholder_descriptors(
    BreadboardDescriptor* out_descriptors,
    const StrataPlaceholderDescriptorSpec* specs,
    size_t count)
{
    size_t index;

    if (!out_descriptors || !specs)
    {
        return;
    }

    for (index = 0u; index < count; ++index)
    {
        out_descriptors[index].id = specs[index].id;
        out_descriptors[index].name = specs[index].name;
        out_descriptors[index].width = specs[index].width;
        out_descriptors[index].class_type =
            (BreadboardDescriptorClass)specs[index].class_type;
        out_descriptors[index].is_placeholder = true;
    }
}

static BreadboardResult
copy_module_identity(
    uint64_t module_id,
    const char* module_name,
    uint64_t* out_module_id,
    char out_module_name[STRATA_PLACEHOLDER_MODULE_NAME_CAPACITY])
{
    size_t name_len;

    if (!out_module_id || !out_module_name)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    *out_module_id = module_id;
    memset(out_module_name, 0, STRATA_PLACEHOLDER_MODULE_NAME_CAPACITY);

    if (!module_name)
    {
        return BREADBOARD_OK;
    }

    name_len = strlen(module_name);
    if (name_len >= STRATA_PLACEHOLDER_MODULE_NAME_CAPACITY)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    memcpy(out_module_name, module_name, name_len);
    return BREADBOARD_OK;
}

static BreadboardResult
validate_requirement_profile(
    BreadboardTarget target,
    const BreadboardRequirementProfile* profile)
{
    if (!profile)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (profile->requires_advanced_controls &&
        (profile->requires_native_state_read || profile->requires_native_inputs))
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (target == BREADBOARD_TARGET_FAST_4STATE &&
        (profile->requires_advanced_controls ||
         profile->requires_native_state_read ||
         profile->requires_native_inputs))
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    return BREADBOARD_OK;
}

static void
fill_default_structure_summary(BreadboardStructureSummary* out_summary)
{
    if (!out_summary)
    {
        return;
    }

    memset(out_summary, 0, sizeof(*out_summary));
}

static void
fill_default_requirement_profile(
    BreadboardTarget target,
    BreadboardRequirementProfile* out_profile)
{
    if (!out_profile)
    {
        return;
    }

    memset(out_profile, 0, sizeof(*out_profile));

    if (target == BREADBOARD_TARGET_TEMPORAL)
    {
        out_profile->extension_flags = 1u;
        out_profile->requires_advanced_controls = true;
    }
}

static uint32_t
target_allowed_projection_families_mask(BreadboardTarget target)
{
    switch (target)
    {
        case BREADBOARD_TARGET_FAST_4STATE:
            /* FAST_4STATE supports initialization and strength distinction,
               but not backend-specific (since it's a reduced-state backend). */
            return (1u << STRATA_PROJECTION_FAMILY_INITIALIZATION) |
                   (1u << STRATA_PROJECTION_FAMILY_STRENGTH_DISTINCTION);
        case BREADBOARD_TARGET_TEMPORAL:
            /* TEMPORAL supports all families (full-state backend). */
            return (1u << STRATA_PROJECTION_FAMILY_INITIALIZATION) |
                   (1u << STRATA_PROJECTION_FAMILY_STRENGTH_DISTINCTION) |
                   (1u << STRATA_PROJECTION_FAMILY_BACKEND_SPECIFIC);
        default:
            /* Unknown target: allow none. */
            return 0u;
    }
}

static void
fill_default_projection_policy(
    BreadboardTarget target,
    BreadboardProjectionPolicy* out_policy)
{
    if (!out_policy)
    {
        return;
    }

    memset(out_policy, 0, sizeof(*out_policy));
    out_policy->allowed_families_mask = target_allowed_projection_families_mask(target);
    /* Approximation denial and strict projection default to false. */
    out_policy->deny_approximation = false;
    out_policy->strict_projection = false;
    out_policy->generate_report = false;
}

static void
free_descriptor_array(
    BreadboardDescriptor* descriptors,
    size_t count)
{
    size_t index;

    if (!descriptors)
    {
        return;
    }

    for (index = 0u; index < count; ++index)
    {
        if (!descriptors[index].is_placeholder && descriptors[index].name)
        {
            free((void*)descriptors[index].name);
        }
    }

    free(descriptors);
}

static int
copy_descriptor_name(
    const char* name,
    const char** out_name)
{
    size_t len;
    char* copy;

    if (!name || !out_name)
    {
        return 0;
    }

    len = strlen(name);
    copy = (char*)malloc(len + 1u);
    if (!copy)
    {
        return 0;
    }

    memcpy(copy, name, len + 1u);
    *out_name = copy;
    return 1;
}

static void
free_component_array(
    BreadboardComponent* components,
    size_t count)
{
    size_t index;

    if (!components)
    {
        return;
    }

    for (index = 0u; index < count; ++index)
    {
        if (components[index].kind_name)
        {
            free((void*)components[index].kind_name);
        }
    }

    free(components);
}

static int
copy_component_array(
    BreadboardComponent** out_components,
    const BreadboardComponent* components,
    size_t count)
{
    BreadboardComponent* copy;
    size_t index;

    if (!out_components)
    {
        return 0;
    }

    *out_components = NULL;

    if (count == 0u)
    {
        return 1;
    }

    if (!components)
    {
        return 0;
    }

    copy = (BreadboardComponent*)calloc(count, sizeof(BreadboardComponent));
    if (!copy)
    {
        return 0;
    }

    for (index = 0u; index < count; ++index)
    {
        copy[index].id = components[index].id;
        copy[index].is_stateful = components[index].is_stateful;
        if (!copy_descriptor_name(components[index].kind_name, &copy[index].kind_name))
        {
            free_component_array(copy, index);
            return 0;
        }
    }

    *out_components = copy;
    return 1;
}

static int
copy_connection_array(
    BreadboardConnection** out_connections,
    const BreadboardConnection* connections,
    size_t count)
{
    BreadboardConnection* copy;

    if (!out_connections)
    {
        return 0;
    }

    *out_connections = NULL;

    if (count == 0u)
    {
        return 1;
    }

    if (!connections)
    {
        return 0;
    }

    copy = (BreadboardConnection*)calloc(count, sizeof(BreadboardConnection));
    if (!copy)
    {
        return 0;
    }

    memcpy(copy, connections, count * sizeof(BreadboardConnection));
    *out_connections = copy;
    return 1;
}

static int
copy_executable_structure_connection_array(
    BreadboardConnection** out_connections,
    size_t* out_count,
    const BreadboardExecutableConnectionSpec* connections,
    size_t count)
{
    BreadboardConnection* copy;
    size_t index;
    size_t structural_count;
    size_t write_index;

    if (!out_connections || !out_count)
    {
        return 0;
    }

    *out_connections = NULL;
    *out_count = 0u;

    if (count == 0u)
    {
        return 1;
    }

    if (!connections)
    {
        return 0;
    }

    structural_count = 0u;
    for (index = 0u; index < count; ++index)
    {
        if (connections[index].source.endpoint_class ==
                BREADBOARD_ENDPOINT_COMPONENT_OUTPUT_SOURCE &&
            connections[index].sink.endpoint_class ==
                BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK)
        {
            structural_count += 1u;
        }
    }

    if (structural_count == 0u)
    {
        return 1;
    }

    copy = (BreadboardConnection*)calloc(structural_count, sizeof(BreadboardConnection));
    if (!copy)
    {
        return 0;
    }

    write_index = 0u;
    for (index = 0u; index < count; ++index)
    {
        if (connections[index].source.endpoint_class ==
                BREADBOARD_ENDPOINT_COMPONENT_OUTPUT_SOURCE &&
            connections[index].sink.endpoint_class ==
                BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK)
        {
            copy[write_index].source_component_id =
                connections[index].source.component_id;
            copy[write_index].sink_component_id =
                connections[index].sink.component_id;
            write_index += 1u;
        }
    }

    *out_connections = copy;
    *out_count = structural_count;
    return 1;
}

static int
module_find_component_index(
    const BreadboardModule* module,
    uint64_t component_id,
    size_t* out_index)
{
    size_t index;

    if (!module || !out_index)
    {
        return 0;
    }

    for (index = 0u; index < module->component_count; ++index)
    {
        if (module->components[index].id == component_id)
        {
            *out_index = index;
            return 1;
        }
    }

    return 0;
}

static int
find_descriptor_index(
    const BreadboardDescriptor* descriptors,
    size_t count,
    uint64_t descriptor_id,
    size_t* out_index)
{
    size_t index;

    if (!descriptors || !out_index)
    {
        return 0;
    }

    for (index = 0u; index < count; ++index)
    {
        if (descriptors[index].id == descriptor_id)
        {
            *out_index = index;
            return 1;
        }
    }

    return 0;
}

static int
copy_declared_descriptors(
    BreadboardDescriptor** out_descriptors,
    const BreadboardDescriptor* descriptors,
    size_t count)
{
    BreadboardDescriptor* copy;
    size_t index;

    if (!out_descriptors)
    {
        return 0;
    }

    *out_descriptors = NULL;

    if (count == 0u)
    {
        return 1;
    }

    if (!descriptors)
    {
        return 0;
    }

    copy = (BreadboardDescriptor*)calloc(count, sizeof(BreadboardDescriptor));
    if (!copy)
    {
        return 0;
    }

    for (index = 0u; index < count; ++index)
    {
        copy[index].id = descriptors[index].id;
        copy[index].width = descriptors[index].width;
        copy[index].class_type = descriptors[index].class_type;
        copy[index].is_placeholder = descriptors[index].is_placeholder;

        if (!copy[index].is_placeholder)
        {
            if (!copy_descriptor_name(descriptors[index].name, &copy[index].name))
            {
                free_descriptor_array(copy, index);
                return 0;
            }
        }
        else
        {
            copy[index].name = descriptors[index].name;
        }
    }

    *out_descriptors = copy;
    return 1;
}

static void
clear_fast_payload_records(BreadboardArtifactDraft* draft)
{
    if (!draft)
    {
        return;
    }

    free(draft->fast_signals);
    free(draft->fast_primitives);
    free(draft->fast_input_bindings);
    free(draft->fast_output_bindings);

    draft->fast_signals = NULL;
    draft->fast_primitives = NULL;
    draft->fast_input_bindings = NULL;
    draft->fast_output_bindings = NULL;
    draft->fast_signal_count = 0u;
    draft->fast_primitive_count = 0u;
    draft->fast_input_binding_count = 0u;
    draft->fast_output_binding_count = 0u;
}

static BreadboardResult
record_diagnostic(
    BreadboardModule* module,
    BreadboardDiagnosticSeverity severity,
    BreadboardDiagnosticCode code,
    const char* message);

static BreadboardResult
fail_real_fast_payload(
    const BreadboardModule* module,
    BreadboardArtifactDraft* draft,
    const char* message)
{
    clear_fast_payload_records(draft);
    if (module && message)
    {
        record_diagnostic(
            (BreadboardModule*)module,
            BREADBOARD_DIAG_ERROR,
            BREADBOARD_DIAG_CODE_EXECUTABLE_LOWERING_UNAVAILABLE,
            message);
    }
    return BREADBOARD_ERR_COMPILE_FAILED;
}

static uint32_t
primitive_opcode_from_kind(BreadboardPrimitiveKind primitive_kind)
{
    switch (primitive_kind)
    {
        case BREADBOARD_PRIMITIVE_BUF:
            return STRATA_FAST_EXEC_OPCODE_BUF;
        case BREADBOARD_PRIMITIVE_NOT:
            return STRATA_FAST_EXEC_OPCODE_NOT;
        case BREADBOARD_PRIMITIVE_AND:
            return STRATA_FAST_EXEC_OPCODE_AND;
        case BREADBOARD_PRIMITIVE_OR:
            return STRATA_FAST_EXEC_OPCODE_OR;
        case BREADBOARD_PRIMITIVE_XOR:
            return STRATA_FAST_EXEC_OPCODE_XOR;
        default:
            return STRATA_FAST_EXEC_OPCODE_INVALID;
    }
}

static size_t
real_fast_payload_bytes_for_counts(
    size_t primitive_count,
    size_t signal_count,
    size_t input_binding_count,
    size_t output_binding_count)
{
    if (primitive_count > (size_t)UINT32_MAX ||
        signal_count > (size_t)UINT32_MAX ||
        input_binding_count > (size_t)UINT32_MAX ||
        output_binding_count > (size_t)UINT32_MAX)
    {
        return 0u;
    }

    return strata_placeholder_fast_payload_bytes_for_counts(
        (uint32_t)primitive_count,
        (uint32_t)signal_count,
        (uint32_t)input_binding_count,
        (uint32_t)output_binding_count);
}

static size_t
real_fast_artifact_size_for_layout(
    uint32_t input_count,
    uint32_t output_count,
    uint32_t probe_count,
    size_t primitive_count,
    size_t signal_count,
    size_t input_binding_count,
    size_t output_binding_count,
    uint32_t component_count,
    uint32_t connection_count)
{
    size_t payload_bytes;

    payload_bytes = real_fast_payload_bytes_for_counts(
        primitive_count,
        signal_count,
        input_binding_count,
        output_binding_count);
    if (payload_bytes == 0u)
    {
        return 0u;
    }

    return sizeof(StrataPlaceholderArtifactHeader) +
        strata_placeholder_section_table_bytes(5u) +
        sizeof(StrataPlaceholderAdmissionInfo) +
        sizeof(StrataPlaceholderDraftSummary) +
        strata_placeholder_structure_bytes_for_counts(
            component_count,
            connection_count) +
        strata_placeholder_descriptor_bytes_for_counts(
            input_count,
            output_count,
            probe_count) +
        payload_bytes;
}

static int
primitive_kind_from_name(
    const char* kind_name,
    BreadboardPrimitiveKind* out_kind);

static uint32_t
primitive_input_count(
    BreadboardPrimitiveKind kind);

static BreadboardResult
build_real_fast_payload(
    const BreadboardModule* module,
    BreadboardArtifactDraft* draft)
{
    size_t component_index;
    size_t connection_index;
    size_t input_index;
    size_t output_index;
    size_t signal_count;
    BreadboardPrimitiveKind primitive_kind;
    uint32_t required_inputs;

    if (!module || !draft)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    signal_count = module->input_count + module->component_count;
    if (signal_count > (size_t)UINT32_MAX ||
        module->component_count > (size_t)UINT32_MAX ||
        module->input_count > (size_t)UINT32_MAX ||
        module->output_count > (size_t)UINT32_MAX)
    {
        return BREADBOARD_ERR_INTERNAL;
    }

    clear_fast_payload_records(draft);

    draft->fast_signal_count = signal_count;
    draft->fast_primitive_count = module->component_count;
    draft->fast_input_binding_count = module->input_count;
    draft->fast_output_binding_count = module->output_count;

    draft->fast_signals = (StrataPlaceholderFastSignalRecord*)calloc(
        draft->fast_signal_count,
        sizeof(StrataPlaceholderFastSignalRecord));
    draft->fast_primitives = (StrataPlaceholderFastPrimitiveRecord*)calloc(
        draft->fast_primitive_count,
        sizeof(StrataPlaceholderFastPrimitiveRecord));
    draft->fast_input_bindings = (StrataPlaceholderFastInputBinding*)calloc(
        draft->fast_input_binding_count,
        sizeof(StrataPlaceholderFastInputBinding));
    draft->fast_output_bindings = (StrataPlaceholderFastOutputBinding*)calloc(
        draft->fast_output_binding_count,
        sizeof(StrataPlaceholderFastOutputBinding));

    if ((draft->fast_signal_count != 0u && !draft->fast_signals) ||
        (draft->fast_primitive_count != 0u && !draft->fast_primitives) ||
        (draft->fast_input_binding_count != 0u && !draft->fast_input_bindings) ||
        (draft->fast_output_binding_count != 0u && !draft->fast_output_bindings))
    {
        return fail_real_fast_payload(module, draft, "real fast payload: allocation failed");
    }

    for (input_index = 0u; input_index < module->input_count; ++input_index)
    {
        draft->fast_signals[input_index].source_kind =
            STRATA_FAST_SIGNAL_SOURCE_MODULE_INPUT;
        draft->fast_signals[input_index].source_record_index = (uint32_t)input_index;
        draft->fast_signals[input_index].source_output_slot = 0u;
        draft->fast_signals[input_index].reserved = 0u;

        draft->fast_input_bindings[input_index].descriptor_id =
            (uint32_t)module->inputs[input_index].id;
        draft->fast_input_bindings[input_index].signal_index =
            (uint32_t)input_index;
    }

    for (component_index = 0u; component_index < module->component_count; ++component_index)
    {
        size_t output_signal_index = module->input_count + component_index;

        if (!primitive_kind_from_name(
                module->components[component_index].kind_name,
                &primitive_kind))
        {
            return fail_real_fast_payload(module, draft, "real fast payload: unsupported component kind");
        }

        draft->fast_signals[output_signal_index].source_kind =
            STRATA_FAST_SIGNAL_SOURCE_PRIMITIVE_OUTPUT;
        draft->fast_signals[output_signal_index].source_record_index =
            (uint32_t)component_index;
        draft->fast_signals[output_signal_index].source_output_slot = 0u;
        draft->fast_signals[output_signal_index].reserved = 0u;

        draft->fast_primitives[component_index].opcode =
            primitive_opcode_from_kind(primitive_kind);
        draft->fast_primitives[component_index].input_signal_index_a =
            STRATA_PLACEHOLDER_FAST_EXECUTABLE_UNUSED_INDEX;
        draft->fast_primitives[component_index].input_signal_index_b =
            STRATA_PLACEHOLDER_FAST_EXECUTABLE_UNUSED_INDEX;
        draft->fast_primitives[component_index].output_signal_index =
            (uint32_t)output_signal_index;
    }

    for (output_index = 0u; output_index < module->output_count; ++output_index)
    {
        draft->fast_output_bindings[output_index].descriptor_id =
            (uint32_t)module->outputs[output_index].id;
        draft->fast_output_bindings[output_index].signal_index =
            STRATA_PLACEHOLDER_FAST_EXECUTABLE_UNUSED_INDEX;
    }

    for (connection_index = 0u; connection_index < module->executable_connection_count; ++connection_index)
    {
        const BreadboardExecutableConnectionSpec* connection =
            &module->executable_connections[connection_index];
        uint32_t source_signal_index = STRATA_PLACEHOLDER_FAST_EXECUTABLE_UNUSED_INDEX;
        size_t source_component_index = 0u;
        size_t sink_component_index = 0u;
        size_t sink_descriptor_index = 0u;
        uint32_t required_inputs = 0u;

        switch (connection->source.endpoint_class)
        {
            case BREADBOARD_ENDPOINT_MODULE_INPUT_SOURCE:
                if (connection->source.component_id != 0u ||
                    connection->source.slot_index != 0u ||
                    !find_descriptor_index(
                        module->inputs,
                        module->input_count,
                        connection->source.descriptor_id,
                        &source_component_index))
                {
                    return fail_real_fast_payload(module, draft, "real fast payload: invalid module input source");
                }

                source_signal_index = (uint32_t)source_component_index;
                break;

            case BREADBOARD_ENDPOINT_COMPONENT_OUTPUT_SOURCE:
                if (connection->source.descriptor_id != 0u ||
                    connection->source.slot_index != 0u ||
                    !module_find_component_index(
                        module,
                        connection->source.component_id,
                        &source_component_index))
                {
                    return fail_real_fast_payload(module, draft, "real fast payload: invalid component output source");
                }

                source_signal_index =
                    (uint32_t)(module->input_count + source_component_index);
                break;

            default:
                return fail_real_fast_payload(module, draft, "real fast payload: invalid source endpoint class");
        }

        switch (connection->sink.endpoint_class)
        {
            case BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK:
                if (connection->sink.descriptor_id != 0u ||
                    !module_find_component_index(
                        module,
                        connection->sink.component_id,
                        &sink_component_index) ||
                    !primitive_kind_from_name(
                        module->components[sink_component_index].kind_name,
                        &primitive_kind))
                {
                    return fail_real_fast_payload(module, draft, "real fast payload: invalid component input sink");
                }

                required_inputs = primitive_input_count(primitive_kind);
                if (connection->sink.slot_index >= required_inputs ||
                    required_inputs == 0u)
                {
                    return fail_real_fast_payload(module, draft, "real fast payload: invalid primitive input slot");
                }

                if (connection->sink.slot_index == 0u)
                {
                    if (draft->fast_primitives[sink_component_index].input_signal_index_a !=
                        STRATA_PLACEHOLDER_FAST_EXECUTABLE_UNUSED_INDEX)
                    {
                        return fail_real_fast_payload(module, draft, "real fast payload: duplicate primitive input driver");
                    }
                    draft->fast_primitives[sink_component_index].input_signal_index_a =
                        source_signal_index;
                }
                else
                {
                    if (draft->fast_primitives[sink_component_index].input_signal_index_b !=
                        STRATA_PLACEHOLDER_FAST_EXECUTABLE_UNUSED_INDEX)
                    {
                        return fail_real_fast_payload(module, draft, "real fast payload: duplicate primitive input driver");
                    }
                    draft->fast_primitives[sink_component_index].input_signal_index_b =
                        source_signal_index;
                }
                break;

            case BREADBOARD_ENDPOINT_MODULE_OUTPUT_SINK:
                if (connection->sink.component_id != 0u ||
                    connection->sink.slot_index != 0u ||
                    !find_descriptor_index(
                        module->outputs,
                        module->output_count,
                        connection->sink.descriptor_id,
                        &sink_descriptor_index))
                {
                    return fail_real_fast_payload(module, draft, "real fast payload: invalid module output sink");
                }

                if (draft->fast_output_bindings[sink_descriptor_index].signal_index !=
                    STRATA_PLACEHOLDER_FAST_EXECUTABLE_UNUSED_INDEX)
                {
                    return fail_real_fast_payload(module, draft, "real fast payload: duplicate module output driver");
                }

                draft->fast_output_bindings[sink_descriptor_index].signal_index =
                    source_signal_index;
                break;

            default:
                return fail_real_fast_payload(module, draft, "real fast payload: invalid sink endpoint class");
        }
    }

    for (component_index = 0u; component_index < module->component_count; ++component_index)
    {
        if (!primitive_kind_from_name(
                module->components[component_index].kind_name,
                &primitive_kind))
        {
            return fail_real_fast_payload(module, draft, "real fast payload: validation could not resolve component kind");
        }

        required_inputs = primitive_input_count(primitive_kind);
        if (required_inputs == 1u)
        {
            if (draft->fast_primitives[component_index].input_signal_index_a ==
                STRATA_PLACEHOLDER_FAST_EXECUTABLE_UNUSED_INDEX ||
                draft->fast_primitives[component_index].input_signal_index_b !=
                STRATA_PLACEHOLDER_FAST_EXECUTABLE_UNUSED_INDEX)
            {
                return fail_real_fast_payload(module, draft, "real fast payload: unary primitive input mismatch");
            }
        }
        else if (required_inputs == 2u)
        {
            if (draft->fast_primitives[component_index].input_signal_index_a ==
                    STRATA_PLACEHOLDER_FAST_EXECUTABLE_UNUSED_INDEX ||
                draft->fast_primitives[component_index].input_signal_index_b ==
                    STRATA_PLACEHOLDER_FAST_EXECUTABLE_UNUSED_INDEX)
            {
                return fail_real_fast_payload(module, draft, "real fast payload: binary primitive input mismatch");
            }
        }
        else
        {
            return fail_real_fast_payload(module, draft, "real fast payload: unsupported primitive arity");
        }
    }

    for (output_index = 0u; output_index < module->output_count; ++output_index)
    {
        if (draft->fast_output_bindings[output_index].signal_index ==
            STRATA_PLACEHOLDER_FAST_EXECUTABLE_UNUSED_INDEX)
        {
            return fail_real_fast_payload(module, draft, "real fast payload: missing module output driver");
        }
    }

    return BREADBOARD_OK;
}

static BreadboardResult
append_module_descriptor(
    BreadboardDescriptor** io_descriptors,
    size_t* io_count,
    const BreadboardDescriptorSpec* spec,
    BreadboardDescriptorClass descriptor_class)
{
    BreadboardDescriptor* new_descriptors;
    const char* name_copy;
    size_t index;

    if (!io_descriptors || !io_count || !spec || !spec->name ||
        spec->name[0] == '\0' || spec->width == 0u)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    for (index = 0u; index < *io_count; ++index)
    {
        if ((*io_descriptors)[index].id == spec->id ||
            strcmp((*io_descriptors)[index].name, spec->name) == 0)
        {
            return BREADBOARD_ERR_INVALID_ARGUMENT;
        }
    }

    new_descriptors = (BreadboardDescriptor*)realloc(
        *io_descriptors,
        (*io_count + 1u) * sizeof(BreadboardDescriptor));
    if (!new_descriptors)
    {
        return BREADBOARD_ERR_INTERNAL;
    }

    *io_descriptors = new_descriptors;
    name_copy = NULL;
    if (!copy_descriptor_name(spec->name, &name_copy))
    {
        return BREADBOARD_ERR_INTERNAL;
    }

    (*io_descriptors)[*io_count].id = spec->id;
    (*io_descriptors)[*io_count].name = name_copy;
    (*io_descriptors)[*io_count].width = spec->width;
    (*io_descriptors)[*io_count].class_type = descriptor_class;
    (*io_descriptors)[*io_count].is_placeholder = false;
    *io_count += 1u;

    return BREADBOARD_OK;
}

static int
module_has_component_id(
    const BreadboardModule* module,
    uint64_t component_id)
{
    size_t index;

    if (!module)
    {
        return 0;
    }

    for (index = 0u; index < module->component_count; ++index)
    {
        if (module->components[index].id == component_id)
        {
            return 1;
        }
    }

    return 0;
}

static BreadboardResult
append_component_instance(
    BreadboardModule* module,
    const BreadboardComponentSpec* spec)
{
    BreadboardComponent* new_components;
    const char* kind_name_copy;

    if (!module || !spec || !spec->kind_name || spec->kind_name[0] == '\0')
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (module_has_component_id(module, spec->id))
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    new_components = (BreadboardComponent*)realloc(
        module->components,
        (module->component_count + 1u) * sizeof(BreadboardComponent));
    if (!new_components)
    {
        return BREADBOARD_ERR_INTERNAL;
    }

    module->components = new_components;
    kind_name_copy = NULL;
    if (!copy_descriptor_name(spec->kind_name, &kind_name_copy))
    {
        return BREADBOARD_ERR_INTERNAL;
    }

    module->components[module->component_count].id = spec->id;
    module->components[module->component_count].kind_name = kind_name_copy;
    module->components[module->component_count].is_stateful = spec->is_stateful;
    module->component_count += 1u;
    if (spec->is_stateful)
    {
        module->stateful_component_count += 1u;
    }

    return BREADBOARD_OK;
}

static BreadboardResult
append_connection(
    BreadboardModule* module,
    const BreadboardConnectionSpec* spec)
{
    BreadboardConnection* new_connections;

    if (!module || !spec)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (!module_has_component_id(module, spec->source_component_id) ||
        !module_has_component_id(module, spec->sink_component_id))
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    new_connections = (BreadboardConnection*)realloc(
        module->connections,
        (module->connection_count + 1u) * sizeof(BreadboardConnection));
    if (!new_connections)
    {
        return BREADBOARD_ERR_INTERNAL;
    }

    module->connections = new_connections;
    module->connections[module->connection_count].source_component_id =
        spec->source_component_id;
    module->connections[module->connection_count].sink_component_id =
        spec->sink_component_id;
    module->connection_count += 1u;

    return BREADBOARD_OK;
}

static int
primitive_kind_from_name(
    const char* kind_name,
    BreadboardPrimitiveKind* out_kind)
{
    if (!kind_name || !out_kind)
    {
        return 0;
    }

    if (strcmp(kind_name, "BUF") == 0)
    {
        *out_kind = BREADBOARD_PRIMITIVE_BUF;
        return 1;
    }
    if (strcmp(kind_name, "NOT") == 0)
    {
        *out_kind = BREADBOARD_PRIMITIVE_NOT;
        return 1;
    }
    if (strcmp(kind_name, "AND") == 0)
    {
        *out_kind = BREADBOARD_PRIMITIVE_AND;
        return 1;
    }
    if (strcmp(kind_name, "OR") == 0)
    {
        *out_kind = BREADBOARD_PRIMITIVE_OR;
        return 1;
    }
    if (strcmp(kind_name, "XOR") == 0)
    {
        *out_kind = BREADBOARD_PRIMITIVE_XOR;
        return 1;
    }

    *out_kind = BREADBOARD_PRIMITIVE_INVALID;
    return 0;
}

static uint32_t
primitive_input_count(
    BreadboardPrimitiveKind kind)
{
    switch (kind)
    {
        case BREADBOARD_PRIMITIVE_BUF:
        case BREADBOARD_PRIMITIVE_NOT:
            return 1u;
        case BREADBOARD_PRIMITIVE_AND:
        case BREADBOARD_PRIMITIVE_OR:
        case BREADBOARD_PRIMITIVE_XOR:
            return 2u;
        default:
            return 0u;
    }
}

static void
fill_executable_assessment_default(
    BreadboardExecutableAssessment* out_assessment)
{
    if (!out_assessment)
    {
        return;
    }

    memset(out_assessment, 0, sizeof(*out_assessment));
    out_assessment->subset = BREADBOARD_EXECUTABLE_SUBSET_NONE;
    out_assessment->status = BREADBOARD_EXECUTABLE_ASSESSMENT_PLACEHOLDER_ONLY;
    out_assessment->reason = BREADBOARD_EXEC_REASON_NONE;
    out_assessment->failing_connection_index = (size_t)-1;
}

static BreadboardResult
append_executable_connection(
    BreadboardModule* module,
    const BreadboardExecutableConnectionSpec* spec)
{
    BreadboardExecutableConnectionSpec* new_connections;

    if (!module || !spec)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    new_connections = (BreadboardExecutableConnectionSpec*)realloc(
        module->executable_connections,
        (module->executable_connection_count + 1u) *
            sizeof(BreadboardExecutableConnectionSpec));
    if (!new_connections)
    {
        return BREADBOARD_ERR_INTERNAL;
    }

    module->executable_connections = new_connections;
    module->executable_connections[module->executable_connection_count] = *spec;
    module->executable_connection_count += 1u;
    return BREADBOARD_OK;
}

static const char*
executable_assessment_reason_message(
    BreadboardExecutableAssessmentReason reason)
{
    switch (reason)
    {
        case BREADBOARD_EXEC_REASON_TARGET_UNSUPPORTED:
            return "Current target has no admitted real executable subset";
        case BREADBOARD_EXEC_REASON_PROFILE_UNSUPPORTED:
            return "Requirement profile falls outside the admitted executable subset";
        case BREADBOARD_EXEC_REASON_PROBES_UNSUPPORTED:
            return "Probe descriptors are not part of the admitted executable subset";
        case BREADBOARD_EXEC_REASON_DESCRIPTOR_WIDTH_UNSUPPORTED:
            return "Executable subset currently requires single-bit inputs and outputs";
        case BREADBOARD_EXEC_REASON_STATEFUL_COMPONENT_UNSUPPORTED:
            return "Stateful components are not admitted in the first executable subset";
        case BREADBOARD_EXEC_REASON_PRIMITIVE_UNSUPPORTED:
            return "Authored component kind is outside the admitted primitive family";
        case BREADBOARD_EXEC_REASON_EXECUTABLE_CONNECTIONS_REQUIRED:
            return "Real executable assessment requires endpoint-aware executable connections";
        case BREADBOARD_EXEC_REASON_INVALID_ENDPOINT:
            return "Executable connection endpoints are malformed or reference invalid slots";
        case BREADBOARD_EXEC_REASON_DUPLICATE_SINK_DRIVER:
            return "Executable subset requires every sink to have at most one driver";
        case BREADBOARD_EXEC_REASON_MISSING_REQUIRED_DRIVER:
            return "Executable subset requires every primitive input and module output to be driven";
        case BREADBOARD_EXEC_REASON_CYCLE_DETECTED:
            return "Executable subset requires an acyclic combinational topology";
        default:
            return "Executable subset requirement not satisfied";
    }
}

static BreadboardExecutableAssessment
assess_executable_subset(
    const BreadboardModule* module)
{
    BreadboardExecutableAssessment assessment;
    size_t descriptor_index;
    size_t component_index;
    size_t connection_index;
    unsigned char* component_input_seen;
    unsigned char* output_seen;
    unsigned char* adjacency;
    size_t* indegree;
    size_t* queue;
    size_t queue_head;
    size_t queue_tail;
    size_t visited_count;

    fill_executable_assessment_default(&assessment);

    if (!module)
    {
        assessment.status = BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID;
        assessment.reason = BREADBOARD_EXEC_REASON_INVALID_ENDPOINT;
        return assessment;
    }

    if (module->target != BREADBOARD_TARGET_FAST_4STATE)
    {
        assessment.reason = BREADBOARD_EXEC_REASON_TARGET_UNSUPPORTED;
        return assessment;
    }

    assessment.subset = BREADBOARD_EXECUTABLE_SUBSET_FAST_COMBINATIONAL_V1;

    if (module->has_requirement_profile &&
        (module->requirement_profile.extension_flags != 0u ||
         module->requirement_profile.requires_advanced_controls ||
         module->requirement_profile.requires_native_state_read ||
         module->requirement_profile.requires_native_inputs))
    {
        assessment.reason = BREADBOARD_EXEC_REASON_PROFILE_UNSUPPORTED;
        return assessment;
    }

    if (module->probe_count != 0u)
    {
        assessment.reason = BREADBOARD_EXEC_REASON_PROBES_UNSUPPORTED;
        return assessment;
    }

    for (descriptor_index = 0u; descriptor_index < module->input_count; ++descriptor_index)
    {
        if (module->inputs[descriptor_index].width != 1u)
        {
            assessment.reason = BREADBOARD_EXEC_REASON_DESCRIPTOR_WIDTH_UNSUPPORTED;
            return assessment;
        }
    }

    for (descriptor_index = 0u; descriptor_index < module->output_count; ++descriptor_index)
    {
        if (module->outputs[descriptor_index].width != 1u)
        {
            assessment.reason = BREADBOARD_EXEC_REASON_DESCRIPTOR_WIDTH_UNSUPPORTED;
            return assessment;
        }
    }

    for (component_index = 0u; component_index < module->component_count; ++component_index)
    {
        BreadboardPrimitiveKind primitive_kind;

        if (module->components[component_index].is_stateful)
        {
            assessment.reason = BREADBOARD_EXEC_REASON_STATEFUL_COMPONENT_UNSUPPORTED;
            assessment.failing_component_id = module->components[component_index].id;
            return assessment;
        }

        if (!primitive_kind_from_name(
                module->components[component_index].kind_name,
                &primitive_kind))
        {
            assessment.reason = BREADBOARD_EXEC_REASON_PRIMITIVE_UNSUPPORTED;
            assessment.failing_component_id = module->components[component_index].id;
            return assessment;
        }
    }

    if (module->executable_connection_count == 0u)
    {
        assessment.reason = BREADBOARD_EXEC_REASON_EXECUTABLE_CONNECTIONS_REQUIRED;
        return assessment;
    }

    component_input_seen = NULL;
    output_seen = NULL;
    adjacency = NULL;
    indegree = NULL;
    queue = NULL;

    if (module->component_count != 0u)
    {
        component_input_seen = (unsigned char*)calloc(
            module->component_count * 2u,
            sizeof(unsigned char));
        adjacency = (unsigned char*)calloc(
            module->component_count * module->component_count,
            sizeof(unsigned char));
        indegree = (size_t*)calloc(module->component_count, sizeof(size_t));
        queue = (size_t*)calloc(module->component_count, sizeof(size_t));
        if (!component_input_seen || !adjacency || !indegree || !queue)
        {
            free(queue);
            free(indegree);
            free(adjacency);
            free(output_seen);
            free(component_input_seen);
            assessment.status = BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID;
            assessment.reason = BREADBOARD_EXEC_REASON_INVALID_ENDPOINT;
            return assessment;
        }
    }

    if (module->output_count != 0u)
    {
        output_seen = (unsigned char*)calloc(module->output_count, sizeof(unsigned char));
        if (!output_seen)
        {
            free(queue);
            free(indegree);
            free(adjacency);
            free(component_input_seen);
            assessment.status = BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID;
            assessment.reason = BREADBOARD_EXEC_REASON_INVALID_ENDPOINT;
            return assessment;
        }
    }

    for (connection_index = 0u;
         connection_index < module->executable_connection_count;
         ++connection_index)
    {
        const BreadboardExecutableConnectionSpec* connection =
            &module->executable_connections[connection_index];
        size_t source_component_index = 0u;
        size_t sink_component_index = 0u;
        size_t output_index = 0u;
        BreadboardPrimitiveKind primitive_kind = BREADBOARD_PRIMITIVE_INVALID;
        uint32_t input_count = 0u;
        int source_is_component = 0;
        int sink_is_component = 0;

        assessment.failing_connection_index = connection_index;

        switch (connection->source.endpoint_class)
        {
            case BREADBOARD_ENDPOINT_MODULE_INPUT_SOURCE:
                if (connection->source.component_id != 0u ||
                    connection->source.slot_index != 0u ||
                    !find_descriptor_index(
                        module->inputs,
                        module->input_count,
                        connection->source.descriptor_id,
                        &descriptor_index))
                {
                    assessment.status = BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID;
                    assessment.reason = BREADBOARD_EXEC_REASON_INVALID_ENDPOINT;
                    goto cleanup;
                }
                break;

            case BREADBOARD_ENDPOINT_COMPONENT_OUTPUT_SOURCE:
                if (connection->source.descriptor_id != 0u ||
                    connection->source.slot_index != 0u ||
                    !module_find_component_index(
                        module,
                        connection->source.component_id,
                        &source_component_index) ||
                    !primitive_kind_from_name(
                        module->components[source_component_index].kind_name,
                        &primitive_kind))
                {
                    assessment.status = BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID;
                    assessment.reason = BREADBOARD_EXEC_REASON_INVALID_ENDPOINT;
                    goto cleanup;
                }
                source_is_component = 1;
                break;

            default:
                assessment.status = BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID;
                assessment.reason = BREADBOARD_EXEC_REASON_INVALID_ENDPOINT;
                goto cleanup;
        }

        switch (connection->sink.endpoint_class)
        {
            case BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK:
                if (connection->sink.descriptor_id != 0u ||
                    !module_find_component_index(
                        module,
                        connection->sink.component_id,
                        &sink_component_index) ||
                    !primitive_kind_from_name(
                        module->components[sink_component_index].kind_name,
                        &primitive_kind))
                {
                    assessment.status = BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID;
                    assessment.reason = BREADBOARD_EXEC_REASON_INVALID_ENDPOINT;
                    goto cleanup;
                }

                input_count = primitive_input_count(primitive_kind);
                if (input_count == 0u || connection->sink.slot_index >= input_count)
                {
                    assessment.status = BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID;
                    assessment.reason = BREADBOARD_EXEC_REASON_INVALID_ENDPOINT;
                    goto cleanup;
                }

                if (component_input_seen[(sink_component_index * 2u) +
                                         connection->sink.slot_index] != 0u)
                {
                    assessment.status = BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID;
                    assessment.reason = BREADBOARD_EXEC_REASON_DUPLICATE_SINK_DRIVER;
                    assessment.failing_component_id =
                        module->components[sink_component_index].id;
                    goto cleanup;
                }

                component_input_seen[(sink_component_index * 2u) +
                                     connection->sink.slot_index] = 1u;
                sink_is_component = 1;
                break;

            case BREADBOARD_ENDPOINT_MODULE_OUTPUT_SINK:
                if (connection->sink.component_id != 0u ||
                    connection->sink.slot_index != 0u ||
                    !find_descriptor_index(
                        module->outputs,
                        module->output_count,
                        connection->sink.descriptor_id,
                        &output_index))
                {
                    assessment.status = BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID;
                    assessment.reason = BREADBOARD_EXEC_REASON_INVALID_ENDPOINT;
                    goto cleanup;
                }

                if (output_seen[output_index] != 0u)
                {
                    assessment.status = BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID;
                    assessment.reason = BREADBOARD_EXEC_REASON_DUPLICATE_SINK_DRIVER;
                    goto cleanup;
                }

                output_seen[output_index] = 1u;
                break;

            default:
                assessment.status = BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID;
                assessment.reason = BREADBOARD_EXEC_REASON_INVALID_ENDPOINT;
                goto cleanup;
        }

        if (source_is_component && sink_is_component)
        {
            if (source_component_index == sink_component_index)
            {
                assessment.status = BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID;
                assessment.reason = BREADBOARD_EXEC_REASON_CYCLE_DETECTED;
                assessment.failing_component_id =
                    module->components[source_component_index].id;
                goto cleanup;
            }

            adjacency[(source_component_index * module->component_count) +
                      sink_component_index] = 1u;
        }
    }

    for (component_index = 0u; component_index < module->component_count; ++component_index)
    {
        BreadboardPrimitiveKind primitive_kind;
        uint32_t input_count;
        uint32_t slot_index;

        primitive_kind_from_name(module->components[component_index].kind_name, &primitive_kind);
        input_count = primitive_input_count(primitive_kind);
        for (slot_index = 0u; slot_index < input_count; ++slot_index)
        {
            if (component_input_seen[(component_index * 2u) + slot_index] == 0u)
            {
                assessment.status = BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID;
                assessment.reason = BREADBOARD_EXEC_REASON_MISSING_REQUIRED_DRIVER;
                assessment.failing_component_id = module->components[component_index].id;
                goto cleanup;
            }
        }
    }

    for (descriptor_index = 0u; descriptor_index < module->output_count; ++descriptor_index)
    {
        if (output_seen[descriptor_index] == 0u)
        {
            assessment.status = BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID;
            assessment.reason = BREADBOARD_EXEC_REASON_MISSING_REQUIRED_DRIVER;
            goto cleanup;
        }
    }

    for (component_index = 0u; component_index < module->component_count; ++component_index)
    {
        size_t sink_index;
        for (sink_index = 0u; sink_index < module->component_count; ++sink_index)
        {
            if (adjacency[(component_index * module->component_count) + sink_index] != 0u)
            {
                indegree[sink_index] += 1u;
            }
        }
    }

    queue_head = 0u;
    queue_tail = 0u;
    for (component_index = 0u; component_index < module->component_count; ++component_index)
    {
        if (indegree[component_index] == 0u)
        {
            queue[queue_tail++] = component_index;
        }
    }

    visited_count = 0u;
    while (queue_head < queue_tail)
    {
        size_t current_index = queue[queue_head++];
        size_t sink_index;

        visited_count += 1u;
        for (sink_index = 0u; sink_index < module->component_count; ++sink_index)
        {
            if (adjacency[(current_index * module->component_count) + sink_index] == 0u)
            {
                continue;
            }

            indegree[sink_index] -= 1u;
            if (indegree[sink_index] == 0u)
            {
                queue[queue_tail++] = sink_index;
            }
        }
    }

    if (visited_count != module->component_count)
    {
        assessment.status = BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID;
        assessment.reason = BREADBOARD_EXEC_REASON_CYCLE_DETECTED;
        goto cleanup;
    }

    assessment.status = BREADBOARD_EXECUTABLE_ASSESSMENT_EXECUTABLE;
    assessment.reason = BREADBOARD_EXEC_REASON_NONE;

cleanup:
    free(queue);
    free(indegree);
    free(adjacency);
    free(output_seen);
    free(component_input_seen);
    return assessment;
}

static void
serialize_breadboard_descriptors(
    StrataPlaceholderSerializedDescriptor* out_descriptors,
    const BreadboardDescriptor* descriptors,
    size_t count)
{
    size_t index;
    size_t copy_len;

    if (!out_descriptors || !descriptors)
    {
        return;
    }

    for (index = 0u; index < count; ++index)
    {
        out_descriptors[index].id = (uint32_t)descriptors[index].id;
        out_descriptors[index].width = descriptors[index].width;
        out_descriptors[index].class_type = (uint32_t)descriptors[index].class_type;
        out_descriptors[index].placeholder_flags =
            descriptors[index].is_placeholder ? 1u : 0u;

        memset(out_descriptors[index].name, 0, sizeof(out_descriptors[index].name));

        if (!descriptors[index].name)
        {
            continue;
        }

        copy_len = strlen(descriptors[index].name);
        if (copy_len >= sizeof(out_descriptors[index].name))
        {
            copy_len = sizeof(out_descriptors[index].name) - 1u;
        }

        memcpy(out_descriptors[index].name, descriptors[index].name, copy_len);
    }
}

static void
serialize_breadboard_components(
    StrataPlaceholderSerializedComponent* out_components,
    const BreadboardComponent* components,
    size_t count)
{
    size_t index;
    size_t copy_len;

    if (!out_components || !components)
    {
        return;
    }

    for (index = 0u; index < count; ++index)
    {
        out_components[index].id = components[index].id;
        out_components[index].stateful_flags =
            components[index].is_stateful ? 1u : 0u;
        memset(out_components[index].kind_name, 0, sizeof(out_components[index].kind_name));

        if (!components[index].kind_name)
        {
            continue;
        }

        copy_len = strlen(components[index].kind_name);
        if (copy_len >= sizeof(out_components[index].kind_name))
        {
            copy_len = sizeof(out_components[index].kind_name) - 1u;
        }

        memcpy(out_components[index].kind_name, components[index].kind_name, copy_len);
    }
}

static void
serialize_breadboard_connections(
    StrataPlaceholderSerializedConnection* out_connections,
    const BreadboardConnection* connections,
    size_t count)
{
    size_t index;

    if (!out_connections || !connections)
    {
        return;
    }

    for (index = 0u; index < count; ++index)
    {
        out_connections[index].source_component_id =
            connections[index].source_component_id;
        out_connections[index].sink_component_id =
            connections[index].sink_component_id;
    }
}

static void
fill_empty_executable_subset_info(
    BreadboardTarget target,
    BreadboardExecutableSubsetInfo* out_info)
{
    if (!out_info)
    {
        return;
    }

    memset(out_info, 0, sizeof(*out_info));
    out_info->subset = BREADBOARD_EXECUTABLE_SUBSET_NONE;
    out_info->target = target;
}

static void
fill_fast_combinational_subset_info(
    BreadboardExecutableSubsetInfo* out_info)
{
    if (!out_info)
    {
        return;
    }

    memset(out_info, 0, sizeof(*out_info));
    out_info->subset = BREADBOARD_EXECUTABLE_SUBSET_FAST_COMBINATIONAL_V1;
    out_info->target = BREADBOARD_TARGET_FAST_4STATE;
    out_info->has_real_executable_subset = true;
    out_info->flat_only = true;
    out_info->single_bit_only = true;
    out_info->combinational_only = true;
    out_info->allows_stateful_components = false;
    out_info->allows_cycles = false;
    out_info->real_path_hard_rejects_out_of_subset = true;
    out_info->placeholder_fallback_requires_explicit_allowance = true;
    out_info->admitted_primitive_mask = BREADBOARD_PRIMITIVE_MASK_ALL_FIRST_EXECUTABLE;
    out_info->module_input_endpoint_class =
        BREADBOARD_ENDPOINT_MODULE_INPUT_SOURCE;
    out_info->component_output_endpoint_class =
        BREADBOARD_ENDPOINT_COMPONENT_OUTPUT_SOURCE;
    out_info->component_input_endpoint_class =
        BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK;
    out_info->module_output_endpoint_class =
        BREADBOARD_ENDPOINT_MODULE_OUTPUT_SINK;
}

static int
fill_primitive_signature(
    BreadboardTarget target,
    BreadboardPrimitiveKind primitive_kind,
    BreadboardPrimitiveSignature* out_signature)
{
    if (!out_signature || target != BREADBOARD_TARGET_FAST_4STATE)
    {
        return 0;
    }

    memset(out_signature, 0, sizeof(*out_signature));
    out_signature->primitive_kind = primitive_kind;
    out_signature->target = target;
    out_signature->output_count = 1u;
    out_signature->output_name_0 = "out";
    out_signature->single_bit_only = true;
    out_signature->is_stateful = false;

    switch (primitive_kind)
    {
        case BREADBOARD_PRIMITIVE_BUF:
        case BREADBOARD_PRIMITIVE_NOT:
            out_signature->input_count = 1u;
            out_signature->input_name_0 = "in";
            return 1;
        case BREADBOARD_PRIMITIVE_AND:
        case BREADBOARD_PRIMITIVE_OR:
        case BREADBOARD_PRIMITIVE_XOR:
            out_signature->input_count = 2u;
            out_signature->input_name_0 = "a";
            out_signature->input_name_1 = "b";
            return 1;
        default:
            return 0;
    }
}

/*
 * breadboard_api.c
 *
 * Minimal scaffolding implementation of the Breadboard compiler boundary.
 * These are stubs providing structural handles and returning placeholder
 * errors to prove the build and API mapping without providing real logic.
 */

BreadboardResult breadboard_module_create(BreadboardModule** out_module)
{
    if (!out_module)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    BreadboardModule* mod = (BreadboardModule*)calloc(1, sizeof(BreadboardModule));
    if (!mod)
    {
        return BREADBOARD_ERR_INTERNAL;
    }

    mod->target = BREADBOARD_TARGET_UNSPECIFIED;
    mod->allowed_targets = BREADBOARD_TARGET_MASK_ALL;
    mod->diagnostic_count = 0;
    mod->diagnostics = NULL;

    *out_module = mod;
    return BREADBOARD_OK;
}

void breadboard_module_free(BreadboardModule* module)
{
    if (module)
    {
        free_component_array(module->components, module->component_count);
        free(module->connections);
        free(module->executable_connections);
        free_descriptor_array(module->inputs, module->input_count);
        free_descriptor_array(module->outputs, module->output_count);
        free_descriptor_array(module->probes, module->probe_count);
        if (module->diagnostics)
        {
            free(module->diagnostics);
        }
        /* Future: free structural lists */
        free(module);
    }
}

BreadboardResult breadboard_module_set_target(
    BreadboardModule* module,
    BreadboardTarget target)
{
    if (!module)
    {
        return BREADBOARD_ERR_INVALID_HANDLE;
    }

    /* Minimal validation of known enum values */
    if (target != BREADBOARD_TARGET_UNSPECIFIED &&
        target != BREADBOARD_TARGET_FAST_4STATE &&
        target != BREADBOARD_TARGET_TEMPORAL)
    {
        return BREADBOARD_ERR_INVALID_TARGET;
    }

    module->target = target;
    module->has_projection_policy = false;
    return BREADBOARD_OK;
}

BreadboardResult breadboard_module_set_target_policy(
    BreadboardModule* module,
    BreadboardTargetMask allowed_mask)
{
    if (!module)
    {
        return BREADBOARD_ERR_INVALID_HANDLE;
    }

    if ((allowed_mask & ~BREADBOARD_TARGET_MASK_ALL) != 0u)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    module->allowed_targets = allowed_mask;
    return BREADBOARD_OK;
}

BreadboardResult breadboard_module_query_target_availability(
    const BreadboardModule* module,
    BreadboardTarget target,
    bool* out_is_available)
{
    if (!module)
    {
        return BREADBOARD_ERR_INVALID_HANDLE;
    }

    if (!out_is_available)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (target == BREADBOARD_TARGET_UNSPECIFIED)
    {
        *out_is_available = false;
        return BREADBOARD_OK;
    }

    if (target != BREADBOARD_TARGET_FAST_4STATE &&
        target != BREADBOARD_TARGET_TEMPORAL)
    {
        return BREADBOARD_ERR_INVALID_TARGET;
    }

    BreadboardTargetMask target_bit = (1u << target);
    *out_is_available = ((module->allowed_targets & target_bit) != 0);
    return BREADBOARD_OK;
}

BreadboardResult breadboard_module_add_input_descriptor(
    BreadboardModule* module,
    const BreadboardDescriptorSpec* spec)
{
    if (!module)
    {
        return BREADBOARD_ERR_INVALID_HANDLE;
    }

    return append_module_descriptor(
        &module->inputs,
        &module->input_count,
        spec,
        BREADBOARD_DESC_INPUT);
}

BreadboardResult breadboard_module_add_output_descriptor(
    BreadboardModule* module,
    const BreadboardDescriptorSpec* spec)
{
    if (!module)
    {
        return BREADBOARD_ERR_INVALID_HANDLE;
    }

    return append_module_descriptor(
        &module->outputs,
        &module->output_count,
        spec,
        BREADBOARD_DESC_OUTPUT);
}

BreadboardResult breadboard_module_add_probe_descriptor(
    BreadboardModule* module,
    const BreadboardDescriptorSpec* spec)
{
    if (!module)
    {
        return BREADBOARD_ERR_INVALID_HANDLE;
    }

    return append_module_descriptor(
        &module->probes,
        &module->probe_count,
        spec,
        BREADBOARD_DESC_PROBE);
}

BreadboardResult breadboard_module_set_identity(
    BreadboardModule* module,
    const BreadboardModuleIdentity* identity)
{
    if (!module)
    {
        return BREADBOARD_ERR_INVALID_HANDLE;
    }

    if (!identity)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    return copy_module_identity(
        identity->module_id,
        identity->module_name,
        &module->module_id,
        module->module_name);
}

BreadboardResult breadboard_module_set_requirement_profile(
    BreadboardModule* module,
    const BreadboardRequirementProfile* profile)
{
    BreadboardResult result;

    if (!module)
    {
        return BREADBOARD_ERR_INVALID_HANDLE;
    }

    result = validate_requirement_profile(module->target, profile);
    if (result != BREADBOARD_OK)
    {
        return result;
    }

    module->requirement_profile = *profile;
    module->has_requirement_profile = true;
    return BREADBOARD_OK;
}

BreadboardResult breadboard_module_set_structure_summary(
    BreadboardModule* module,
    const BreadboardStructureSummary* summary)
{
    if (!module)
    {
        return BREADBOARD_ERR_INVALID_HANDLE;
    }

    if (!summary)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    module->structure_summary = *summary;
    module->has_structure_summary = true;
    return BREADBOARD_OK;
}

BreadboardResult breadboard_module_set_projection_policy(
    BreadboardModule* module,
    const BreadboardProjectionPolicy* policy)
{
    if (!module)
    {
        return BREADBOARD_ERR_INVALID_HANDLE;
    }

    if (!policy)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    module->projection_policy = *policy;
    module->has_projection_policy = true;
    return BREADBOARD_OK;
}

BreadboardResult breadboard_module_get_projection_policy(
    const BreadboardModule* module,
    BreadboardProjectionPolicy* out_policy)
{
    if (!module)
    {
        return BREADBOARD_ERR_INVALID_HANDLE;
    }

    if (!out_policy)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (module->has_projection_policy)
    {
        *out_policy = module->projection_policy;
    }
    else
    {
        fill_default_projection_policy(module->target, out_policy);
    }
    return BREADBOARD_OK;
}

BreadboardResult breadboard_module_add_component_instance(
    BreadboardModule* module,
    const BreadboardComponentSpec* spec)
{
    if (!module)
    {
        return BREADBOARD_ERR_INVALID_HANDLE;
    }

    return append_component_instance(module, spec);
}

BreadboardResult breadboard_module_add_connection(
    BreadboardModule* module,
    const BreadboardConnectionSpec* spec)
{
    if (!module)
    {
        return BREADBOARD_ERR_INVALID_HANDLE;
    }

    return append_connection(module, spec);
}

BreadboardResult breadboard_module_add_executable_connection(
    BreadboardModule* module,
    const BreadboardExecutableConnectionSpec* spec)
{
    if (!module)
    {
        return BREADBOARD_ERR_INVALID_HANDLE;
    }

    return append_executable_connection(module, spec);
}

/* Helper to record a diagnostic */
static BreadboardResult record_diagnostic(
    BreadboardModule* module,
    BreadboardDiagnosticSeverity severity,
    BreadboardDiagnosticCode code,
    const char* message)
{
    if (!module) return BREADBOARD_ERR_INTERNAL;

    if (module->diagnostic_count >= module->diagnostic_capacity)
    {
        size_t new_cap = module->diagnostic_capacity == 0 ? 4 : module->diagnostic_capacity * 2;
        BreadboardDiagnostic* new_diags = (BreadboardDiagnostic*)realloc(
            module->diagnostics,
            new_cap * sizeof(BreadboardDiagnostic));
        
        if (!new_diags) return BREADBOARD_ERR_INTERNAL;

        module->diagnostics = new_diags;
        module->diagnostic_capacity = new_cap;
    }

    BreadboardDiagnostic* diag = &module->diagnostics[module->diagnostic_count++];
    diag->severity = severity;
    diag->code     = code;
    diag->message  = message;

    return BREADBOARD_OK;
}

BreadboardResult breadboard_module_compile(
    BreadboardModule* module,
    const BreadboardCompileOptions* options,
    BreadboardArtifactDraft** out_draft)
{
    BreadboardExecutableAssessment executable_assessment;

    if (!module || !out_draft)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    *out_draft = NULL;

    if (module->target == BREADBOARD_TARGET_UNSPECIFIED)
    {
        record_diagnostic(module, BREADBOARD_DIAG_ERROR, BREADBOARD_DIAG_CODE_UNSUPPORTED_TARGET, "Cannot compile without a specified target");
        return BREADBOARD_ERR_COMPILE_FAILED;
    }

    if (module->has_requirement_profile &&
        validate_requirement_profile(
            module->target,
            &module->requirement_profile) != BREADBOARD_OK)
    {
        record_diagnostic(module, BREADBOARD_DIAG_ERROR, BREADBOARD_DIAG_CODE_UNSUPPORTED_CONSTRUCT, "Declared requirement profile is incompatible with the current target");
        return BREADBOARD_ERR_COMPILE_FAILED;
    }

    BreadboardTargetMask target_bit = (1u << module->target);
    if ((module->allowed_targets & target_bit) == 0)
    {
        record_diagnostic(module, BREADBOARD_DIAG_ERROR, BREADBOARD_DIAG_CODE_TARGET_DENIED_BY_POLICY, "Target denied by product/compile policy");
        return BREADBOARD_ERR_COMPILE_FAILED;
    }

    if (options &&
        options->require_real_executable &&
        options->allow_placeholders)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    executable_assessment = assess_executable_subset(module);
    if (executable_assessment.status == BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID)
    {
        record_diagnostic(
            module,
            BREADBOARD_DIAG_ERROR,
            BREADBOARD_DIAG_CODE_EXECUTABLE_SUBSET_INVALID,
            executable_assessment_reason_message(executable_assessment.reason));
        return BREADBOARD_ERR_COMPILE_FAILED;
    }

    if (options && options->require_real_executable)
    {
        if (executable_assessment.status !=
            BREADBOARD_EXECUTABLE_ASSESSMENT_EXECUTABLE)
        {
            record_diagnostic(
                module,
                BREADBOARD_DIAG_ERROR,
                BREADBOARD_DIAG_CODE_EXECUTABLE_SUBSET_REQUIRED,
                executable_assessment_reason_message(executable_assessment.reason));
            return BREADBOARD_ERR_COMPILE_FAILED;
        }
    }

    /* We are a skeleton unless the caller explicitly asks for the real fast path. */
    if ((!options || !options->allow_placeholders) &&
        !(options && options->require_real_executable))
    {
        record_diagnostic(module, BREADBOARD_DIAG_ERROR, BREADBOARD_DIAG_CODE_UNSUPPORTED_CONSTRUCT, "Compilation requires allow_placeholders in the current skeleton limit");
        return BREADBOARD_ERR_COMPILE_FAILED;
    }

    /* If strict projection is demanded, emit a diagnostic noting that structural analysis is incomplete. */
    if (options &&
        (options->strict_projection || options->deny_approximation))
    {
        record_diagnostic(module, BREADBOARD_DIAG_ERROR, BREADBOARD_DIAG_CODE_UNSUPPORTED_CONSTRUCT, "Approximation denial and strict projection are not yet supported without real structural analysis");
        return BREADBOARD_ERR_COMPILE_FAILED;
    }

    /* If placeholders are allowed, return an empty dummy draft to establish plumbing. */
    BreadboardArtifactDraft* draft = (BreadboardArtifactDraft*)calloc(1, sizeof(BreadboardArtifactDraft));
    if (!draft)
    {
        record_diagnostic(module, BREADBOARD_DIAG_ERROR, BREADBOARD_DIAG_CODE_INTERNAL_ERROR, "Failed to allocate artifact draft");
        return BREADBOARD_ERR_INTERNAL;
    }

    if (module->input_count != 0u || module->output_count != 0u || module->probe_count != 0u)
    {
        draft->input_count = module->input_count;
        if (!copy_declared_descriptors(&draft->inputs, module->inputs, draft->input_count))
        {
            breadboard_artifact_draft_free(draft);
            record_diagnostic(module, BREADBOARD_DIAG_ERROR, BREADBOARD_DIAG_CODE_INTERNAL_ERROR, "Failed to copy declared input descriptors");
            return BREADBOARD_ERR_INTERNAL;
        }

        draft->output_count = module->output_count;
        if (!copy_declared_descriptors(&draft->outputs, module->outputs, draft->output_count))
        {
            breadboard_artifact_draft_free(draft);
            record_diagnostic(module, BREADBOARD_DIAG_ERROR, BREADBOARD_DIAG_CODE_INTERNAL_ERROR, "Failed to copy declared output descriptors");
            return BREADBOARD_ERR_INTERNAL;
        }

        draft->probe_count = module->probe_count;
        if (!copy_declared_descriptors(&draft->probes, module->probes, draft->probe_count))
        {
            breadboard_artifact_draft_free(draft);
            record_diagnostic(module, BREADBOARD_DIAG_ERROR, BREADBOARD_DIAG_CODE_INTERNAL_ERROR, "Failed to copy declared probe descriptors");
            return BREADBOARD_ERR_INTERNAL;
        }
    }
    else
    {
        /* Placeholder fallback path used until real structure ingestion is richer. */
        draft->input_count = strata_placeholder_input_descriptor_count();
        draft->inputs = (BreadboardDescriptor*)calloc(
            draft->input_count,
            sizeof(BreadboardDescriptor));
        if (draft->inputs)
        {
            copy_placeholder_descriptors(
                draft->inputs,
                k_strata_placeholder_input_descriptors,
                draft->input_count);
        }

        draft->output_count = strata_placeholder_output_descriptor_count();
        draft->outputs = (BreadboardDescriptor*)calloc(
            draft->output_count,
            sizeof(BreadboardDescriptor));
        if (draft->outputs)
        {
            copy_placeholder_descriptors(
                draft->outputs,
                k_strata_placeholder_output_descriptors,
                draft->output_count);
        }

        draft->probe_count = strata_placeholder_probe_descriptor_count();
        draft->probes = (BreadboardDescriptor*)calloc(
            draft->probe_count,
            sizeof(BreadboardDescriptor));
        if (draft->probes)
        {
            copy_placeholder_descriptors(
                draft->probes,
                k_strata_placeholder_probe_descriptors,
                draft->probe_count);
        }
    }

    /* Copy target expectation and admission logic to the draft */
    draft->target = module->target;
    if (copy_module_identity(
            module->module_id,
            module->module_name,
            &draft->source_module_id,
            draft->source_module_name) != BREADBOARD_OK)
    {
        breadboard_artifact_draft_free(draft);
        record_diagnostic(module, BREADBOARD_DIAG_ERROR, BREADBOARD_DIAG_CODE_INTERNAL_ERROR, "Failed to copy module identity into draft");
        return BREADBOARD_ERR_INTERNAL;
    }

    if (module->component_count != 0u || module->connection_count != 0u)
    {
        draft->component_count = module->component_count;
        if (!copy_component_array(&draft->components, module->components, draft->component_count))
        {
            breadboard_artifact_draft_free(draft);
            record_diagnostic(module, BREADBOARD_DIAG_ERROR, BREADBOARD_DIAG_CODE_INTERNAL_ERROR, "Failed to copy declared components");
            return BREADBOARD_ERR_INTERNAL;
        }

        draft->connection_count = module->connection_count;
        if (!copy_connection_array(&draft->connections, module->connections, draft->connection_count))
        {
            breadboard_artifact_draft_free(draft);
            record_diagnostic(module, BREADBOARD_DIAG_ERROR, BREADBOARD_DIAG_CODE_INTERNAL_ERROR, "Failed to copy declared connections");
            return BREADBOARD_ERR_INTERNAL;
        }

        draft->structure_summary.declared_component_count =
            (uint32_t)module->component_count;
        draft->structure_summary.declared_connection_count =
            (uint32_t)module->connection_count;
        draft->structure_summary.declared_stateful_node_count =
            (uint32_t)module->stateful_component_count;
    }
    else if (module->has_structure_summary)
    {
        draft->structure_summary = module->structure_summary;
    }
    else
    {
        fill_default_structure_summary(&draft->structure_summary);
    }

    draft->info.target = module->target;
    draft->info.source_module_id = draft->source_module_id;
    draft->info.source_module_name = draft->source_module_name;
    draft->info.declared_component_count =
        draft->structure_summary.declared_component_count;
    draft->info.declared_connection_count =
        draft->structure_summary.declared_connection_count;
    draft->info.declared_stateful_node_count =
        draft->structure_summary.declared_stateful_node_count;
    draft->admission_info.target = module->target;

    if (options && options->require_real_executable)
    {
        if (build_real_fast_payload(module, draft) != BREADBOARD_OK)
        {
            breadboard_artifact_draft_free(draft);
            record_diagnostic(
                module,
                BREADBOARD_DIAG_ERROR,
                BREADBOARD_DIAG_CODE_EXECUTABLE_LOWERING_UNAVAILABLE,
                "Real executable fast-path lowering failed");
            return BREADBOARD_ERR_COMPILE_FAILED;
        }

        draft->connection_count = 0u;
        if (!copy_executable_structure_connection_array(
                &draft->connections,
                &draft->connection_count,
                module->executable_connections,
                module->executable_connection_count))
        {
            breadboard_artifact_draft_free(draft);
            record_diagnostic(
                module,
                BREADBOARD_DIAG_ERROR,
                BREADBOARD_DIAG_CODE_INTERNAL_ERROR,
                "Failed to copy real executable connections");
            return BREADBOARD_ERR_INTERNAL;
        }

        draft->structure_summary.declared_connection_count =
            (uint32_t)draft->connection_count;
        draft->info.declared_connection_count =
            draft->structure_summary.declared_connection_count;

        draft->info.has_placeholders = false;
        draft->info.approximate_size_bytes = real_fast_artifact_size_for_layout(
            (uint32_t)draft->input_count,
            (uint32_t)draft->output_count,
            (uint32_t)draft->probe_count,
            draft->fast_primitive_count,
            draft->fast_signal_count,
            draft->fast_input_binding_count,
            draft->fast_output_binding_count,
            (uint32_t)draft->component_count,
            (uint32_t)draft->connection_count);
        draft->admission_info.is_placeholder = false;
        draft->admission_info.approximate_size_bytes = draft->info.approximate_size_bytes;
        draft->admission_info.extension_flags = 0u;
        draft->admission_info.requires_advanced_controls = false;
        draft->admission_info.requires_native_state_read = false;
        draft->admission_info.requires_native_inputs = false;
        draft->admission_info.native_only_behavior = false;
    }
    else
    {
        draft->info.has_placeholders =
            (module->input_count == 0u &&
             module->output_count == 0u &&
             module->probe_count == 0u);
        draft->info.approximate_size_bytes = strata_placeholder_artifact_size_for_layout(
            (uint32_t)draft->input_count,
            (uint32_t)draft->output_count,
            (uint32_t)draft->probe_count,
            (uint32_t)draft->component_count,
            (uint32_t)draft->connection_count);
        draft->admission_info.is_placeholder = true;
        draft->admission_info.approximate_size_bytes = draft->info.approximate_size_bytes;

        if (module->has_requirement_profile)
        {
            draft->admission_info.extension_flags = module->requirement_profile.extension_flags;
            draft->admission_info.requires_advanced_controls =
                module->requirement_profile.requires_advanced_controls;
            draft->admission_info.requires_native_state_read =
                module->requirement_profile.requires_native_state_read;
            draft->admission_info.requires_native_inputs =
                module->requirement_profile.requires_native_inputs;
        }
        else
        {
            BreadboardRequirementProfile default_profile;
            fill_default_requirement_profile(module->target, &default_profile);
            draft->admission_info.extension_flags = default_profile.extension_flags;
            draft->admission_info.requires_advanced_controls =
                default_profile.requires_advanced_controls;
            draft->admission_info.requires_native_state_read =
                default_profile.requires_native_state_read;
            draft->admission_info.requires_native_inputs =
                default_profile.requires_native_inputs;
        }
        draft->admission_info.native_only_behavior =
            (draft->admission_info.requires_native_state_read ||
             draft->admission_info.requires_native_inputs);
    }

    /* Record a warning that placeholders were emitted */
    if (!options || !options->require_real_executable)
    {
        record_diagnostic(module, BREADBOARD_DIAG_WARNING, BREADBOARD_DIAG_CODE_NONE, "Draft emitted with placeholder structures");
    }

    *out_draft = draft;
    return BREADBOARD_OK;
}

BreadboardResult breadboard_module_get_target(
    const BreadboardModule* module,
    BreadboardTarget* out_target)
{
    if (!module || !out_target)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    *out_target = module->target;
    return BREADBOARD_OK;
}

void breadboard_artifact_draft_free(BreadboardArtifactDraft* draft)
{
    if (draft)
    {
        clear_fast_payload_records(draft);
        free_component_array(draft->components, draft->component_count);
        free(draft->connections);
        free_descriptor_array(draft->inputs, draft->input_count);
        free_descriptor_array(draft->outputs, draft->output_count);
        free_descriptor_array(draft->probes, draft->probe_count);
        free(draft);
    }
}

BreadboardResult breadboard_module_get_diagnostic_count(
    const BreadboardModule* module,
    size_t* out_count)
{
    if (!module || !out_count)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    *out_count = module->diagnostic_count;
    return BREADBOARD_OK;
}

BreadboardResult breadboard_module_get_diagnostic(
    const BreadboardModule* module,
    size_t index,
    BreadboardDiagnostic* out_diagnostic)
{
    if (!module || !out_diagnostic)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (index >= module->diagnostic_count || !module->diagnostics)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    *out_diagnostic = module->diagnostics[index];
    return BREADBOARD_OK;
}

BreadboardResult breadboard_module_get_last_diagnostic(
    const BreadboardModule* module,
    BreadboardDiagnostic* out_diagnostic)
{
    if (!module || !out_diagnostic)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (module->diagnostic_count == 0 || !module->diagnostics)
    {
        return BREADBOARD_ERR_UNSUPPORTED;
    }

    *out_diagnostic = module->diagnostics[module->diagnostic_count - 1];
    return BREADBOARD_OK;
}

BreadboardResult breadboard_artifact_draft_query_metadata(
    const BreadboardArtifactDraft* draft,
    BreadboardTarget* out_target)
{
    if (!draft || !out_target)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    *out_target = draft->target;
    return BREADBOARD_OK;
}

BreadboardResult breadboard_artifact_draft_query_info(
    const BreadboardArtifactDraft* draft,
    BreadboardDraftInfo* out_info)
{
    if (!draft || !out_info)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    *out_info = draft->info;
    return BREADBOARD_OK;
}

BreadboardResult breadboard_draft_query_admission_info(
    const BreadboardArtifactDraft* draft,
    BreadboardDraftAdmissionInfo* out_info)
{
    if (!draft || !out_info)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    *out_info = draft->admission_info;
    return BREADBOARD_OK;
}

BreadboardResult breadboard_artifact_draft_export_placeholder_size(
    const BreadboardArtifactDraft* draft,
    size_t* out_size)
{
    if (!draft || !out_size)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (!draft->admission_info.is_placeholder)
    {
        return BREADBOARD_ERR_UNSUPPORTED;
    }

    *out_size = strata_placeholder_artifact_size_for_layout(
        (uint32_t)draft->input_count,
        (uint32_t)draft->output_count,
        (uint32_t)draft->probe_count,
        (uint32_t)draft->component_count,
        (uint32_t)draft->connection_count);
    return BREADBOARD_OK;
}

BreadboardResult breadboard_artifact_draft_export_placeholder(
    const BreadboardArtifactDraft* draft,
    void* buffer,
    size_t buffer_size,
    size_t* out_size)
{
    size_t required_size;
    uint32_t target_backend_id;
    StrataPlaceholderPayloadKind payload_kind;
    StrataPlaceholderAdmissionInfo admission_info;
    StrataPlaceholderDraftSummary draft_summary;
    size_t total_descriptor_count;
    StrataPlaceholderSerializedDescriptor* serialized_descriptors;
    StrataPlaceholderSerializedComponent* serialized_components;
    StrataPlaceholderSerializedConnection* serialized_connections;

    if (!draft || !buffer || !out_size)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (!draft->admission_info.is_placeholder)
    {
        return BREADBOARD_ERR_UNSUPPORTED;
    }

    required_size = strata_placeholder_artifact_size_for_layout(
        (uint32_t)draft->input_count,
        (uint32_t)draft->output_count,
        (uint32_t)draft->probe_count,
        (uint32_t)draft->component_count,
        (uint32_t)draft->connection_count);

    if (buffer_size < required_size)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (resolve_placeholder_backend_id(draft->target, &target_backend_id) != BREADBOARD_OK)
    {
        return BREADBOARD_ERR_UNSUPPORTED;
    }

    if (draft->admission_info.requires_native_state_read ||
        draft->admission_info.requires_native_inputs)
    {
        payload_kind = STRATA_PLACEHOLDER_PAYLOAD_NATIVE;
    }
    else if (draft->admission_info.requires_advanced_controls)
    {
        payload_kind = STRATA_PLACEHOLDER_PAYLOAD_ADVANCED;
    }
    else
    {
        payload_kind = STRATA_PLACEHOLDER_PAYLOAD_BASELINE;
    }

    if (!fill_placeholder_admission_info(draft, payload_kind, &admission_info))
    {
        return BREADBOARD_ERR_INTERNAL;
    }

    draft_summary.source_target_value = (uint32_t)draft->info.target;
    draft_summary.has_placeholders = draft->info.has_placeholders ? 1u : 0u;
    draft_summary.approximate_size_bytes =
        (uint64_t)draft->info.approximate_size_bytes;
    draft_summary.source_module_id = draft->source_module_id;
    memset(draft_summary.source_module_name, 0, sizeof(draft_summary.source_module_name));
    memcpy(
        draft_summary.source_module_name,
        draft->source_module_name,
        sizeof(draft_summary.source_module_name) - 1u);
    draft_summary.declared_component_count =
        draft->structure_summary.declared_component_count;
    draft_summary.declared_connection_count =
        draft->structure_summary.declared_connection_count;
    draft_summary.declared_stateful_node_count =
        draft->structure_summary.declared_stateful_node_count;

    total_descriptor_count = draft->input_count +
        draft->output_count +
        draft->probe_count;
    serialized_descriptors = (StrataPlaceholderSerializedDescriptor*)calloc(
        total_descriptor_count,
        sizeof(StrataPlaceholderSerializedDescriptor));
    if (!serialized_descriptors)
    {
        return BREADBOARD_ERR_INTERNAL;
    }

    serialized_components = NULL;
    serialized_connections = NULL;
    if (draft->component_count != 0u)
    {
        serialized_components = (StrataPlaceholderSerializedComponent*)calloc(
            draft->component_count,
            sizeof(StrataPlaceholderSerializedComponent));
        if (!serialized_components)
        {
            free(serialized_descriptors);
            return BREADBOARD_ERR_INTERNAL;
        }

        serialize_breadboard_components(
            serialized_components,
            draft->components,
            draft->component_count);
    }

    if (draft->connection_count != 0u)
    {
        serialized_connections = (StrataPlaceholderSerializedConnection*)calloc(
            draft->connection_count,
            sizeof(StrataPlaceholderSerializedConnection));
        if (!serialized_connections)
        {
            free(serialized_components);
            free(serialized_descriptors);
            return BREADBOARD_ERR_INTERNAL;
        }

        serialize_breadboard_connections(
            serialized_connections,
            draft->connections,
            draft->connection_count);
    }

    serialize_breadboard_descriptors(
        serialized_descriptors,
        draft->inputs,
        draft->input_count);
    serialize_breadboard_descriptors(
        serialized_descriptors + draft->input_count,
        draft->outputs,
        draft->output_count);
    serialize_breadboard_descriptors(
        serialized_descriptors + draft->input_count + draft->output_count,
        draft->probes,
        draft->probe_count);

    if (!strata_placeholder_artifact_write_with_descriptors(
        buffer,
        buffer_size,
        target_backend_id,
        payload_kind,
        &admission_info,
        &draft_summary,
        serialized_components,
        (uint32_t)draft->component_count,
        serialized_connections,
        (uint32_t)draft->connection_count,
        serialized_descriptors,
        (uint32_t)draft->input_count,
        (uint32_t)draft->output_count,
        (uint32_t)draft->probe_count,
        out_size))
    {
        free(serialized_connections);
        free(serialized_components);
        free(serialized_descriptors);
        return BREADBOARD_ERR_INTERNAL;
    }

    free(serialized_connections);
    free(serialized_components);
    free(serialized_descriptors);
    return BREADBOARD_OK;
}

BreadboardResult breadboard_artifact_draft_export_fast_size(
    const BreadboardArtifactDraft* draft,
    size_t* out_size)
{
    if (!draft || !out_size)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (draft->admission_info.is_placeholder)
    {
        return BREADBOARD_ERR_UNSUPPORTED;
    }

    if (draft->target != BREADBOARD_TARGET_FAST_4STATE)
    {
        return BREADBOARD_ERR_UNSUPPORTED;
    }

    *out_size = real_fast_artifact_size_for_layout(
        (uint32_t)draft->input_count,
        (uint32_t)draft->output_count,
        (uint32_t)draft->probe_count,
        draft->fast_primitive_count,
        draft->fast_signal_count,
        draft->fast_input_binding_count,
        draft->fast_output_binding_count,
        (uint32_t)draft->component_count,
        (uint32_t)draft->connection_count);
    return BREADBOARD_OK;
}

BreadboardResult breadboard_artifact_draft_export_fast(
    const BreadboardArtifactDraft* draft,
    void* buffer,
    size_t buffer_size,
    size_t* out_size)
{
    size_t required_size;
    uint32_t target_backend_id;
    StrataPlaceholderPayloadKind payload_kind;
    StrataPlaceholderAdmissionInfo admission_info;
    StrataPlaceholderDraftSummary draft_summary;
    size_t total_descriptor_count;
    StrataPlaceholderSerializedDescriptor* serialized_descriptors;
    StrataPlaceholderSerializedComponent* serialized_components;
    StrataPlaceholderSerializedConnection* serialized_connections;

    if (!draft || !buffer || !out_size)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (draft->admission_info.is_placeholder)
    {
        return BREADBOARD_ERR_UNSUPPORTED;
    }

    if (draft->target != BREADBOARD_TARGET_FAST_4STATE)
    {
        return BREADBOARD_ERR_UNSUPPORTED;
    }

    required_size = real_fast_artifact_size_for_layout(
        (uint32_t)draft->input_count,
        (uint32_t)draft->output_count,
        (uint32_t)draft->probe_count,
        draft->fast_primitive_count,
        draft->fast_signal_count,
        draft->fast_input_binding_count,
        draft->fast_output_binding_count,
        (uint32_t)draft->component_count,
        (uint32_t)draft->connection_count);

    if (buffer_size < required_size)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (resolve_placeholder_backend_id(draft->target, &target_backend_id) != BREADBOARD_OK)
    {
        return BREADBOARD_ERR_UNSUPPORTED;
    }

    payload_kind = STRATA_PLACEHOLDER_PAYLOAD_FAST_EXECUTABLE_V1;

    if (!copy_export_admission_info(&draft->admission_info, &admission_info))
    {
        return BREADBOARD_ERR_INTERNAL;
    }

    draft_summary.source_target_value = (uint32_t)draft->info.target;
    draft_summary.has_placeholders = draft->info.has_placeholders ? 1u : 0u;
    draft_summary.approximate_size_bytes =
        (uint64_t)draft->info.approximate_size_bytes;
    draft_summary.source_module_id = draft->source_module_id;
    memset(draft_summary.source_module_name, 0, sizeof(draft_summary.source_module_name));
    memcpy(
        draft_summary.source_module_name,
        draft->source_module_name,
        sizeof(draft_summary.source_module_name) - 1u);
    draft_summary.declared_component_count =
        draft->structure_summary.declared_component_count;
    draft_summary.declared_connection_count =
        draft->structure_summary.declared_connection_count;
    draft_summary.declared_stateful_node_count =
        draft->structure_summary.declared_stateful_node_count;

    total_descriptor_count = draft->input_count +
        draft->output_count +
        draft->probe_count;
    serialized_descriptors = (StrataPlaceholderSerializedDescriptor*)calloc(
        total_descriptor_count,
        sizeof(StrataPlaceholderSerializedDescriptor));
    if (!serialized_descriptors)
    {
        return BREADBOARD_ERR_INTERNAL;
    }

    serialized_components = NULL;
    serialized_connections = NULL;
    if (draft->component_count != 0u)
    {
        serialized_components = (StrataPlaceholderSerializedComponent*)calloc(
            draft->component_count,
            sizeof(StrataPlaceholderSerializedComponent));
        if (!serialized_components)
        {
            free(serialized_descriptors);
            return BREADBOARD_ERR_INTERNAL;
        }

        serialize_breadboard_components(
            serialized_components,
            draft->components,
            draft->component_count);
    }

    if (draft->connection_count != 0u)
    {
        serialized_connections = (StrataPlaceholderSerializedConnection*)calloc(
            draft->connection_count,
            sizeof(StrataPlaceholderSerializedConnection));
        if (!serialized_connections)
        {
            free(serialized_components);
            free(serialized_descriptors);
            return BREADBOARD_ERR_INTERNAL;
        }

        serialize_breadboard_connections(
            serialized_connections,
            draft->connections,
            draft->connection_count);
    }

    serialize_breadboard_descriptors(
        serialized_descriptors,
        draft->inputs,
        draft->input_count);
    serialize_breadboard_descriptors(
        serialized_descriptors + draft->input_count,
        draft->outputs,
        draft->output_count);
    serialize_breadboard_descriptors(
        serialized_descriptors + draft->input_count + draft->output_count,
        draft->probes,
        draft->probe_count);

    {
        StrataPlaceholderArtifactHeader header;
        StrataPlaceholderSectionEntry sections[5];
        const size_t descriptor_bytes = strata_placeholder_descriptor_bytes_for_counts(
            (uint32_t)draft->input_count,
            (uint32_t)draft->output_count,
            (uint32_t)draft->probe_count);
        const size_t structure_bytes = strata_placeholder_structure_bytes_for_counts(
            (uint32_t)draft->component_count,
            (uint32_t)draft->connection_count);
        const size_t section_table_bytes = strata_placeholder_section_table_bytes(5u);
        const size_t payload_bytes = real_fast_payload_bytes_for_counts(
            draft->fast_primitive_count,
            draft->fast_signal_count,
            draft->fast_input_binding_count,
            draft->fast_output_binding_count);
        const size_t required_size = sizeof(StrataPlaceholderArtifactHeader) +
            section_table_bytes +
            sizeof(StrataPlaceholderAdmissionInfo) +
            sizeof(StrataPlaceholderDraftSummary) +
            structure_bytes +
            descriptor_bytes +
            payload_bytes;
        uint32_t admission_offset;
        uint32_t draft_summary_offset;
        uint32_t structure_offset;
        size_t component_bytes;
        size_t connection_bytes;
        StrataPlaceholderStructureSummary structure_summary;
        StrataPlaceholderFastExecutablePayloadHeader payload_header;

        if (required_size == 0u || buffer_size < required_size)
        {
            free(serialized_connections);
            free(serialized_components);
            free(serialized_descriptors);
            return BREADBOARD_ERR_INVALID_ARGUMENT;
        }

        memcpy(header.magic, k_strata_placeholder_artifact_magic, sizeof(header.magic));
        header.version_major = STRATA_PLACEHOLDER_ARTIFACT_VERSION_MAJOR;
        header.version_minor = STRATA_PLACEHOLDER_ARTIFACT_VERSION_MINOR;
        header.target_backend_id = target_backend_id;
        header.input_descriptor_count = (uint32_t)draft->input_count;
        header.output_descriptor_count = (uint32_t)draft->output_count;
        header.probe_descriptor_count = (uint32_t)draft->probe_count;
        header.section_table_offset = (uint32_t)sizeof(StrataPlaceholderArtifactHeader);
        header.section_count = 5u;
        header.descriptor_bytes = (uint32_t)descriptor_bytes;
        admission_offset = (uint32_t)(sizeof(StrataPlaceholderArtifactHeader) +
            section_table_bytes);
        draft_summary_offset = (uint32_t)(admission_offset +
            sizeof(StrataPlaceholderAdmissionInfo));
        structure_offset = (uint32_t)(draft_summary_offset +
            sizeof(StrataPlaceholderDraftSummary));
        header.descriptor_offset = (uint32_t)(structure_offset + structure_bytes);
        header.payload_size = (uint32_t)payload_bytes;
        header.payload_offset = (uint32_t)(header.descriptor_offset + descriptor_bytes);
        header.payload_kind = (uint32_t)payload_kind;
        header.admission_info = admission_info;

        sections[0].section_kind = STRATA_PLACEHOLDER_SECTION_ADMISSION;
        sections[0].section_offset = admission_offset;
        sections[0].section_size = sizeof(StrataPlaceholderAdmissionInfo);
        sections[1].section_kind = STRATA_PLACEHOLDER_SECTION_DRAFT_SUMMARY;
        sections[1].section_offset = draft_summary_offset;
        sections[1].section_size = sizeof(StrataPlaceholderDraftSummary);
        sections[2].section_kind = STRATA_PLACEHOLDER_SECTION_STRUCTURE;
        sections[2].section_offset = structure_offset;
        sections[2].section_size = (uint32_t)structure_bytes;
        sections[3].section_kind = STRATA_PLACEHOLDER_SECTION_DESCRIPTORS;
        sections[3].section_offset = header.descriptor_offset;
        sections[3].section_size = header.descriptor_bytes;
        sections[4].section_kind = STRATA_PLACEHOLDER_SECTION_PAYLOAD;
        sections[4].section_offset = header.payload_offset;
        sections[4].section_size = header.payload_size;

        component_bytes =
            (size_t)draft->component_count * sizeof(StrataPlaceholderSerializedComponent);
        connection_bytes =
            (size_t)draft->connection_count * sizeof(StrataPlaceholderSerializedConnection);

        structure_summary.component_count = (uint32_t)draft->component_count;
        structure_summary.connection_count = (uint32_t)draft->connection_count;

        memcpy(buffer, &header, sizeof(header));
        memcpy(
            ((uint8_t*)buffer) + header.section_table_offset,
            sections,
            section_table_bytes);
        memcpy(
            ((uint8_t*)buffer) + admission_offset,
            &admission_info,
            sizeof(StrataPlaceholderAdmissionInfo));
        memcpy(
            ((uint8_t*)buffer) + draft_summary_offset,
            &draft_summary,
            sizeof(StrataPlaceholderDraftSummary));
        memcpy(
            ((uint8_t*)buffer) + structure_offset,
            &structure_summary,
            sizeof(StrataPlaceholderStructureSummary));
        if (component_bytes != 0u && serialized_components)
        {
            memcpy(
                ((uint8_t*)buffer) + structure_offset +
                    sizeof(StrataPlaceholderStructureSummary),
                serialized_components,
                component_bytes);
        }
        if (connection_bytes != 0u && serialized_connections)
        {
            memcpy(
                ((uint8_t*)buffer) + structure_offset +
                    sizeof(StrataPlaceholderStructureSummary) +
                    component_bytes,
                serialized_connections,
                connection_bytes);
        }
        memcpy(
            ((uint8_t*)buffer) + header.descriptor_offset,
            serialized_descriptors,
            descriptor_bytes);

        payload_header.primitive_count = (uint32_t)draft->fast_primitive_count;
        payload_header.signal_count = (uint32_t)draft->fast_signal_count;
        payload_header.input_binding_count = (uint32_t)draft->fast_input_binding_count;
        payload_header.output_binding_count = (uint32_t)draft->fast_output_binding_count;
        memcpy(
            ((uint8_t*)buffer) + header.payload_offset,
            &payload_header,
            sizeof(payload_header));
        memcpy(
            ((uint8_t*)buffer) + header.payload_offset + sizeof(payload_header),
            draft->fast_signals,
            (size_t)draft->fast_signal_count * sizeof(StrataPlaceholderFastSignalRecord));
        memcpy(
            ((uint8_t*)buffer) + header.payload_offset + sizeof(payload_header) +
                ((size_t)draft->fast_signal_count * sizeof(StrataPlaceholderFastSignalRecord)),
            draft->fast_primitives,
            (size_t)draft->fast_primitive_count * sizeof(StrataPlaceholderFastPrimitiveRecord));
        memcpy(
            ((uint8_t*)buffer) + header.payload_offset + sizeof(payload_header) +
                ((size_t)draft->fast_signal_count * sizeof(StrataPlaceholderFastSignalRecord)) +
                ((size_t)draft->fast_primitive_count * sizeof(StrataPlaceholderFastPrimitiveRecord)),
            draft->fast_input_bindings,
            (size_t)draft->fast_input_binding_count * sizeof(StrataPlaceholderFastInputBinding));
        memcpy(
            ((uint8_t*)buffer) + header.payload_offset + sizeof(payload_header) +
                ((size_t)draft->fast_signal_count * sizeof(StrataPlaceholderFastSignalRecord)) +
                ((size_t)draft->fast_primitive_count * sizeof(StrataPlaceholderFastPrimitiveRecord)) +
                ((size_t)draft->fast_input_binding_count * sizeof(StrataPlaceholderFastInputBinding)),
            draft->fast_output_bindings,
            (size_t)draft->fast_output_binding_count * sizeof(StrataPlaceholderFastOutputBinding));

        *out_size = required_size;
    }

    free(serialized_connections);
    free(serialized_components);
    free(serialized_descriptors);
    return BREADBOARD_OK;
}

BreadboardResult breadboard_draft_component_count(
    const BreadboardArtifactDraft* draft,
    size_t* out_count)
{
    if (!draft || !out_count)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    *out_count = draft->component_count;
    return BREADBOARD_OK;
}

BreadboardResult breadboard_draft_component_at(
    const BreadboardArtifactDraft* draft,
    size_t index,
    BreadboardComponent* out_component)
{
    if (!draft || !out_component)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (index >= draft->component_count)
    {
        return BREADBOARD_ERR_OUT_OF_BOUNDS;
    }

    *out_component = draft->components[index];
    return BREADBOARD_OK;
}

BreadboardResult breadboard_draft_component_by_id(
    const BreadboardArtifactDraft* draft,
    uint64_t component_id,
    BreadboardComponent* out_component)
{
    size_t index;

    if (!draft || !out_component)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    for (index = 0u; index < draft->component_count; ++index)
    {
        if (draft->components[index].id == component_id)
        {
            *out_component = draft->components[index];
            return BREADBOARD_OK;
        }
    }

    return BREADBOARD_ERR_NOT_FOUND;
}

BreadboardResult breadboard_draft_connection_count(
    const BreadboardArtifactDraft* draft,
    size_t* out_count)
{
    if (!draft || !out_count)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    *out_count = draft->connection_count;
    return BREADBOARD_OK;
}

BreadboardResult breadboard_draft_connection_at(
    const BreadboardArtifactDraft* draft,
    size_t index,
    BreadboardConnection* out_connection)
{
    if (!draft || !out_connection)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (index >= draft->connection_count)
    {
        return BREADBOARD_ERR_OUT_OF_BOUNDS;
    }

    *out_connection = draft->connections[index];
    return BREADBOARD_OK;
}

BreadboardResult breadboard_module_query_target_info(
    const BreadboardModule* module,
    BreadboardTargetInfo* out_info)
{
    if (!module || !out_info)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    out_info->target = module->target;
    out_info->allowed_projection_families_mask = target_allowed_projection_families_mask(module->target);
    memset(out_info->reserved, 0, sizeof(out_info->reserved));
    return BREADBOARD_OK;
}

BreadboardResult breadboard_query_executable_subset_info(
    BreadboardTarget target,
    BreadboardExecutableSubsetInfo* out_info)
{
    if (!out_info)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (target != BREADBOARD_TARGET_FAST_4STATE &&
        target != BREADBOARD_TARGET_TEMPORAL)
    {
        return BREADBOARD_ERR_INVALID_TARGET;
    }

    if (target == BREADBOARD_TARGET_FAST_4STATE)
    {
        fill_fast_combinational_subset_info(out_info);
    }
    else
    {
        fill_empty_executable_subset_info(target, out_info);
    }

    return BREADBOARD_OK;
}

BreadboardResult breadboard_query_primitive_signature(
    BreadboardTarget target,
    BreadboardPrimitiveKind primitive_kind,
    BreadboardPrimitiveSignature* out_signature)
{
    if (!out_signature)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (target != BREADBOARD_TARGET_FAST_4STATE &&
        target != BREADBOARD_TARGET_TEMPORAL)
    {
        return BREADBOARD_ERR_INVALID_TARGET;
    }

    if (target != BREADBOARD_TARGET_FAST_4STATE)
    {
        return BREADBOARD_ERR_UNSUPPORTED;
    }

    if (!fill_primitive_signature(target, primitive_kind, out_signature))
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    return BREADBOARD_OK;
}

BreadboardResult breadboard_module_assess_executable_subset(
    const BreadboardModule* module,
    BreadboardExecutableAssessment* out_assessment)
{
    if (!module)
    {
        return BREADBOARD_ERR_INVALID_HANDLE;
    }

    if (!out_assessment)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    *out_assessment = assess_executable_subset(module);
    return BREADBOARD_OK;
}

BreadboardResult breadboard_draft_input_descriptor_count(
    const BreadboardArtifactDraft* draft,
    size_t* out_count)
{
    if (!draft || !out_count)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    *out_count = draft->input_count;
    return BREADBOARD_OK;
}

BreadboardResult breadboard_draft_input_descriptor_at(
    const BreadboardArtifactDraft* draft,
    size_t index,
    BreadboardDescriptor* out_descriptor)
{
    if (!draft || !out_descriptor)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (index >= draft->input_count || !draft->inputs)
    {
        return BREADBOARD_ERR_OUT_OF_BOUNDS;
    }

    *out_descriptor = draft->inputs[index];
    return BREADBOARD_OK;
}

BreadboardResult breadboard_draft_output_descriptor_count(
    const BreadboardArtifactDraft* draft,
    size_t* out_count)
{
    if (!draft || !out_count)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    *out_count = draft->output_count;
    return BREADBOARD_OK;
}

BreadboardResult breadboard_draft_output_descriptor_at(
    const BreadboardArtifactDraft* draft,
    size_t index,
    BreadboardDescriptor* out_descriptor)
{
    if (!draft || !out_descriptor)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (index >= draft->output_count || !draft->outputs)
    {
        return BREADBOARD_ERR_OUT_OF_BOUNDS;
    }

    *out_descriptor = draft->outputs[index];
    return BREADBOARD_OK;
}

BreadboardResult breadboard_draft_probe_descriptor_count(
    const BreadboardArtifactDraft* draft,
    size_t* out_count)
{
    if (!draft || !out_count)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    *out_count = draft->probe_count;
    return BREADBOARD_OK;
}

BreadboardResult breadboard_draft_probe_descriptor_at(
    const BreadboardArtifactDraft* draft,
    size_t index,
    BreadboardDescriptor* out_descriptor)
{
    if (!draft || !out_descriptor)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (index >= draft->probe_count || !draft->probes)
    {
        return BREADBOARD_ERR_OUT_OF_BOUNDS;
    }

    *out_descriptor = draft->probes[index];
    return BREADBOARD_OK;
}

BreadboardResult breadboard_draft_input_descriptor_by_id(
    const BreadboardArtifactDraft* draft,
    uint64_t id,
    BreadboardDescriptor* out_descriptor)
{
    if (!draft || !out_descriptor)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    for (size_t i = 0; i < draft->input_count; i++)
    {
        if (draft->inputs[i].id == id)
        {
            *out_descriptor = draft->inputs[i];
            return BREADBOARD_OK;
        }
    }

    return BREADBOARD_ERR_NOT_FOUND;
}

BreadboardResult breadboard_draft_input_descriptor_by_name(
    const BreadboardArtifactDraft* draft,
    const char* name,
    BreadboardDescriptor* out_descriptor)
{
    if (!draft || !name || !out_descriptor)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    for (size_t i = 0; i < draft->input_count; i++)
    {
        if (draft->inputs[i].name && strcmp(draft->inputs[i].name, name) == 0)
        {
            *out_descriptor = draft->inputs[i];
            return BREADBOARD_OK;
        }
    }

    return BREADBOARD_ERR_NOT_FOUND;
}

BreadboardResult breadboard_draft_output_descriptor_by_id(
    const BreadboardArtifactDraft* draft,
    uint64_t id,
    BreadboardDescriptor* out_descriptor)
{
    if (!draft || !out_descriptor)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    for (size_t i = 0; i < draft->output_count; i++)
    {
        if (draft->outputs[i].id == id)
        {
            *out_descriptor = draft->outputs[i];
            return BREADBOARD_OK;
        }
    }

    return BREADBOARD_ERR_NOT_FOUND;
}

BreadboardResult breadboard_draft_output_descriptor_by_name(
    const BreadboardArtifactDraft* draft,
    const char* name,
    BreadboardDescriptor* out_descriptor)
{
    if (!draft || !name || !out_descriptor)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    for (size_t i = 0; i < draft->output_count; i++)
    {
        if (draft->outputs[i].name && strcmp(draft->outputs[i].name, name) == 0)
        {
            *out_descriptor = draft->outputs[i];
            return BREADBOARD_OK;
        }
    }

    return BREADBOARD_ERR_NOT_FOUND;
}

BreadboardResult breadboard_draft_probe_descriptor_by_id(
    const BreadboardArtifactDraft* draft,
    uint64_t id,
    BreadboardDescriptor* out_descriptor)
{
    if (!draft || !out_descriptor)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    for (size_t i = 0; i < draft->probe_count; i++)
    {
        if (draft->probes[i].id == id)
        {
            *out_descriptor = draft->probes[i];
            return BREADBOARD_OK;
        }
    }

    return BREADBOARD_ERR_NOT_FOUND;
}

BreadboardResult breadboard_draft_probe_descriptor_by_name(
    const BreadboardArtifactDraft* draft,
    const char* name,
    BreadboardDescriptor* out_descriptor)
{
    if (!draft || !name || !out_descriptor)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    for (size_t i = 0; i < draft->probe_count; i++)
    {
        if (draft->probes[i].name && strcmp(draft->probes[i].name, name) == 0)
        {
            *out_descriptor = draft->probes[i];
            return BREADBOARD_OK;
        }
    }

    return BREADBOARD_ERR_NOT_FOUND;
}
