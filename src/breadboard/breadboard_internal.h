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
