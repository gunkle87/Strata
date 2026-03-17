#ifndef BREADBOARD_API_H
#define BREADBOARD_API_H

#include "breadboard_types.h"
#include "breadboard_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * breadboard_api.h
 *
 * Public interface for the Breadboard structural compiler.
 * This boundary defines how structure is ingested, targeted, and lowered into
 * an executable artifact draft.
 */

/*
 * breadboard_module_create
 *
 * Allocates a new empty module for structural compilation.
 */
BreadboardResult breadboard_module_create(BreadboardModule** out_module);

/*
 * breadboard_module_free
 *
 * Releases a module and any uncompiled structural state.
 */
void breadboard_module_free(BreadboardModule* module);

/*
 * breadboard_module_set_target
 *
 * Specifies the target backend class for subsequent compilation constraints.
 */
BreadboardResult breadboard_module_set_target(
    BreadboardModule* module,
    BreadboardTarget target);

/*
 * breadboard_module_compile
 *
 * Compiles the accumulated structure targeting the specified backend restrictions,
 * optionally emitting placeholder structures based on the provided options.
 *
 * On success, produces a BreadboardArtifactDraft ready for inspection or handoff.
 */
BreadboardResult breadboard_module_compile(
    BreadboardModule* module,
    const BreadboardCompileOptions* options,
    BreadboardArtifactDraft** out_draft);

/*
 * breadboard_artifact_draft_free
 *
 * Releases an artifact draft.
 */
void breadboard_artifact_draft_free(BreadboardArtifactDraft* draft);

/*
 * breadboard_module_get_diagnostic_count
 *
 * Retrieve the number of diagnostics emitted during the current session.
 */
BreadboardResult breadboard_module_get_diagnostic_count(
    const BreadboardModule* module,
    size_t* out_count);

/*
 * breadboard_module_get_diagnostic
 *
 * Retrieves a specific diagnostic from the compiler module by index.
 */
BreadboardResult breadboard_module_get_diagnostic(
    const BreadboardModule* module,
    size_t index,
    BreadboardDiagnostic* out_diagnostic);

/*
 * breadboard_artifact_draft_query_metadata
 *
 * Queries basic target validation metadata from an artifact draft to verify
 * targeting intent before committing it to Forge.
 */
BreadboardResult breadboard_artifact_draft_query_metadata(
    const BreadboardArtifactDraft* draft,
    BreadboardTarget* out_target);

#ifdef __cplusplus
}
#endif

#endif /* BREADBOARD_API_H */
