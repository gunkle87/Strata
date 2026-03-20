#ifndef BREADBOARD_INTERNAL_H
#define BREADBOARD_INTERNAL_H

#include "breadboard_types.h"
#include "../../include/strata_placeholder_artifact.h"
#include <stddef.h>

/*
 * breadboard_internal.h
 *
 * Internal structures for the Breadboard compiler.
 * These are strictly hidden from the public API.
 */

struct BreadboardModule
{
    BreadboardTarget target;
    BreadboardTargetMask allowed_targets;
    uint64_t module_id;
    char module_name[STRATA_PLACEHOLDER_MODULE_NAME_CAPACITY];
    BreadboardRequirementProfile requirement_profile;
    bool has_requirement_profile;
    BreadboardStructureSummary structure_summary;
    bool has_structure_summary;
    BreadboardProjectionPolicy projection_policy;
    bool has_projection_policy;
    size_t component_count;
    size_t stateful_component_count;
    BreadboardComponent* components;
    size_t connection_count;
    BreadboardConnection* connections;
    size_t executable_connection_count;
    BreadboardExecutableConnectionSpec* executable_connections;

    size_t input_count;
    BreadboardDescriptor* inputs;

    size_t output_count;
    BreadboardDescriptor* outputs;

    size_t probe_count;
    BreadboardDescriptor* probes;

    /* Diagnostic storage block */
    size_t diagnostic_capacity;
    size_t diagnostic_count;
    BreadboardDiagnostic* diagnostics;

    /* Future: structure graph blocks, normalization context, etc. */
};

struct BreadboardArtifactDraft
{
    BreadboardTarget target;
    uint64_t source_module_id;
    char source_module_name[STRATA_PLACEHOLDER_MODULE_NAME_CAPACITY];
    BreadboardStructureSummary structure_summary;
    size_t component_count;
    BreadboardComponent* components;
    size_t connection_count;
    BreadboardConnection* connections;
    size_t fast_signal_count;
    StrataPlaceholderFastSignalRecord* fast_signals;
    size_t fast_primitive_count;
    StrataPlaceholderFastPrimitiveRecord* fast_primitives;
    size_t fast_input_binding_count;
    StrataPlaceholderFastInputBinding* fast_input_bindings;
    size_t fast_output_binding_count;
    StrataPlaceholderFastOutputBinding* fast_output_bindings;
    BreadboardDraftInfo info;
    BreadboardDraftAdmissionInfo admission_info;

    /* Placeholder descriptors exposed when allow_placeholders was true */
    size_t input_count;
    BreadboardDescriptor* inputs;

    size_t output_count;
    BreadboardDescriptor* outputs;

    size_t probe_count;
    BreadboardDescriptor* probes;

    /* Future: target compatibility rules, real descriptor tables, payload block */
};

#endif /* BREADBOARD_INTERNAL_H */
