#ifndef BREADBOARD_INTERNAL_H
#define BREADBOARD_INTERNAL_H

#include "breadboard_types.h"
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

    /* Diagnostic storage block */
    size_t diagnostic_capacity;
    size_t diagnostic_count;
    BreadboardDiagnostic* diagnostics;

    /* Future: structure graph blocks, normalization context, etc. */
};

struct BreadboardArtifactDraft
{
    BreadboardTarget target;
    BreadboardDraftInfo info;

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
