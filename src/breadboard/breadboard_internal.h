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

    /* Diagnostic storage block */
    size_t diagnostic_count;
    BreadboardDiagnostic* diagnostics;

    /* Future: structure graph blocks, normalization context, etc. */
};

struct BreadboardArtifactDraft
{
    BreadboardTarget target;

    /* Future: target compatibility rules, descriptor tables, payload block */
};

#endif /* BREADBOARD_INTERNAL_H */
