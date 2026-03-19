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
 * breadboard_module_set_target_policy
 *
 * Specifies the allowed targets for this compilation session according to
 * build/product policy. Targets outside this mask will be rejected explicitly.
 */
BreadboardResult breadboard_module_set_target_policy(
    BreadboardModule* module,
    BreadboardTargetMask allowed_mask);

/*
 * breadboard_module_query_target_availability
 *
 * Queries whether the given target is allowed under the current target policy.
 */
BreadboardResult breadboard_module_query_target_availability(
    const BreadboardModule* module,
    BreadboardTarget target,
    bool* out_is_available);

/*
 * breadboard_module_set_identity
 *
 * Sets optional authored source identity metadata on the module. This source
 * metadata can be propagated into draft summaries and temporary export
 * handoff artifacts.
 */
BreadboardResult breadboard_module_set_identity(
    BreadboardModule* module,
    const BreadboardModuleIdentity* identity);

/*
 * breadboard_module_set_requirement_profile
 *
 * Sets an optional coarse authored requirement profile for the module. This
 * profile feeds draft admission metadata and the temporary placeholder handoff
 * payload class.
 */
BreadboardResult breadboard_module_set_requirement_profile(
    BreadboardModule* module,
    const BreadboardRequirementProfile* profile);

/*
 * breadboard_module_set_structure_summary
 *
 * Sets optional coarse authored structural summary metadata on the module.
 * This remains source-side scaffolding only and is propagated through draft
 * info and the temporary placeholder export path.
 */
BreadboardResult breadboard_module_set_structure_summary(
    BreadboardModule* module,
    const BreadboardStructureSummary* summary);

/*
 * breadboard_module_add_component_instance
 *
 * Declares a minimal authored structural component instance on the module.
 * Current scaffolding uses these declarations only to derive coarse structure
 * summary counts.
 */
BreadboardResult breadboard_module_add_component_instance(
    BreadboardModule* module,
    const BreadboardComponentSpec* spec);

/*
 * breadboard_module_add_connection
 *
 * Declares a minimal authored structural connection on the module. Current
 * scaffolding uses these declarations only to derive coarse structure summary
 * counts and basic authored topology presence.
 */
BreadboardResult breadboard_module_add_connection(
    BreadboardModule* module,
    const BreadboardConnectionSpec* spec);

/*
 * breadboard_module_add_executable_connection
 *
 * Declares an endpoint-aware connection intended for admitted executable
 * subset legality assessment. This does not lower or execute anything yet.
 */
BreadboardResult breadboard_module_add_executable_connection(
    BreadboardModule* module,
    const BreadboardExecutableConnectionSpec* spec);

/*
 * breadboard_module_add_input_descriptor
 *
 * Declares a draft-visible input descriptor on the module.
 */
BreadboardResult breadboard_module_add_input_descriptor(
    BreadboardModule* module,
    const BreadboardDescriptorSpec* spec);

/*
 * breadboard_module_add_output_descriptor
 *
 * Declares a draft-visible output descriptor on the module.
 */
BreadboardResult breadboard_module_add_output_descriptor(
    BreadboardModule* module,
    const BreadboardDescriptorSpec* spec);

/*
 * breadboard_module_add_probe_descriptor
 *
 * Declares a draft-visible probe descriptor on the module.
 */
BreadboardResult breadboard_module_add_probe_descriptor(
    BreadboardModule* module,
    const BreadboardDescriptorSpec* spec);

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
 * Retrieves coarse structural metadata about the generated draft.
 */
BreadboardResult breadboard_artifact_draft_query_info(
    const BreadboardArtifactDraft* draft,
    BreadboardDraftInfo* out_info);

/*
 * breadboard_draft_query_admission_info
 *
 * Retrieves admission-oriented metadata about the generated draft.
 */
BreadboardResult breadboard_draft_query_admission_info(
    const BreadboardArtifactDraft* draft,
    BreadboardDraftAdmissionInfo* out_info);

/*
 * breadboard_artifact_draft_export_placeholder_size
 *
 * Returns the exact byte size required to export the current placeholder draft
 * into the temporary Forge-compatible stub artifact format.
 */
BreadboardResult breadboard_artifact_draft_export_placeholder_size(
    const BreadboardArtifactDraft* draft,
    size_t* out_size);

/*
 * breadboard_artifact_draft_export_placeholder
 *
 * Exports the current placeholder draft into the temporary Forge-compatible
 * stub artifact format using the backend identity implied by the draft target
 * and the current temporary handoff contract.
 *
 * This is a temporary handoff path for scaffolding and tests. It is not the
 * final Strata artifact format.
 */
BreadboardResult breadboard_artifact_draft_export_placeholder(
    const BreadboardArtifactDraft* draft,
    void* buffer,
    size_t buffer_size,
    size_t* out_size);

/*
 * breadboard_artifact_draft_export_fast
 *
 * Exports a real FAST_4STATE draft into the same temporary artifact envelope
 * using the admitted fast-path payload kind.
 *
 * This is still temporary scaffolding, but it carries the lowered executable
 * descriptor truth instead of placeholder fallback content.
 */
BreadboardResult breadboard_artifact_draft_export_fast(
    const BreadboardArtifactDraft* draft,
    void* buffer,
    size_t buffer_size,
    size_t* out_size);

BreadboardResult breadboard_artifact_draft_export_fast_size(
    const BreadboardArtifactDraft* draft,
    size_t* out_size);

/*
 * breadboard_draft_component_count
 *
 * Retrieves the total number of authored structural components retained on the
 * draft.
 */
BreadboardResult breadboard_draft_component_count(
    const BreadboardArtifactDraft* draft,
    size_t* out_count);

/*
 * breadboard_draft_component_at
 *
 * Retrieves a specific authored structural component retained on the draft by
 * index.
 */
BreadboardResult breadboard_draft_component_at(
    const BreadboardArtifactDraft* draft,
    size_t index,
    BreadboardComponent* out_component);

/*
 * breadboard_draft_component_by_id
 *
 * Retrieves a specific authored structural component retained on the draft by
 * authored component ID.
 */
BreadboardResult breadboard_draft_component_by_id(
    const BreadboardArtifactDraft* draft,
    uint64_t component_id,
    BreadboardComponent* out_component);

/*
 * breadboard_draft_connection_count
 *
 * Retrieves the total number of authored structural connections retained on
 * the draft.
 */
BreadboardResult breadboard_draft_connection_count(
    const BreadboardArtifactDraft* draft,
    size_t* out_count);

/*
 * breadboard_draft_connection_at
 *
 * Retrieves a specific authored structural connection retained on the draft by
 * index.
 */
BreadboardResult breadboard_draft_connection_at(
    const BreadboardArtifactDraft* draft,
    size_t index,
    BreadboardConnection* out_connection);

/*
 * breadboard_module_query_target_info
 *
 * Retrieves information about the current target capabilities.
 */
BreadboardResult breadboard_module_query_target_info(
    const BreadboardModule* module,
    BreadboardTargetInfo* out_info);

/*
 * breadboard_query_executable_subset_info
 *
 * Queries the frozen admitted executable subset contract for a target. This
 * allows later implementation and audit work to use one stable subset truth.
 */
BreadboardResult breadboard_query_executable_subset_info(
    BreadboardTarget target,
    BreadboardExecutableSubsetInfo* out_info);

/*
 * breadboard_query_primitive_signature
 *
 * Queries the frozen primitive signature for one admitted primitive kind on a
 * target-specific executable subset contract.
 */
BreadboardResult breadboard_query_primitive_signature(
    BreadboardTarget target,
    BreadboardPrimitiveKind primitive_kind,
    BreadboardPrimitiveSignature* out_signature);

/*
 * breadboard_module_assess_executable_subset
 *
 * Assesses whether the current authored module is executable, placeholder-only,
 * or structurally invalid for the first admitted executable subset.
 */
BreadboardResult breadboard_module_assess_executable_subset(
    const BreadboardModule* module,
    BreadboardExecutableAssessment* out_assessment);

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
