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
 * breadboard_module_get_target
 *
 * Retrieves the currently selected target for the compilation module.
 */
BreadboardResult breadboard_module_get_target(
    const BreadboardModule* module,
    BreadboardTarget* out_target);

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
 * breadboard_module_get_last_diagnostic
 *
 * Retrieves the most recently emitted diagnostic from the compiler module.
 * Returns BREADBOARD_ERR_UNSUPPORTED if no diagnostics exist.
 */
BreadboardResult breadboard_module_get_last_diagnostic(
    const BreadboardModule* module,
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

/*
 * breadboard_artifact_draft_query_info
 *
 * Retrieves structural metadata about the generated draft.
 */
BreadboardResult breadboard_artifact_draft_query_info(
    const BreadboardArtifactDraft* draft,
    BreadboardDraftInfo* out_info);

/*
 * breadboard_module_query_target_info
 *
 * Retrieves information about the current target capabilities.
 */
BreadboardResult breadboard_module_query_target_info(
    const BreadboardModule* module,
    BreadboardTargetInfo* out_info);

/*
 * breadboard_draft_input_descriptor_count
 *
 * Retrieves the total number of input descriptors exported by the draft.
 */
BreadboardResult breadboard_draft_input_descriptor_count(
    const BreadboardArtifactDraft* draft,
    size_t* out_count);

/*
 * breadboard_draft_input_descriptor_at
 *
 * Retrieves a specific input descriptor exported by the draft by index.
 */
BreadboardResult breadboard_draft_input_descriptor_at(
    const BreadboardArtifactDraft* draft,
    size_t index,
    BreadboardDescriptor* out_descriptor);

/*
 * breadboard_draft_output_descriptor_count
 *
 * Retrieves the total number of output descriptors exported by the draft.
 */
BreadboardResult breadboard_draft_output_descriptor_count(
    const BreadboardArtifactDraft* draft,
    size_t* out_count);

/*
 * breadboard_draft_output_descriptor_at
 *
 * Retrieves a specific output descriptor exported by the draft by index.
 */
BreadboardResult breadboard_draft_output_descriptor_at(
    const BreadboardArtifactDraft* draft,
    size_t index,
    BreadboardDescriptor* out_descriptor);

/*
 * breadboard_draft_probe_descriptor_count
 *
 * Retrieves the total number of probe descriptors exported by the draft.
 */
BreadboardResult breadboard_draft_probe_descriptor_count(
    const BreadboardArtifactDraft* draft,
    size_t* out_count);

/*
 * breadboard_draft_probe_descriptor_at
 *
 * Retrieves a specific probe descriptor exported by the draft by index.
 */
BreadboardResult breadboard_draft_probe_descriptor_at(
    const BreadboardArtifactDraft* draft,
    size_t index,
    BreadboardDescriptor* out_descriptor);

/*
 * breadboard_draft_input_descriptor_by_id
 *
 * Retrieves a specific input descriptor exported by the draft by exact ID.
 */
BreadboardResult breadboard_draft_input_descriptor_by_id(
    const BreadboardArtifactDraft* draft,
    uint64_t id,
    BreadboardDescriptor* out_descriptor);

/*
 * breadboard_draft_input_descriptor_by_name
 *
 * Retrieves a specific input descriptor exported by the draft by exact name.
 */
BreadboardResult breadboard_draft_input_descriptor_by_name(
    const BreadboardArtifactDraft* draft,
    const char* name,
    BreadboardDescriptor* out_descriptor);

/*
 * breadboard_draft_output_descriptor_by_id
 *
 * Retrieves a specific output descriptor exported by the draft by exact ID.
 */
BreadboardResult breadboard_draft_output_descriptor_by_id(
    const BreadboardArtifactDraft* draft,
    uint64_t id,
    BreadboardDescriptor* out_descriptor);

/*
 * breadboard_draft_output_descriptor_by_name
 *
 * Retrieves a specific output descriptor exported by the draft by exact name.
 */
BreadboardResult breadboard_draft_output_descriptor_by_name(
    const BreadboardArtifactDraft* draft,
    const char* name,
    BreadboardDescriptor* out_descriptor);

/*
 * breadboard_draft_probe_descriptor_by_id
 *
 * Retrieves a specific probe descriptor exported by the draft by exact ID.
 */
BreadboardResult breadboard_draft_probe_descriptor_by_id(
    const BreadboardArtifactDraft* draft,
    uint64_t id,
    BreadboardDescriptor* out_descriptor);

/*
 * breadboard_draft_probe_descriptor_by_name
 *
 * Retrieves a specific probe descriptor exported by the draft by exact name.
 */
BreadboardResult breadboard_draft_probe_descriptor_by_name(
    const BreadboardArtifactDraft* draft,
    const char* name,
    BreadboardDescriptor* out_descriptor);

#ifdef __cplusplus
}
#endif

#endif /* BREADBOARD_API_H */
