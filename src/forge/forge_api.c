#include <stdlib.h>
#include <string.h>
#include "../../include/forge_api.h"
#include "../../include/strata_placeholder_artifact.h"
#include "../common/forge_diagnostic.h"
#include "forge_internal.h"
#include "forge_policy.h"
#include "forge_registry.h"

/*
 * forge_api.c
 *
 * Artifact, descriptor, and session lifecycle for Forge.
 * Admitted real fast-path advancement is implemented here through the shared
 * runtime boundary.
 * Backend-specific execution kernels remain outside Forge.
 * All execution paths return explicit result codes.
 *
 * Backend discovery functions (forge_backend_count, forge_backend_id_at,
 * forge_backend_info, and forge_backend_capabilities) delegate directly to the
 * registry.
 */

/* -------------------------------------------------------------------------
 * Internal convenience: set diagnostic and return a result in one call.
 * ------------------------------------------------------------------------- */
static ForgeResult
forge_fail(ForgeResult result, const char *msg)
{
    forge_diag_set(msg);
    return result;
}

static void
forge_session_release_runtime_state(ForgeSession *session)
{
    if (!session)
    {
        return;
    }

    free(session->input_values);
    free(session->signal_values);
    free(session->output_values);
    session->input_values = NULL;
    session->signal_values = NULL;
    session->output_values = NULL;
    session->input_value_count = 0u;
    session->signal_value_count = 0u;
    session->output_value_count = 0u;
    session->real_fast_runtime = 0u;
}

static void
forge_session_reset_runtime_state(ForgeSession *session)
{
    uint32_t index;

    if (!session)
    {
        return;
    }

    if (session->input_values)
    {
        for (index = 0u; index < session->input_value_count; ++index)
        {
            session->input_values[index].value = FORGE_LOGIC_X;
        }
    }

    if (session->signal_values)
    {
        for (index = 0u; index < session->signal_value_count; ++index)
        {
            session->signal_values[index] = FORGE_LOGIC_X;
        }
    }

    if (session->output_values)
    {
        for (index = 0u; index < session->output_value_count; ++index)
        {
            session->output_values[index].value = FORGE_LOGIC_X;
        }
    }
}

static const StrataPlaceholderFastExecutablePayloadHeader *
forge_artifact_fast_payload_header(const ForgeArtifact *artifact)
{
    if (!artifact ||
        !artifact->payload_bytes ||
        artifact->placeholder_flags != 0u ||
        artifact->payload_size < sizeof(StrataPlaceholderFastExecutablePayloadHeader))
    {
        return NULL;
    }

    return (const StrataPlaceholderFastExecutablePayloadHeader *)artifact->payload_bytes;
}

static ForgeLogicValue
forge_logic_not(ForgeLogicValue value)
{
    switch (value)
    {
        case FORGE_LOGIC_0:
            return FORGE_LOGIC_1;
        case FORGE_LOGIC_1:
            return FORGE_LOGIC_0;
        case FORGE_LOGIC_X:
        case FORGE_LOGIC_Z:
        default:
            return FORGE_LOGIC_X;
    }
}

static ForgeLogicValue
forge_logic_and(ForgeLogicValue a, ForgeLogicValue b)
{
    if (a == FORGE_LOGIC_0 || b == FORGE_LOGIC_0)
    {
        return FORGE_LOGIC_0;
    }

    if (a == FORGE_LOGIC_1 && b == FORGE_LOGIC_1)
    {
        return FORGE_LOGIC_1;
    }

    return FORGE_LOGIC_X;
}

static ForgeLogicValue
forge_logic_or(ForgeLogicValue a, ForgeLogicValue b)
{
    if (a == FORGE_LOGIC_1 || b == FORGE_LOGIC_1)
    {
        return FORGE_LOGIC_1;
    }

    if (a == FORGE_LOGIC_0 && b == FORGE_LOGIC_0)
    {
        return FORGE_LOGIC_0;
    }

    return FORGE_LOGIC_X;
}

static ForgeLogicValue
forge_logic_xor(ForgeLogicValue a, ForgeLogicValue b)
{
    if (a == FORGE_LOGIC_X || a == FORGE_LOGIC_Z ||
        b == FORGE_LOGIC_X || b == FORGE_LOGIC_Z)
    {
        return FORGE_LOGIC_X;
    }

    if (a == b)
    {
        return FORGE_LOGIC_0;
    }

    return FORGE_LOGIC_1;
}

static ForgeLogicValue
forge_logic_value_from_opcode(
    uint32_t opcode,
    ForgeLogicValue input_a,
    ForgeLogicValue input_b)
{
    switch (opcode)
    {
        case STRATA_FAST_EXEC_OPCODE_BUF:
            return (input_a == FORGE_LOGIC_Z) ? FORGE_LOGIC_X : input_a;
        case STRATA_FAST_EXEC_OPCODE_NOT:
            return forge_logic_not(input_a);
        case STRATA_FAST_EXEC_OPCODE_AND:
            return forge_logic_and(input_a, input_b);
        case STRATA_FAST_EXEC_OPCODE_OR:
            return forge_logic_or(input_a, input_b);
        case STRATA_FAST_EXEC_OPCODE_XOR:
            return forge_logic_xor(input_a, input_b);
        default:
            return FORGE_LOGIC_X;
    }
}

static ForgeResult
forge_session_execute_real_fast(ForgeSession *session)
{
    const ForgeArtifact *artifact;
    const StrataPlaceholderFastExecutablePayloadHeader *payload_header;
    const StrataPlaceholderFastPrimitiveRecord *primitives;
    const StrataPlaceholderFastInputBinding *input_bindings;
    const StrataPlaceholderFastOutputBinding *output_bindings;
    uint32_t index;

    if (!session || !session->artifact || !session->signal_values ||
        !session->output_values || !session->input_values)
    {
        return forge_fail(FORGE_ERR_INTERNAL,
            "forge_step: real fast runtime state unavailable");
    }

    artifact = session->artifact;
    payload_header = forge_artifact_fast_payload_header(artifact);
    if (!payload_header)
    {
        return forge_fail(FORGE_ERR_INTERNAL,
            "forge_step: real fast payload header unavailable");
    }

    primitives = strata_placeholder_fast_payload_primitives(payload_header);
    input_bindings = strata_placeholder_fast_payload_input_bindings(payload_header);
    output_bindings = strata_placeholder_fast_payload_output_bindings(payload_header);
    if (!primitives || !input_bindings || !output_bindings)
    {
        return forge_fail(FORGE_ERR_INTERNAL,
            "forge_step: real fast payload records unavailable");
    }

    if (session->signal_value_count != payload_header->signal_count ||
        session->input_value_count != artifact->input_descriptor_count ||
        session->output_value_count != artifact->output_descriptor_count)
    {
        return forge_fail(FORGE_ERR_INTERNAL,
            "forge_step: real fast runtime dimensions changed unexpectedly");
    }

    for (index = 0u; index < session->signal_value_count; ++index)
    {
        session->signal_values[index] = FORGE_LOGIC_X;
    }

    for (index = 0u; index < session->input_value_count; ++index)
    {
        if (input_bindings[index].signal_index >= session->signal_value_count)
        {
            return forge_fail(FORGE_ERR_INTERNAL,
                "forge_step: input binding signal index out of bounds");
        }

        session->signal_values[input_bindings[index].signal_index] =
            session->input_values[index].value;
    }

    for (index = 0u; index < payload_header->primitive_count; ++index)
    {
        const StrataPlaceholderFastPrimitiveRecord *primitive;
        ForgeLogicValue input_a;
        ForgeLogicValue input_b;

        primitive = &primitives[index];
        if (primitive->output_signal_index >= session->signal_value_count ||
            primitive->input_signal_index_a >= session->signal_value_count)
        {
            return forge_fail(FORGE_ERR_INTERNAL,
                "forge_step: primitive signal index out of bounds");
        }

        input_a = session->signal_values[primitive->input_signal_index_a];
        input_b = FORGE_LOGIC_X;
        if (primitive->opcode != STRATA_FAST_EXEC_OPCODE_BUF &&
            primitive->opcode != STRATA_FAST_EXEC_OPCODE_NOT)
        {
            if (primitive->input_signal_index_b >= session->signal_value_count)
            {
                return forge_fail(FORGE_ERR_INTERNAL,
                    "forge_step: primitive secondary input signal index out of bounds");
            }

            input_b = session->signal_values[primitive->input_signal_index_b];
        }

        session->signal_values[primitive->output_signal_index] =
            forge_logic_value_from_opcode(
                primitive->opcode,
                input_a,
                input_b);
    }

    for (index = 0u; index < session->output_value_count; ++index)
    {
        if (output_bindings[index].signal_index >= session->signal_value_count)
        {
            return forge_fail(FORGE_ERR_INTERNAL,
                "forge_step: output binding signal index out of bounds");
        }

        session->output_values[index].value =
            session->signal_values[output_bindings[index].signal_index];
    }

    return FORGE_OK;
}

static ForgeResult
forge_session_prepare_runtime_state(ForgeSession *session)
{
    const ForgeArtifact *artifact;
    const StrataPlaceholderFastExecutablePayloadHeader *payload_header;
    size_t input_count;
    size_t signal_count;
    size_t output_count;
    size_t index;

    if (!session || !session->artifact)
    {
        return forge_fail(FORGE_ERR_INTERNAL,
            "forge_session_create: session artifact unavailable");
    }

    artifact = session->artifact;
    if (artifact->placeholder_flags != 0u)
    {
        forge_session_release_runtime_state(session);
        return FORGE_OK;
    }

    input_count = artifact->input_descriptor_count;
    output_count = artifact->output_descriptor_count;
    payload_header = forge_artifact_fast_payload_header(artifact);
    if (!payload_header)
    {
        forge_session_release_runtime_state(session);
        return forge_fail(FORGE_ERR_INTERNAL,
            "forge_session_create: real fast payload header unavailable");
    }

    signal_count = payload_header->signal_count;

    if (signal_count < input_count)
    {
        forge_session_release_runtime_state(session);
        return forge_fail(FORGE_ERR_INTERNAL,
            "forge_session_create: fast runtime signal count overflowed");
    }

    session->input_values = NULL;
    session->signal_values = NULL;
    session->output_values = NULL;
    session->input_value_count = 0u;
    session->signal_value_count = 0u;
    session->output_value_count = 0u;

    if (input_count != 0u)
    {
        session->input_values = (ForgeSignalValue *)calloc(
            input_count,
            sizeof(ForgeSignalValue));
        if (!session->input_values)
        {
            forge_session_release_runtime_state(session);
            return forge_fail(FORGE_ERR_INTERNAL,
                "forge_session_create: input state allocation failed");
        }

        for (index = 0u; index < input_count; ++index)
        {
            session->input_values[index].signal_id =
                artifact->descriptors[index].id;
            session->input_values[index].value = FORGE_LOGIC_X;
        }
        session->input_value_count = (uint32_t)input_count;
    }

    if (signal_count != 0u)
    {
        session->signal_values = (ForgeLogicValue *)calloc(
            signal_count,
            sizeof(ForgeLogicValue));
        if (!session->signal_values)
        {
            forge_session_release_runtime_state(session);
            return forge_fail(FORGE_ERR_INTERNAL,
                "forge_session_create: signal state allocation failed");
        }

        for (index = 0u; index < signal_count; ++index)
        {
            session->signal_values[index] = FORGE_LOGIC_X;
        }
        session->signal_value_count = (uint32_t)signal_count;
    }

    if (output_count != 0u)
    {
        session->output_values = (ForgeSignalValue *)calloc(
            output_count,
            sizeof(ForgeSignalValue));
        if (!session->output_values)
        {
            forge_session_release_runtime_state(session);
            return forge_fail(FORGE_ERR_INTERNAL,
                "forge_session_create: output state allocation failed");
        }

        for (index = 0u; index < output_count; ++index)
        {
            session->output_values[index].signal_id =
                artifact->descriptors[input_count + index].id;
            session->output_values[index].value = FORGE_LOGIC_X;
        }
        session->output_value_count = (uint32_t)output_count;
    }

    session->real_fast_runtime = 1u;
    return FORGE_OK;
}

/*
 * Placeholder artifact header validation for the first Forge slice.
 * The artifact must carry:
 * - shared magic
 * - format version
 * - target backend id
 * - payload size
 *
 * The payload is still placeholder-only and is governed by the temporary
 * shared placeholder handoff contract.
 */

static uint32_t
forge_extension_mask_for_family(ForgeExtensionFamily extension_family)
{
    switch (extension_family)
    {
        case FORGE_EXT_PERFORMANCE_PROFILE:
            return 1u << 0;
        case FORGE_EXT_TEMPORAL_CONTROL:
            return 1u << 1;
        case FORGE_EXT_NATIVE_STATE_READ:
            return 1u << 2;
        case FORGE_EXT_RUNTIME_DIAGNOSTICS:
            return 1u << 3;
        default:
            return 0u;
    }
}

static uint32_t
forge_extension_mask_from_placeholder_requirements(uint32_t requirement_flags)
{
    uint32_t result;

    result = 0u;

    if ((requirement_flags & STRATA_PLACEHOLDER_REQUIREMENT_ADVANCED_CONTROL) != 0u)
    {
        result |= forge_extension_mask_for_family(FORGE_EXT_TEMPORAL_CONTROL);
    }

    if ((requirement_flags & STRATA_PLACEHOLDER_REQUIREMENT_NATIVE_STATE) != 0u)
    {
        result |= forge_extension_mask_for_family(FORGE_EXT_NATIVE_STATE_READ);
    }

    return result;
}

static ForgeResult
forge_validate_artifact_requirements(
    const ForgeEffectiveProfile *profile,
    ForgeBackendClass backend_class,
    uint32_t required_extension_mask,
    uint32_t requires_advanced_controls,
    uint32_t requires_native_state_read,
    uint32_t requires_native_inputs)
{
    if (!profile)
    {
        return forge_fail(FORGE_ERR_INTERNAL,
            "forge_artifact_load: effective profile unavailable");
    }

    if ((required_extension_mask & ~profile->allowed_extension_mask) != 0u)
    {
        return forge_fail(FORGE_ERR_FORBIDDEN,
            "forge_artifact_load: artifact requires extension family denied by active profile");
    }

    if (requires_advanced_controls && !profile->allow_advanced_controls)
    {
        return forge_fail(FORGE_ERR_FORBIDDEN,
            "forge_artifact_load: artifact requires advanced controls denied by active profile");
    }

    if (requires_native_state_read && !profile->allow_native_state_read)
    {
        return forge_fail(FORGE_ERR_FORBIDDEN,
            "forge_artifact_load: artifact requires native state read denied by active profile");
    }

    if (requires_native_inputs && !profile->allow_native_inputs)
    {
        return forge_fail(FORGE_ERR_FORBIDDEN,
            "forge_artifact_load: artifact requires native inputs denied by active profile");
    }

    if ((requires_advanced_controls || requires_native_state_read || requires_native_inputs) &&
        backend_class != FORGE_BACKEND_CLASS_TEMPORAL)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: artifact requirements are incompatible with selected backend");
    }

    return FORGE_OK;
}

static uint32_t
forge_descriptor_class_visible(
    const ForgeEffectiveProfile *profile,
    ForgeDescriptorClass descriptor_class)
{
    if (!profile)
    {
        return 0u;
    }

    switch (descriptor_class)
    {
        case FORGE_DESCRIPTOR_CLASS_INPUT:
            return profile->allow_common_inputs != 0u;
        case FORGE_DESCRIPTOR_CLASS_OUTPUT:
            return profile->allow_common_observation != 0u;
        case FORGE_DESCRIPTOR_CLASS_PROBE:
            return profile->allow_common_probes != 0u;
        default:
            return 0u;
    }
}

static ForgeDescriptor
forge_descriptor_from_serialized(
    const StrataPlaceholderSerializedDescriptor *descriptor_data)
{
    ForgeDescriptor descriptor;

    descriptor.id = descriptor_data->id;
    descriptor.name = descriptor_data->name;
    descriptor.width = descriptor_data->width;
    descriptor.descriptor_class =
        (ForgeDescriptorClass)descriptor_data->class_type;
    descriptor.placeholder_flags = descriptor_data->placeholder_flags;

    return descriptor;
}

static const StrataPlaceholderSerializedDescriptor*
forge_artifact_input_descriptors(const ForgeArtifact *artifact)
{
    return artifact ? artifact->descriptors : NULL;
}

static const StrataPlaceholderSerializedDescriptor*
forge_artifact_output_descriptors(const ForgeArtifact *artifact)
{
    if (!artifact || !artifact->descriptors)
    {
        return NULL;
    }

    return artifact->descriptors + artifact->input_descriptor_count;
}

static const StrataPlaceholderSerializedDescriptor*
forge_artifact_probe_descriptors(const ForgeArtifact *artifact)
{
    if (!artifact || !artifact->descriptors)
    {
        return NULL;
    }

    return artifact->descriptors +
        artifact->input_descriptor_count +
        artifact->output_descriptor_count;
}

static ForgeStructureComponent
forge_structure_component_from_serialized(
    const StrataPlaceholderSerializedComponent *component_data)
{
    ForgeStructureComponent component;

    component.id = component_data->id;
    component.kind_name = component_data->kind_name;
    component.stateful_flags = component_data->stateful_flags;

    return component;
}

static ForgeStructureConnection
forge_structure_connection_from_serialized(
    const StrataPlaceholderSerializedConnection *connection_data)
{
    ForgeStructureConnection connection;

    connection.source_component_id = connection_data->source_component_id;
    connection.sink_component_id = connection_data->sink_component_id;

    return connection;
}

static ForgeResult
forge_descriptor_filtered_at(
    const ForgeEffectiveProfile *profile,
    const StrataPlaceholderSerializedDescriptor *descriptors,
    uint32_t count,
    uint32_t index,
    ForgeDescriptor *out_descriptor,
    const char *null_out_msg,
    const char *out_of_bounds_msg)
{
    uint32_t source_index;
    uint32_t visible_index;

    if (!out_descriptor)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT, null_out_msg);
    }

    visible_index = 0u;

    for (source_index = 0u; source_index < count; ++source_index)
    {
        if (!forge_descriptor_class_visible(
            profile,
            (ForgeDescriptorClass)descriptors[source_index].class_type))
        {
            continue;
        }

        if (visible_index == index)
        {
            *out_descriptor = forge_descriptor_from_serialized(
                &descriptors[source_index]);
            forge_diag_set("");
            return FORGE_OK;
        }

        ++visible_index;
    }

    return forge_fail(FORGE_ERR_OUT_OF_BOUNDS, out_of_bounds_msg);
}

static ForgeResult
forge_descriptor_find_by_id(
    const ForgeEffectiveProfile *profile,
    const StrataPlaceholderSerializedDescriptor *descriptors,
    uint32_t               count,
    uint32_t               descriptor_id,
    ForgeDescriptor       *out_descriptor,
    const char            *null_descriptor_msg,
    const char            *not_found_msg)
{
    uint32_t index;

    if (!out_descriptor)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT, null_descriptor_msg);
    }

    for (index = 0; index < count; ++index)
    {
        if (!forge_descriptor_class_visible(
            profile,
            (ForgeDescriptorClass)descriptors[index].class_type))
        {
            continue;
        }

        if (descriptors[index].id == descriptor_id)
        {
            *out_descriptor = forge_descriptor_from_serialized(
                &descriptors[index]);
            forge_diag_set("");
            return FORGE_OK;
        }
    }

    return forge_fail(FORGE_ERR_OUT_OF_BOUNDS, not_found_msg);
}

static ForgeResult
forge_descriptor_find_by_name(
    const ForgeEffectiveProfile *profile,
    const StrataPlaceholderSerializedDescriptor *descriptors,
    uint32_t               count,
    const char            *name,
    ForgeDescriptor       *out_descriptor,
    const char            *null_argument_msg,
    const char            *not_found_msg)
{
    uint32_t index;

    if (!name || !out_descriptor)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT, null_argument_msg);
    }

    for (index = 0; index < count; ++index)
    {
        if (!forge_descriptor_class_visible(
            profile,
            (ForgeDescriptorClass)descriptors[index].class_type))
        {
            continue;
        }

        if (strcmp(descriptors[index].name, name) == 0)
        {
            *out_descriptor = forge_descriptor_from_serialized(
                &descriptors[index]);
            forge_diag_set("");
            return FORGE_OK;
        }
    }

    return forge_fail(FORGE_ERR_OUT_OF_BOUNDS, not_found_msg);
}

static ForgeResult
forge_descriptor_filtered_count(
    const ForgeEffectiveProfile *profile,
    ForgeDescriptorClass descriptor_class,
    uint32_t count,
    uint32_t *out_count,
    const char *null_out_msg)
{
    if (!out_count)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT, null_out_msg);
    }

    if (!forge_descriptor_class_visible(profile, descriptor_class))
    {
        *out_count = 0u;
        forge_diag_set("");
        return FORGE_OK;
    }

    *out_count = count;
    forge_diag_set("");
    return FORGE_OK;
}

static ForgeResult
forge_validate_serialized_descriptor_block(
    const ForgeArtifactHeader *header,
    size_t size,
    const StrataPlaceholderAdmissionInfo **out_admission_info,
    const StrataPlaceholderDraftSummary **out_draft_summary,
    const StrataPlaceholderStructureSummary **out_structure_summary,
    const StrataPlaceholderSerializedComponent **out_components,
    const StrataPlaceholderSerializedConnection **out_connections,
    const StrataPlaceholderSerializedDescriptor **out_descriptors)
{
    const StrataPlaceholderAdmissionInfo *admission_info;
    const StrataPlaceholderSectionEntry *admission_section;
    const StrataPlaceholderDraftSummary *draft_summary;
    const StrataPlaceholderSectionEntry *draft_summary_section;
    const StrataPlaceholderStructureSummary *structure_summary;
    const StrataPlaceholderSectionEntry *structure_section;
    const StrataPlaceholderSerializedComponent *components;
    const StrataPlaceholderSerializedConnection *connections;
    const StrataPlaceholderSerializedDescriptor *descriptors;
    const StrataPlaceholderSectionEntry *descriptor_section;
    const StrataPlaceholderSectionEntry *payload_section;
    size_t expected_descriptor_bytes;
    size_t expected_structure_bytes;
    size_t max_descriptor_entries;
    size_t section_table_bytes;
    size_t total_expected_size;
    size_t total_descriptor_count;
    uint32_t index;
    uint32_t connection_index;
    uint32_t component_index;
    int endpoint_found;

    if (!header || !out_admission_info || !out_draft_summary ||
        !out_structure_summary || !out_components || !out_connections || !out_descriptors)
    {
        return forge_fail(FORGE_ERR_INTERNAL,
            "forge_artifact_load: descriptor validation received invalid input");
    }

    if (header->section_count != 5u)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: unsupported placeholder section count");
    }

    section_table_bytes = strata_placeholder_section_table_bytes(
        header->section_count);
    if ((size_t)header->section_table_offset < sizeof(ForgeArtifactHeader) ||
        (size_t)header->section_table_offset > size ||
        (size_t)header->section_table_offset + section_table_bytes > size)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: section table is out of bounds");
    }

    if ((size_t)header->section_table_offset + section_table_bytes >
        (size_t)header->descriptor_offset)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: section table overlaps descriptor block");
    }

    if ((size_t)header->section_table_offset + section_table_bytes >
        (size_t)header->payload_offset)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: section table overlaps payload block");
    }

    if ((size_t)header->descriptor_offset < sizeof(ForgeArtifactHeader) ||
        (size_t)header->descriptor_offset > size)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: descriptor offset is out of bounds");
    }

    if ((size_t)header->payload_offset < sizeof(ForgeArtifactHeader) ||
        (size_t)header->payload_offset > size)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: payload offset is out of bounds");
    }

    if ((size_t)header->descriptor_offset > (size_t)header->payload_offset)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: descriptor offset exceeds payload offset");
    }

    descriptor_section = strata_placeholder_find_section_entry(
        header,
        STRATA_PLACEHOLDER_SECTION_DESCRIPTORS);
    structure_section = strata_placeholder_find_section_entry(
        header,
        STRATA_PLACEHOLDER_SECTION_STRUCTURE);
    payload_section = strata_placeholder_find_section_entry(
        header,
        STRATA_PLACEHOLDER_SECTION_PAYLOAD);
    draft_summary_section = strata_placeholder_find_section_entry(
        header,
        STRATA_PLACEHOLDER_SECTION_DRAFT_SUMMARY);
    admission_section = strata_placeholder_find_section_entry(
        header,
        STRATA_PLACEHOLDER_SECTION_ADMISSION);

    if (!admission_section || !draft_summary_section || !structure_section ||
        !descriptor_section || !payload_section)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: required placeholder sections are missing");
    }

    if ((size_t)admission_section->section_offset < sizeof(ForgeArtifactHeader) ||
        (size_t)admission_section->section_offset > size ||
        (size_t)admission_section->section_offset + admission_section->section_size > size)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: admission section is out of bounds");
    }

    if (admission_section->section_size != sizeof(StrataPlaceholderAdmissionInfo))
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: admission section size is invalid");
    }

    if ((size_t)header->section_table_offset + section_table_bytes >
        (size_t)admission_section->section_offset)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: section table overlaps admission section");
    }

    if ((size_t)admission_section->section_offset + admission_section->section_size >
        (size_t)header->descriptor_offset)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: admission section overlaps later blocks");
    }

    if ((size_t)draft_summary_section->section_offset < sizeof(ForgeArtifactHeader) ||
        (size_t)draft_summary_section->section_offset > size ||
        (size_t)draft_summary_section->section_offset + draft_summary_section->section_size > size)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: draft summary section is out of bounds");
    }

    if (draft_summary_section->section_size != sizeof(StrataPlaceholderDraftSummary))
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: draft summary section size is invalid");
    }

    if ((size_t)admission_section->section_offset + admission_section->section_size >
        (size_t)draft_summary_section->section_offset)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: admission section overlaps draft summary section");
    }

    if ((size_t)draft_summary_section->section_offset + draft_summary_section->section_size >
        (size_t)structure_section->section_offset)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: draft summary section overlaps structure section");
    }

    if ((size_t)structure_section->section_offset < sizeof(ForgeArtifactHeader) ||
        (size_t)structure_section->section_offset > size ||
        (size_t)structure_section->section_offset + structure_section->section_size > size)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: structure section is out of bounds");
    }

    structure_summary = strata_placeholder_artifact_structure_summary(header);
    if (!structure_summary)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: structure section data is missing");
    }

    expected_structure_bytes = strata_placeholder_structure_bytes_for_counts(
        structure_summary->component_count,
        structure_summary->connection_count);
    if (structure_section->section_size != expected_structure_bytes)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: structure section size does not match structure counts");
    }

    if ((size_t)structure_section->section_offset + structure_section->section_size >
        (size_t)header->descriptor_offset)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: structure section overlaps descriptor block");
    }

    if (descriptor_section->section_offset != header->descriptor_offset ||
        descriptor_section->section_size != header->descriptor_bytes)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: descriptor section does not match header");
    }

    if (payload_section->section_offset != header->payload_offset ||
        payload_section->section_size != header->payload_size)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: payload section does not match header");
    }

    admission_info = strata_placeholder_artifact_admission_info(header);
    if (!admission_info)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: admission section data is missing");
    }

    if (memcmp(admission_info, &header->admission_info, sizeof(*admission_info)) != 0)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: admission section does not match header");
    }

    draft_summary = strata_placeholder_artifact_draft_summary(header);
    if (!draft_summary)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: draft summary section data is missing");
    }

    if (strata_placeholder_backend_id_for_target_value(
            draft_summary->source_target_value) != header->target_backend_id)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: draft summary target does not match backend");
    }

    if (!memchr(
            draft_summary->source_module_name,
            '\0',
            sizeof(draft_summary->source_module_name)))
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: draft summary module name is not terminated");
    }

    max_descriptor_entries =
        ((size_t)header->payload_offset - (size_t)header->descriptor_offset) /
        sizeof(StrataPlaceholderSerializedDescriptor);
    if ((size_t)header->input_descriptor_count > max_descriptor_entries ||
        (size_t)header->output_descriptor_count > max_descriptor_entries ||
        (size_t)header->probe_descriptor_count > max_descriptor_entries)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: descriptor counts exceed artifact bounds");
    }

    total_descriptor_count = (size_t)header->input_descriptor_count;
    if ((size_t)header->output_descriptor_count >
        max_descriptor_entries - total_descriptor_count)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: descriptor counts overflow artifact bounds");
    }
    total_descriptor_count += (size_t)header->output_descriptor_count;

    if ((size_t)header->probe_descriptor_count >
        max_descriptor_entries - total_descriptor_count)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: descriptor counts overflow artifact bounds");
    }
    total_descriptor_count += (size_t)header->probe_descriptor_count;

    expected_descriptor_bytes = strata_placeholder_descriptor_bytes_for_counts(
        header->input_descriptor_count,
        header->output_descriptor_count,
        header->probe_descriptor_count);

    if ((size_t)header->descriptor_bytes != expected_descriptor_bytes)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: descriptor block size does not match descriptor counts");
    }

    if ((size_t)header->descriptor_offset + (size_t)header->descriptor_bytes >
        (size_t)header->payload_offset)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: descriptor block overlaps payload block");
    }

    total_expected_size = (size_t)header->payload_offset +
        (size_t)header->payload_size;

    if (size != total_expected_size)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: artifact size does not match declared block offsets");
    }

    descriptors = strata_placeholder_artifact_descriptors(header);
    components = strata_placeholder_artifact_components(header);
    connections = strata_placeholder_artifact_connections(header);

    for (index = 0u; index < structure_summary->component_count; ++index)
    {
        if (memchr(components[index].kind_name, '\0', sizeof(components[index].kind_name)) == NULL)
        {
            return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
                "forge_artifact_load: component structure block is malformed");
        }
    }

    for (connection_index = 0u;
         connection_index < structure_summary->connection_count;
         ++connection_index)
    {
        endpoint_found = 0;
        for (component_index = 0u;
             component_index < structure_summary->component_count;
             ++component_index)
        {
            if (components[component_index].id ==
                connections[connection_index].source_component_id)
            {
                endpoint_found = 1;
                break;
            }
        }

        if (!endpoint_found)
        {
            return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
                "forge_artifact_load: connection source does not match a declared component");
        }

        endpoint_found = 0;
        for (component_index = 0u;
             component_index < structure_summary->component_count;
             ++component_index)
        {
            if (components[component_index].id ==
                connections[connection_index].sink_component_id)
            {
                endpoint_found = 1;
                break;
            }
        }

        if (!endpoint_found)
        {
            return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
                "forge_artifact_load: connection sink does not match a declared component");
        }
    }

    for (index = 0u; index < header->input_descriptor_count; ++index)
    {
        if (descriptors[index].class_type != FORGE_DESCRIPTOR_CLASS_INPUT ||
            memchr(descriptors[index].name, '\0', sizeof(descriptors[index].name)) == NULL)
        {
            return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
                "forge_artifact_load: input descriptor block is malformed");
        }
    }

    for (index = header->input_descriptor_count;
         index < header->input_descriptor_count + header->output_descriptor_count;
         ++index)
    {
        if (descriptors[index].class_type != FORGE_DESCRIPTOR_CLASS_OUTPUT ||
            memchr(descriptors[index].name, '\0', sizeof(descriptors[index].name)) == NULL)
        {
            return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
                "forge_artifact_load: output descriptor block is malformed");
        }
    }

    for (index = header->input_descriptor_count + header->output_descriptor_count;
         index < total_descriptor_count;
         ++index)
    {
        if (descriptors[index].class_type != FORGE_DESCRIPTOR_CLASS_PROBE ||
            memchr(descriptors[index].name, '\0', sizeof(descriptors[index].name)) == NULL)
        {
            return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
                "forge_artifact_load: probe descriptor block is malformed");
        }
    }

    *out_admission_info = admission_info;
    *out_draft_summary = draft_summary;
    *out_structure_summary = structure_summary;
    *out_components = components;
    *out_connections = connections;
    *out_descriptors = descriptors;
    return FORGE_OK;
}

/* -------------------------------------------------------------------------
 * Backend Discovery
 * ------------------------------------------------------------------------- */

uint32_t
forge_backend_count(void)
{
    ForgeEffectiveProfile profile;
    uint32_t index;
    uint32_t visible_count;

    forge_policy_get_library_effective_profile(&profile);

    visible_count = 0u;

    for (index = 0u; index < forge_registry_backend_count(); ++index)
    {
        const ForgeBackendRecord *rec;

        rec = forge_registry_backend_at(index);

        if (rec && forge_policy_backend_visible(&profile, rec->id))
        {
            ++visible_count;
        }
    }

    return visible_count;
}

ForgeResult
forge_install_product_profile(ForgeProductProfileKind profile_kind)
{
    ForgeBuildCapabilitySet build;
    ForgeProductExposureProfile product;

    switch (profile_kind)
    {
        case FORGE_PRODUCT_PROFILE_UNRESTRICTED:
        case FORGE_PRODUCT_PROFILE_LXS_ONLY:
        case FORGE_PRODUCT_PROFILE_COMMON_ONLY:
            break;
        default:
            return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
                "forge_install_product_profile: invalid profile kind");
    }

    forge_policy_get_build_capabilities(&build);
    forge_policy_build_product_profile_kind(profile_kind, &product);

    if (!forge_policy_profile_is_valid(&build, &product))
    {
        return forge_fail(FORGE_ERR_UNSUPPORTED,
            "forge_install_product_profile: requested profile is not valid for this build");
    }

    forge_policy_install_product_profile(&product);
    forge_diag_set("");
    return FORGE_OK;
}

static ForgeResult
forge_validate_fast_payload_descriptor_bindings(
    const ForgeArtifactHeader *header,
    const StrataPlaceholderFastExecutablePayloadHeader *payload_header,
    const StrataPlaceholderSerializedDescriptor *descriptors)
{
    const StrataPlaceholderFastInputBinding *input_bindings;
    const StrataPlaceholderFastOutputBinding *output_bindings;
    uint32_t index;

    if (!header || !payload_header || !descriptors)
    {
        return forge_fail(FORGE_ERR_INTERNAL,
            "forge_artifact_load: fast payload binding validation received invalid input");
    }

    input_bindings = strata_placeholder_fast_payload_input_bindings(payload_header);
    output_bindings = strata_placeholder_fast_payload_output_bindings(payload_header);
    if (!input_bindings || !output_bindings)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: fast payload bindings are not addressable");
    }

    for (index = 0u; index < header->input_descriptor_count; ++index)
    {
        if (input_bindings[index].descriptor_id != descriptors[index].id)
        {
            return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
                "forge_artifact_load: input binding descriptor does not match artifact");
        }
    }

    for (index = 0u; index < header->output_descriptor_count; ++index)
    {
        if (output_bindings[index].descriptor_id !=
            descriptors[header->input_descriptor_count + index].id)
        {
            return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
                "forge_artifact_load: output binding descriptor does not match artifact");
        }
    }

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_backend_id_at(uint32_t index, ForgeBackendId *out_id)
{
    ForgeEffectiveProfile profile;
    uint32_t source_index;
    uint32_t visible_index;

    if (!out_id)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_backend_id_at: out_id is NULL");
    }

    forge_policy_get_library_effective_profile(&profile);

    visible_index = 0u;

    for (source_index = 0u; source_index < forge_registry_backend_count(); ++source_index)
    {
        const ForgeBackendRecord *rec;

        rec = forge_registry_backend_at(source_index);

        if (!rec || !forge_policy_backend_visible(&profile, rec->id))
        {
            continue;
        }

        if (visible_index == index)
        {
            *out_id = rec->id;
            forge_diag_set("");
            return FORGE_OK;
        }

        ++visible_index;
    }

    return forge_fail(FORGE_ERR_OUT_OF_BOUNDS,
        "forge_backend_id_at: index out of bounds");
}

ForgeResult
forge_backend_info(ForgeBackendId backend_id, ForgeBackendInfo *out_info)
{
    const ForgeBackendRecord *rec;
    ForgeEffectiveProfile profile;

    if (!out_info)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_backend_info: out_info is NULL");
    }

    rec = forge_registry_backend_by_id(backend_id);

    forge_policy_get_library_effective_profile(&profile);

    if (!rec || !forge_policy_backend_visible(&profile, backend_id))
    {
        return forge_fail(FORGE_ERR_BACKEND_UNAVAILABLE,
            "forge_backend_info: backend_id not registered");
    }

    *out_info = rec->info;

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_backend_capabilities(ForgeBackendId backend_id, ForgeCapabilities *out_caps)
{
    const ForgeBackendRecord *rec;
    ForgeEffectiveProfile profile;
    uint32_t source_index;
    uint32_t output_index;

    if (!out_caps)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_backend_capabilities: out_caps is NULL");
    }

    rec = forge_registry_backend_by_id(backend_id);

    forge_policy_get_library_effective_profile(&profile);

    if (!rec || !forge_policy_backend_visible(&profile, backend_id))
    {
        return forge_fail(FORGE_ERR_BACKEND_UNAVAILABLE,
            "forge_backend_capabilities: backend_id not registered");
    }

    *out_caps = rec->capabilities;

    if (!profile.allow_common_observation)
    {
        out_caps->observation = FORGE_SUPPORT_NONE;
    }

    if (!profile.allow_common_probes)
    {
        out_caps->probe_support = FORGE_SUPPORT_NONE;
    }

    if (!profile.allow_advanced_controls)
    {
        out_caps->temporal_substep = 0u;
        out_caps->delta_phase_stepping = 0u;
    }

    output_index = 0u;

    for (source_index = 0u; source_index < rec->capabilities.extension_family_count;
         ++source_index)
    {
        ForgeExtensionFamily extension_family;

        extension_family = rec->capabilities.extension_families[source_index];

        if (!forge_policy_extension_allowed(&profile, extension_family))
        {
            continue;
        }

        if (extension_family == FORGE_EXT_NATIVE_STATE_READ &&
            !profile.allow_native_state_read)
        {
            continue;
        }

        if (extension_family == FORGE_EXT_TEMPORAL_CONTROL &&
            !profile.allow_advanced_controls)
        {
            continue;
        }

        out_caps->extension_families[output_index] = extension_family;
        ++output_index;
    }

    out_caps->extension_family_count = output_index;

    forge_diag_set("");
    return FORGE_OK;
}

/* -------------------------------------------------------------------------
 * Artifact Lifecycle
 * ------------------------------------------------------------------------- */

ForgeResult
forge_artifact_load(
    ForgeBackendId   backend_id,
    const void      *data,
    size_t           size,
    ForgeArtifact  **out_artifact)
{
    const ForgeBackendRecord *rec;
    ForgeArtifact            *art;
    const ForgeArtifactHeader *header;
    const StrataPlaceholderAdmissionInfo *admission_info;
    const StrataPlaceholderDraftSummary *draft_summary;
    const StrataPlaceholderStructureSummary *structure_summary;
    const StrataPlaceholderSerializedComponent *serialized_components;
    const StrataPlaceholderSerializedConnection *serialized_connections;
    const StrataPlaceholderSerializedDescriptor *serialized_descriptors;
    const unsigned char *payload;
    ForgeEffectiveProfile profile;
    StrataPlaceholderPayloadKind payload_kind;
    uint32_t required_extension_mask;
    uint32_t requires_advanced_controls;
    uint32_t requires_native_state_read;
    uint32_t requires_native_inputs;
    uint32_t placeholder_flags;
    ForgeResult descriptor_result;
    ForgeResult requirement_result;

    if (!out_artifact || !data)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_artifact_load: NULL argument");
    }

    *out_artifact = NULL;

    rec = forge_registry_backend_by_id(backend_id);

    if (!rec)
    {
        return forge_fail(FORGE_ERR_BACKEND_UNAVAILABLE,
            "forge_artifact_load: backend_id not registered");
    }

    forge_policy_get_library_effective_profile(&profile);

    if (!forge_policy_backend_visible(&profile, backend_id))
    {
        return forge_fail(FORGE_ERR_FORBIDDEN,
            "forge_artifact_load: backend is denied by active profile");
    }

    if (size < sizeof(ForgeArtifactHeader))
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: artifact smaller than required header");
    }

    header = (const ForgeArtifactHeader *)data;

    if (memcmp(header->magic,
        k_strata_placeholder_artifact_magic,
        STRATA_PLACEHOLDER_ARTIFACT_MAGIC_LEN) != 0)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: artifact magic mismatch");
    }

    if (header->version_major != STRATA_PLACEHOLDER_ARTIFACT_VERSION_MAJOR ||
        header->version_minor != STRATA_PLACEHOLDER_ARTIFACT_VERSION_MINOR)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: unsupported artifact version");
    }

    if (header->target_backend_id != backend_id)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: artifact target backend mismatch");
    }

    descriptor_result = forge_validate_serialized_descriptor_block(
        header,
        size,
        &admission_info,
        &draft_summary,
        &structure_summary,
        &serialized_components,
        &serialized_connections,
        &serialized_descriptors);
    if (descriptor_result != FORGE_OK)
    {
        return descriptor_result;
    }

    if ((size_t)header->payload_size != size - (size_t)header->payload_offset)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: payload size does not match artifact size");
    }

    payload = strata_placeholder_artifact_payload(header);
    payload_kind = (StrataPlaceholderPayloadKind)header->payload_kind;
    required_extension_mask = 0u;
    placeholder_flags = 1u;
    requires_advanced_controls = admission_info->requires_advanced_controls;
    requires_native_state_read = admission_info->requires_native_state_read;
    requires_native_inputs = admission_info->requires_native_inputs;

    /*
     * Stub and fast-executable success paths:
     * valid shared header plus a coherent payload family returns a minimal
     * ForgeArtifact with coarse admission metadata.
     */
    if (header->payload_size == STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN)
    {
        if (!strata_placeholder_payload_matches(payload, payload_kind))
        {
            return forge_fail(FORGE_ERR_UNSUPPORTED,
                "forge_artifact_load: artifact header valid but payload decoding is not implemented");
        }

        if (!strata_placeholder_admission_matches_payload_kind(
            payload_kind,
            admission_info))
        {
            return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
                "forge_artifact_load: placeholder admission manifest does not match payload kind");
        }

        required_extension_mask = forge_extension_mask_from_placeholder_requirements(
            admission_info->requirement_flags);

        requirement_result = forge_validate_artifact_requirements(
            &profile,
            rec->info.backend_class,
            required_extension_mask,
            requires_advanced_controls,
            requires_native_state_read,
            requires_native_inputs);

        if (requirement_result != FORGE_OK)
        {
            return requirement_result;
        }

        placeholder_flags = 1u;
    }
    else if (strata_placeholder_payload_kind_is_real_fast_executable(payload_kind))
    {
        if (!strata_placeholder_fast_payload_header_is_coherent(header))
        {
            return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
                "forge_artifact_load: real fast payload header is not coherent");
        }

        if (forge_validate_fast_payload_descriptor_bindings(
            header,
            strata_placeholder_fast_payload_header(header),
            serialized_descriptors) != FORGE_OK)
        {
            return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
                "forge_artifact_load: real fast payload bindings do not match descriptors");
        }

        if (admission_info->requirement_flags != STRATA_PLACEHOLDER_REQUIREMENT_NONE ||
            admission_info->requires_advanced_controls != 0u ||
            admission_info->requires_native_state_read != 0u ||
            admission_info->requires_native_inputs != 0u)
        {
            return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
                "forge_artifact_load: real fast admission manifest must be empty");
        }

        requirement_result = forge_validate_artifact_requirements(
            &profile,
            rec->info.backend_class,
            0u,
            requires_advanced_controls,
            requires_native_state_read,
            requires_native_inputs);

        if (requirement_result != FORGE_OK)
        {
            return requirement_result;
        }

        placeholder_flags = 0u;
    }
    else
    {
        return forge_fail(FORGE_ERR_UNSUPPORTED,
            "forge_artifact_load: artifact header valid but payload decoding is not implemented");
    }

    art = (ForgeArtifact *)malloc(sizeof(ForgeArtifact));

        if (!art)
        {
            return forge_fail(FORGE_ERR_INTERNAL,
                "forge_artifact_load: allocation failed");
        }

        art->backend_id = backend_id;
        art->format_version_major = header->version_major;
        art->format_version_minor = header->version_minor;
        art->source_target_value = draft_summary->source_target_value;
        art->source_has_placeholders = draft_summary->has_placeholders;
        art->source_approximate_size_bytes = draft_summary->approximate_size_bytes;
        art->source_module_id = draft_summary->source_module_id;
        memset(art->source_module_name, 0, sizeof(art->source_module_name));
        memcpy(
            art->source_module_name,
            draft_summary->source_module_name,
            sizeof(art->source_module_name) - 1u);
        art->source_declared_component_count =
            draft_summary->declared_component_count;
        art->source_declared_connection_count =
            draft_summary->declared_connection_count;
        art->source_declared_stateful_node_count =
            draft_summary->declared_stateful_node_count;
        art->payload_size = header->payload_size;
        art->payload_bytes = NULL;
        art->input_descriptor_count = header->input_descriptor_count;
        art->output_descriptor_count = header->output_descriptor_count;
        art->probe_descriptor_count = header->probe_descriptor_count;
        art->source_size = size;
        art->placeholder_flags = placeholder_flags;
        art->required_extension_mask = required_extension_mask;
        art->requires_advanced_controls = requires_advanced_controls;
        art->requires_native_state_read = requires_native_state_read;
        art->requires_native_inputs = requires_native_inputs;
        art->structure_component_count = structure_summary->component_count;
        art->structure_connection_count = structure_summary->connection_count;
        art->descriptors = NULL;
        art->components = NULL;
        art->connections = NULL;
        art->effective_profile = profile;

        if (header->descriptor_bytes != 0u)
        {
            art->descriptors = (StrataPlaceholderSerializedDescriptor*)malloc(
                (size_t)header->descriptor_bytes);
            if (!art->descriptors)
            {
                free(art);
                return forge_fail(FORGE_ERR_INTERNAL,
                    "forge_artifact_load: descriptor block allocation failed");
            }

            memcpy(
                art->descriptors,
                serialized_descriptors,
                (size_t)header->descriptor_bytes);
        }

        if (structure_summary->component_count != 0u)
        {
            art->components = (StrataPlaceholderSerializedComponent*)malloc(
                (size_t)structure_summary->component_count *
                sizeof(StrataPlaceholderSerializedComponent));
            if (!art->components)
            {
                free(art->descriptors);
                free(art);
                return forge_fail(FORGE_ERR_INTERNAL,
                    "forge_artifact_load: structure component allocation failed");
            }

            memcpy(
                art->components,
                serialized_components,
                (size_t)structure_summary->component_count *
                sizeof(StrataPlaceholderSerializedComponent));
        }

        if (structure_summary->connection_count != 0u)
        {
            art->connections = (StrataPlaceholderSerializedConnection*)malloc(
                (size_t)structure_summary->connection_count *
                sizeof(StrataPlaceholderSerializedConnection));
            if (!art->connections)
            {
                free(art->components);
                free(art->descriptors);
                free(art);
                return forge_fail(FORGE_ERR_INTERNAL,
                    "forge_artifact_load: structure connection allocation failed");
            }

            memcpy(
                art->connections,
                serialized_connections,
                (size_t)structure_summary->connection_count *
                sizeof(StrataPlaceholderSerializedConnection));
        }

        if (art->payload_size != 0u)
        {
            art->payload_bytes = (uint8_t*)malloc((size_t)art->payload_size);
            if (!art->payload_bytes)
            {
                free(art->connections);
                free(art->components);
                free(art->descriptors);
                free(art);
                return forge_fail(FORGE_ERR_INTERNAL,
                    "forge_artifact_load: payload block allocation failed");
            }

            memcpy(
                art->payload_bytes,
                payload,
                (size_t)art->payload_size);
        }

        *out_artifact = art;

        forge_diag_set("");
        return FORGE_OK;
}

ForgeResult
forge_artifact_info(
    const ForgeArtifact *artifact,
    ForgeArtifactInfo   *out_info)
{
    if (!out_info)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_artifact_info: out_info is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_artifact_info: artifact is NULL");
    }

    out_info->backend_id = artifact->backend_id;
    out_info->format_version_major = artifact->format_version_major;
    out_info->format_version_minor = artifact->format_version_minor;
    out_info->source_target_value = artifact->source_target_value;
    out_info->source_has_placeholders = artifact->source_has_placeholders;
    out_info->source_approximate_size_bytes = artifact->source_approximate_size_bytes;
    out_info->source_module_id = artifact->source_module_id;
    out_info->source_module_name = artifact->source_module_name;
    out_info->source_declared_component_count =
        artifact->source_declared_component_count;
    out_info->source_declared_connection_count =
        artifact->source_declared_connection_count;
    out_info->source_declared_stateful_node_count =
        artifact->source_declared_stateful_node_count;
    out_info->payload_size = artifact->payload_size;
    out_info->placeholder_flags = artifact->placeholder_flags;
    out_info->required_extension_mask = artifact->required_extension_mask;
    out_info->requires_advanced_controls = artifact->requires_advanced_controls;
    out_info->requires_native_state_read = artifact->requires_native_state_read;
    out_info->requires_native_inputs = artifact->requires_native_inputs;
    out_info->source_size = artifact->source_size;

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_artifact_unload(ForgeArtifact *artifact)
{
    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_artifact_unload: artifact is NULL");
    }

    if (artifact->descriptors)
    {
        free(artifact->descriptors);
    }
    if (artifact->components)
    {
        free(artifact->components);
    }
    if (artifact->connections)
    {
        free(artifact->connections);
    }
    if (artifact->payload_bytes)
    {
        free(artifact->payload_bytes);
    }
    free(artifact);

    forge_diag_set("");
    return FORGE_OK;
}

/* -------------------------------------------------------------------------
 * Session Lifecycle
 * ------------------------------------------------------------------------- */

ForgeResult
forge_session_create(
    ForgeArtifact  *artifact,
    ForgeSession  **out_session)
{
    return forge_session_create_with_profile(
        artifact,
        FORGE_SESSION_PROFILE_DEFAULT,
        out_session);
}

ForgeResult
forge_session_create_with_profile(
    ForgeArtifact            *artifact,
ForgeSessionProfileKind   profile_kind,
ForgeSession            **out_session)
{
    ForgeSession *session;
    ForgeEffectiveProfile profile;
    ForgeBuildCapabilitySet build;
    ForgeProductExposureProfile product;
    ForgeSessionRestrictionProfile session_profile;
    ForgeResult session_result;

    if (!out_session)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_session_create: out_session is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_session_create: artifact is NULL");
    }

    switch (profile_kind)
    {
        case FORGE_SESSION_PROFILE_DEFAULT:
        case FORGE_SESSION_PROFILE_COMMON_ONLY:
        case FORGE_SESSION_PROFILE_NO_PROBES:
            break;
        default:
            return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
                "forge_session_create_with_profile: invalid session profile kind");
    }

    *out_session = NULL;

    session = (ForgeSession *)malloc(sizeof(ForgeSession));

    if (!session)
    {
        return forge_fail(FORGE_ERR_INTERNAL,
            "forge_session_create: allocation failed");
    }

    memset(session, 0, sizeof(*session));
    session->artifact = artifact;
    session->placeholder_state = 0;
    forge_policy_get_build_capabilities(&build);
    forge_policy_get_installed_product_profile(&product);
    forge_policy_build_session_profile_kind(profile_kind, &session_profile);

    if (!forge_policy_session_profile_is_valid(&build, &product, &session_profile))
    {
        free(session);
        return forge_fail(FORGE_ERR_FORBIDDEN,
            "forge_session_create_with_profile: session profile exceeds installed product policy");
    }

    forge_policy_install_session_profile(&session_profile);
    forge_policy_get_session_effective_profile(&profile);

    if (!forge_policy_backend_visible(&profile, artifact->backend_id))
    {
        free(session);
        return forge_fail(FORGE_ERR_FORBIDDEN,
            "forge_session_create_with_profile: backend is denied by active session policy");
    }

    session->effective_profile = profile;

    session_result = forge_session_prepare_runtime_state(session);
    if (session_result != FORGE_OK)
    {
        forge_session_release_runtime_state(session);
        free(session);
        return session_result;
    }

    *out_session = session;

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_session_info(
    const ForgeSession *session,
    ForgeSessionInfo   *out_info)
{
    if (!out_info)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_session_info: out_info is NULL");
    }

    if (!session)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_session_info: session is NULL");
    }

    out_info->backend_id = session->artifact->backend_id;
    out_info->lifecycle_state = FORGE_SESSION_STATE_READY;
    out_info->placeholder_state = session->placeholder_state;

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_apply_inputs(
    ForgeSession *session,
    const ForgeSignalValue *values,
    uint32_t count)
{
    uint32_t index;
    uint32_t descriptor_index;
    uint8_t *seen;

    if (!session)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_apply_inputs: session is NULL");
    }

    if (!values || count == 0)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_apply_inputs: values is NULL or count is zero");
    }

    if (!session->effective_profile.allow_common_inputs)
    {
        return forge_fail(FORGE_ERR_FORBIDDEN,
            "forge_apply_inputs: common input submission denied by active profile");
    }

    if (!session->real_fast_runtime)
    {
        return forge_fail(FORGE_ERR_UNSUPPORTED,
            "forge_apply_inputs: runtime input mapping not yet implemented");
    }

    if (count != session->input_value_count)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_apply_inputs: input count does not match descriptor count");
    }

    seen = NULL;
    if (session->input_value_count != 0u)
    {
        seen = (uint8_t *)calloc(session->input_value_count, sizeof(uint8_t));
        if (!seen)
        {
            return forge_fail(FORGE_ERR_INTERNAL,
                "forge_apply_inputs: input tracking allocation failed");
        }
    }

    for (index = 0u; index < count; ++index)
    {
        if (values[index].value < FORGE_LOGIC_0 ||
            values[index].value > FORGE_LOGIC_Z)
        {
            free(seen);
            return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
                "forge_apply_inputs: input value is not a supported common logic value");
        }

        descriptor_index = 0u;
        while (descriptor_index < session->input_value_count)
        {
            if (session->input_values[descriptor_index].signal_id == values[index].signal_id)
            {
                break;
            }

            ++descriptor_index;
        }

        if (descriptor_index >= session->input_value_count)
        {
            free(seen);
            return forge_fail(FORGE_ERR_OUT_OF_BOUNDS,
                "forge_apply_inputs: input signal_id not found");
        }

        if (seen && seen[descriptor_index] != 0u)
        {
            free(seen);
            return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
                "forge_apply_inputs: duplicate input signal_id");
        }

        session->input_values[descriptor_index].value = values[index].value;
        if (seen)
        {
            seen[descriptor_index] = 1u;
        }
    }

    for (index = 0u; index < session->input_value_count; ++index)
    {
        if (!seen || seen[index] == 0u)
        {
            free(seen);
            return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
                "forge_apply_inputs: missing required input signal");
        }
    }

    free(seen);
    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_step(
    ForgeSession *session,
    uint32_t step_count)
{
    uint32_t index;
    ForgeResult result;

    if (!session)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_step: session is NULL");
    }

    if (step_count == 0)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_step: step_count is zero");
    }

    if (!session->effective_profile.allow_advanced_controls &&
        step_count > 1u)
    {
        return forge_fail(FORGE_ERR_FORBIDDEN,
            "forge_step: advanced stepping denied by active profile");
    }

    if (!session->real_fast_runtime)
    {
        return forge_fail(FORGE_ERR_UNSUPPORTED,
            "forge_step: runtime advancement not yet implemented");
    }

    result = FORGE_OK;
    for (index = 0u; index < step_count; ++index)
    {
        result = forge_session_execute_real_fast(session);
        if (result != FORGE_OK)
        {
            return result;
        }
    }

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_read_outputs(
    const ForgeSession *session,
    ForgeSignalValue   *values,
    uint32_t count)
{
    uint32_t index;

    if (!session)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_read_outputs: session is NULL");
    }

    if (!values || count == 0)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_read_outputs: values is NULL or count is zero");
    }

    if (!session->effective_profile.allow_common_observation)
    {
        return forge_fail(FORGE_ERR_FORBIDDEN,
            "forge_read_outputs: common observation denied by active profile");
    }

    if (!session->real_fast_runtime)
    {
        return forge_fail(FORGE_ERR_UNSUPPORTED,
            "forge_read_outputs: runtime output mapping not yet implemented");
    }

    if (count > session->output_value_count)
    {
        return forge_fail(FORGE_ERR_OUT_OF_BOUNDS,
            "forge_read_outputs: output count exceeds descriptor count");
    }

    if (count != session->output_value_count)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_read_outputs: output count does not match descriptor count");
    }

    if (!session->artifact || !session->artifact->descriptors)
    {
        return forge_fail(FORGE_ERR_INTERNAL,
            "forge_read_outputs: output descriptor layout unavailable");
    }

    for (index = 0u; index < count; ++index)
    {
        if (session->output_values[index].signal_id !=
            session->artifact->descriptors[session->artifact->input_descriptor_count + index].id)
        {
            return forge_fail(FORGE_ERR_INTERNAL,
                "forge_read_outputs: output signal mapping inconsistent");
        }

        values[index] = session->output_values[index];
    }

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_input_descriptor_count(
    const ForgeArtifact *artifact,
    uint32_t            *out_count)
{
    if (!out_count)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_input_descriptor_count: out_count is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_input_descriptor_count: artifact is NULL");
    }

    return forge_descriptor_filtered_count(
        &artifact->effective_profile,
        FORGE_DESCRIPTOR_CLASS_INPUT,
        artifact->input_descriptor_count,
        out_count,
        "forge_input_descriptor_count: out_count is NULL");
}

ForgeResult
forge_input_descriptor_at(
    const ForgeArtifact *artifact,
    uint32_t             index,
    ForgeDescriptor     *out_descriptor)
{
    if (!out_descriptor)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_input_descriptor_at: out_descriptor is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_input_descriptor_at: artifact is NULL");
    }

    return forge_descriptor_filtered_at(
        &artifact->effective_profile,
        forge_artifact_input_descriptors(artifact),
        artifact->input_descriptor_count,
        index,
        out_descriptor,
        "forge_input_descriptor_at: out_descriptor is NULL",
        "forge_input_descriptor_at: index out of bounds");
}

ForgeResult
forge_input_descriptor_by_id(
    const ForgeArtifact *artifact,
    uint32_t             descriptor_id,
    ForgeDescriptor     *out_descriptor)
{
    if (!out_descriptor)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_input_descriptor_by_id: out_descriptor is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_input_descriptor_by_id: artifact is NULL");
    }

    return forge_descriptor_find_by_id(
        &artifact->effective_profile,
        forge_artifact_input_descriptors(artifact),
        artifact->input_descriptor_count,
        descriptor_id,
        out_descriptor,
        "forge_input_descriptor_by_id: out_descriptor is NULL",
        "forge_input_descriptor_by_id: descriptor_id not found");
}

ForgeResult
forge_input_descriptor_by_name(
    const ForgeArtifact *artifact,
    const char          *name,
    ForgeDescriptor     *out_descriptor)
{
    if (!name || !out_descriptor)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_input_descriptor_by_name: name or out_descriptor is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_input_descriptor_by_name: artifact is NULL");
    }

    return forge_descriptor_find_by_name(
        &artifact->effective_profile,
        forge_artifact_input_descriptors(artifact),
        artifact->input_descriptor_count,
        name,
        out_descriptor,
        "forge_input_descriptor_by_name: name or out_descriptor is NULL",
        "forge_input_descriptor_by_name: descriptor name not found");
}

ForgeResult
forge_output_descriptor_count(
    const ForgeArtifact *artifact,
    uint32_t            *out_count)
{
    if (!out_count)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_output_descriptor_count: out_count is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_output_descriptor_count: artifact is NULL");
    }

    return forge_descriptor_filtered_count(
        &artifact->effective_profile,
        FORGE_DESCRIPTOR_CLASS_OUTPUT,
        artifact->output_descriptor_count,
        out_count,
        "forge_output_descriptor_count: out_count is NULL");
}

ForgeResult
forge_output_descriptor_at(
    const ForgeArtifact *artifact,
    uint32_t             index,
    ForgeDescriptor     *out_descriptor)
{
    if (!out_descriptor)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_output_descriptor_at: out_descriptor is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_output_descriptor_at: artifact is NULL");
    }

    return forge_descriptor_filtered_at(
        &artifact->effective_profile,
        forge_artifact_output_descriptors(artifact),
        artifact->output_descriptor_count,
        index,
        out_descriptor,
        "forge_output_descriptor_at: out_descriptor is NULL",
        "forge_output_descriptor_at: index out of bounds");
}

ForgeResult
forge_output_descriptor_by_id(
    const ForgeArtifact *artifact,
    uint32_t             descriptor_id,
    ForgeDescriptor     *out_descriptor)
{
    if (!out_descriptor)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_output_descriptor_by_id: out_descriptor is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_output_descriptor_by_id: artifact is NULL");
    }

    return forge_descriptor_find_by_id(
        &artifact->effective_profile,
        forge_artifact_output_descriptors(artifact),
        artifact->output_descriptor_count,
        descriptor_id,
        out_descriptor,
        "forge_output_descriptor_by_id: out_descriptor is NULL",
        "forge_output_descriptor_by_id: descriptor_id not found");
}

ForgeResult
forge_output_descriptor_by_name(
    const ForgeArtifact *artifact,
    const char          *name,
    ForgeDescriptor     *out_descriptor)
{
    if (!name || !out_descriptor)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_output_descriptor_by_name: name or out_descriptor is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_output_descriptor_by_name: artifact is NULL");
    }

    return forge_descriptor_find_by_name(
        &artifact->effective_profile,
        forge_artifact_output_descriptors(artifact),
        artifact->output_descriptor_count,
        name,
        out_descriptor,
        "forge_output_descriptor_by_name: name or out_descriptor is NULL",
        "forge_output_descriptor_by_name: descriptor name not found");
}

ForgeResult
forge_probe_descriptor_count(
    const ForgeArtifact *artifact,
    uint32_t            *out_count)
{
    if (!out_count)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_probe_descriptor_count: out_count is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_probe_descriptor_count: artifact is NULL");
    }

    return forge_descriptor_filtered_count(
        &artifact->effective_profile,
        FORGE_DESCRIPTOR_CLASS_PROBE,
        artifact->probe_descriptor_count,
        out_count,
        "forge_probe_descriptor_count: out_count is NULL");
}

ForgeResult
forge_probe_descriptor_at(
    const ForgeArtifact *artifact,
    uint32_t             index,
    ForgeDescriptor     *out_descriptor)
{
    if (!out_descriptor)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_probe_descriptor_at: out_descriptor is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_probe_descriptor_at: artifact is NULL");
    }

    return forge_descriptor_filtered_at(
        &artifact->effective_profile,
        forge_artifact_probe_descriptors(artifact),
        artifact->probe_descriptor_count,
        index,
        out_descriptor,
        "forge_probe_descriptor_at: out_descriptor is NULL",
        "forge_probe_descriptor_at: index out of bounds");
}

ForgeResult
forge_probe_descriptor_by_id(
    const ForgeArtifact *artifact,
    uint32_t             descriptor_id,
    ForgeDescriptor     *out_descriptor)
{
    if (!out_descriptor)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_probe_descriptor_by_id: out_descriptor is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_probe_descriptor_by_id: artifact is NULL");
    }

    return forge_descriptor_find_by_id(
        &artifact->effective_profile,
        forge_artifact_probe_descriptors(artifact),
        artifact->probe_descriptor_count,
        descriptor_id,
        out_descriptor,
        "forge_probe_descriptor_by_id: out_descriptor is NULL",
        "forge_probe_descriptor_by_id: descriptor_id not found");
}

ForgeResult
forge_probe_descriptor_by_name(
    const ForgeArtifact *artifact,
    const char          *name,
    ForgeDescriptor     *out_descriptor)
{
    if (!name || !out_descriptor)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_probe_descriptor_by_name: name or out_descriptor is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_probe_descriptor_by_name: artifact is NULL");
    }

    return forge_descriptor_find_by_name(
        &artifact->effective_profile,
        forge_artifact_probe_descriptors(artifact),
        artifact->probe_descriptor_count,
        name,
        out_descriptor,
        "forge_probe_descriptor_by_name: name or out_descriptor is NULL",
        "forge_probe_descriptor_by_name: descriptor name not found");
}

ForgeResult
forge_structure_component_count(
    const ForgeArtifact *artifact,
    uint32_t            *out_count)
{
    if (!out_count)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_structure_component_count: out_count is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_structure_component_count: artifact is NULL");
    }

    *out_count = artifact->structure_component_count;
    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_structure_component_at(
    const ForgeArtifact      *artifact,
    uint32_t                  index,
    ForgeStructureComponent  *out_component)
{
    if (!out_component)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_structure_component_at: out_component is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_structure_component_at: artifact is NULL");
    }

    if (index >= artifact->structure_component_count)
    {
        return forge_fail(FORGE_ERR_OUT_OF_BOUNDS,
            "forge_structure_component_at: index out of bounds");
    }

    *out_component = forge_structure_component_from_serialized(
        &artifact->components[index]);
    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_structure_component_by_id(
    const ForgeArtifact      *artifact,
    uint64_t                  component_id,
    ForgeStructureComponent  *out_component)
{
    uint32_t index;

    if (!out_component)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_structure_component_by_id: out_component is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_structure_component_by_id: artifact is NULL");
    }

    for (index = 0u; index < artifact->structure_component_count; ++index)
    {
        if (artifact->components[index].id == component_id)
        {
            *out_component = forge_structure_component_from_serialized(
                &artifact->components[index]);
            forge_diag_set("");
            return FORGE_OK;
        }
    }

    return forge_fail(FORGE_ERR_OUT_OF_BOUNDS,
        "forge_structure_component_by_id: component_id not found");
}

ForgeResult
forge_structure_connection_count(
    const ForgeArtifact *artifact,
    uint32_t            *out_count)
{
    if (!out_count)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_structure_connection_count: out_count is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_structure_connection_count: artifact is NULL");
    }

    *out_count = artifact->structure_connection_count;
    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_structure_connection_at(
    const ForgeArtifact       *artifact,
    uint32_t                   index,
    ForgeStructureConnection  *out_connection)
{
    if (!out_connection)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_structure_connection_at: out_connection is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_structure_connection_at: artifact is NULL");
    }

    if (index >= artifact->structure_connection_count)
    {
        return forge_fail(FORGE_ERR_OUT_OF_BOUNDS,
            "forge_structure_connection_at: index out of bounds");
    }

    *out_connection = forge_structure_connection_from_serialized(
        &artifact->connections[index]);
    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_read_probes(
    const ForgeSession *session,
    ForgeProbeValue    *values,
    uint32_t            count)
{
    if (!session)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_read_probes: session is NULL");
    }

    if (!values || count == 0)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_read_probes: values is NULL or count is zero");
    }

    if (!session->effective_profile.allow_common_probes)
    {
        return forge_fail(FORGE_ERR_FORBIDDEN,
            "forge_read_probes: probe reads denied by active profile");
    }

    return forge_fail(FORGE_ERR_UNSUPPORTED,
        "forge_read_probes: runtime probe mapping not yet implemented");
}

ForgeResult
forge_session_reset(ForgeSession *session)
{
    if (!session)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_session_reset: session is NULL");
    }

    session->placeholder_state = 0;
    forge_session_reset_runtime_state(session);

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_session_free(ForgeSession *session)
{
    if (!session)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_session_free: session is NULL");
    }

    forge_session_release_runtime_state(session);
    free(session);

    forge_diag_set("");
    return FORGE_OK;
}

/* -------------------------------------------------------------------------
 * Diagnostics
 * ------------------------------------------------------------------------- */

const char *
forge_last_error_string(void)
{
    return forge_diag_get();
}
