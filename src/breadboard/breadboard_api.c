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
    BreadboardComponentInstance* components,
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
    BreadboardComponentInstance** out_components,
    const BreadboardComponentInstance* components,
    size_t count)
{
    BreadboardComponentInstance* copy;
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

    copy = (BreadboardComponentInstance*)calloc(count, sizeof(BreadboardComponentInstance));
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
    BreadboardComponentInstance* new_components;
    const char* kind_name_copy;

    if (!module || !spec || !spec->kind_name || spec->kind_name[0] == '\0')
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (module_has_component_id(module, spec->id))
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    new_components = (BreadboardComponentInstance*)realloc(
        module->components,
        (module->component_count + 1u) * sizeof(BreadboardComponentInstance));
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
    const BreadboardComponentInstance* components,
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

    /* We are a skeleton. We cannot actually compile structure yet. */
    if (!options || !options->allow_placeholders)
    {
        record_diagnostic(module, BREADBOARD_DIAG_ERROR, BREADBOARD_DIAG_CODE_UNSUPPORTED_CONSTRUCT, "Compilation requires allow_placeholders in the current skeleton limit");
        return BREADBOARD_ERR_COMPILE_FAILED;
    }

    /* If strict projection is demanded, emit a diagnostic noting that structural analysis is incomplete. */
    if (options->strict_projection || options->deny_approximation)
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
    draft->info.has_placeholders =
        (module->input_count == 0u && module->output_count == 0u && module->probe_count == 0u);
    draft->info.approximate_size_bytes = strata_placeholder_artifact_size_for_layout(
        (uint32_t)draft->input_count,
        (uint32_t)draft->output_count,
        (uint32_t)draft->probe_count,
        (uint32_t)draft->component_count,
        (uint32_t)draft->connection_count);
    draft->info.source_module_id = draft->source_module_id;
    draft->info.source_module_name = draft->source_module_name;
    draft->info.declared_component_count =
        draft->structure_summary.declared_component_count;
    draft->info.declared_connection_count =
        draft->structure_summary.declared_connection_count;
    draft->info.declared_stateful_node_count =
        draft->structure_summary.declared_stateful_node_count;
    draft->admission_info.target = module->target;
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

    /* Record a warning that placeholders were emitted */
    record_diagnostic(module, BREADBOARD_DIAG_WARNING, BREADBOARD_DIAG_CODE_NONE, "Draft emitted with placeholder structures");

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

BreadboardResult breadboard_module_query_target_info(
    const BreadboardModule* module,
    BreadboardTargetInfo* out_info)
{
    if (!module || !out_info)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    out_info->target = module->target;
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
