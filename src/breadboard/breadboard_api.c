#include "breadboard_api.h"
#include "breadboard_internal.h"
#include <stdlib.h>

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

    if (module->target == BREADBOARD_TARGET_UNSPECIFIED)
    {
        record_diagnostic(module, BREADBOARD_DIAG_ERROR, BREADBOARD_DIAG_CODE_UNSUPPORTED_TARGET, "Cannot compile without a specified target");
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
    draft->input_count = 2;
    draft->inputs = (BreadboardDescriptor*)calloc(2, sizeof(BreadboardDescriptor));
    if (draft->inputs)
    {
        draft->inputs[0].id = 100;
        draft->inputs[0].name = "placeholder_in_0";
        draft->inputs[0].width = 1;
        draft->inputs[0].class_type = BREADBOARD_DESC_INPUT;
        draft->inputs[0].is_placeholder = true;

        draft->inputs[1].id = 101;
        draft->inputs[1].name = "placeholder_in_1";
        draft->inputs[1].width = 8;
        draft->inputs[1].class_type = BREADBOARD_DESC_INPUT;
        draft->inputs[1].is_placeholder = true;
    }

    draft->output_count = 2;
    draft->outputs = (BreadboardDescriptor*)calloc(2, sizeof(BreadboardDescriptor));
    if (draft->outputs)
    {
        draft->outputs[0].id = 200;
        draft->outputs[0].name = "placeholder_out_0";
        draft->outputs[0].width = 1;
        draft->outputs[0].class_type = BREADBOARD_DESC_OUTPUT;
        draft->outputs[0].is_placeholder = true;

        draft->outputs[1].id = 201;
        draft->outputs[1].name = "placeholder_out_1";
        draft->outputs[1].width = 32;
        draft->outputs[1].class_type = BREADBOARD_DESC_OUTPUT;
        draft->outputs[1].is_placeholder = true;
    }

    draft->probe_count = 1;
    draft->probes = (BreadboardDescriptor*)calloc(1, sizeof(BreadboardDescriptor));
    if (draft->probes)
    {
        draft->probes[0].id = 300;
        draft->probes[0].name = "placeholder_probe_0";
        draft->probes[0].width = 1;
        draft->probes[0].class_type = BREADBOARD_DESC_PROBE;
        draft->probes[0].is_placeholder = true;
    }

    /* Copy target expectation to the draft */
    draft->target = module->target;
    draft->info.target = module->target;
    draft->info.has_placeholders = true;
    draft->info.approximate_size_bytes = 1024; /* Fake size */

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
