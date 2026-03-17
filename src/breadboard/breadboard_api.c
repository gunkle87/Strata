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
        /* Future: free diagnostic allocations and structural lists */
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

BreadboardResult breadboard_module_compile(
    BreadboardModule* module,
    const BreadboardCompileOptions* options,
    BreadboardArtifactDraft** out_draft)
{
    if (!module || !out_draft)
    {
        return BREADBOARD_ERR_INVALID_ARGUMENT;
    }

    /* We are a skeleton. We cannot actually compile structure yet. */
    if (!options || !options->allow_placeholders)
    {
        return BREADBOARD_ERR_UNSUPPORTED;
    }

    /* If placeholders are allowed, return an empty dummy draft to establish plumbing. */
    BreadboardArtifactDraft* draft = (BreadboardArtifactDraft*)calloc(1, sizeof(BreadboardArtifactDraft));
    if (!draft)
    {
        return BREADBOARD_ERR_INTERNAL;
    }

    /* Copy target expectation to the draft */
    draft->target = module->target;

    *out_draft = draft;
    return BREADBOARD_OK;
}

void breadboard_artifact_draft_free(BreadboardArtifactDraft* draft)
{
    if (draft)
    {
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
        return BREADBOARD_ERR_UNSUPPORTED; /* or BREADBOARD_ERR_INVALID_ARGUMENT / out-of-bounds */
    }

    *out_diagnostic = module->diagnostics[index];
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
