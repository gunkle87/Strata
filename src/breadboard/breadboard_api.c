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

    /* Allocate and populate hardcoded placeholder descriptors to prove plumbing */
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

    /* Copy target expectation and admission logic to the draft */
    draft->target = module->target;
    draft->info.target = module->target;
    draft->info.has_placeholders = true;
    draft->info.approximate_size_bytes = 1024; /* Fake size */
    draft->admission_info.target = module->target;
    draft->admission_info.is_placeholder = true;
    draft->admission_info.approximate_size_bytes = 1024; /* Fake size */

    if (module->target == BREADBOARD_TARGET_FAST_4STATE)
    {
        draft->admission_info.requires_advanced_controls = false;
        draft->admission_info.native_only_behavior = false;
        draft->admission_info.extension_flags = 0;
    }
    else if (module->target == BREADBOARD_TARGET_TEMPORAL)
    {
        draft->admission_info.requires_advanced_controls = true;
        draft->admission_info.native_only_behavior = true;
        draft->admission_info.extension_flags = 1;
    }
    else
    {
        draft->admission_info.requires_advanced_controls = false;
        draft->admission_info.native_only_behavior = false;
        draft->admission_info.extension_flags = 0;
    }

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
        if (draft->inputs) free(draft->inputs);
        if (draft->outputs) free(draft->outputs);
        if (draft->probes) free(draft->probes);
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

    *out_size = strata_placeholder_artifact_size();
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

    if (!draft || !buffer || !out_size)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (!draft->admission_info.is_placeholder)
    {
        return BREADBOARD_ERR_UNSUPPORTED;
    }

    required_size = strata_placeholder_artifact_size();

    if (buffer_size < required_size)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    if (resolve_placeholder_backend_id(draft->target, &target_backend_id) != BREADBOARD_OK)
    {
        return BREADBOARD_ERR_UNSUPPORTED;
    }

    if (draft->admission_info.requires_advanced_controls)
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

    if (!strata_placeholder_artifact_write(
        buffer,
        buffer_size,
        target_backend_id,
        payload_kind,
        &admission_info,
        out_size))
    {
        return BREADBOARD_ERR_INTERNAL;
    }

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
