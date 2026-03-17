#ifndef BREADBOARD_TYPES_H
#define BREADBOARD_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
 * breadboard_types.h
 *
 * Opaque public types and data structures for the Breadboard compiler boundary.
 * These define the boundary contract without leaking structural compilation internals.
 */

/*
 * BreadboardTarget
 *
 * Identifies the target backend or execution class for structural compilation.
 */
typedef enum BreadboardTarget
{
    BREADBOARD_TARGET_UNSPECIFIED = 0,
    BREADBOARD_TARGET_FAST_4STATE = 1, /* Examples: LXS */
    BREADBOARD_TARGET_TEMPORAL    = 2  /* Examples: HighZ */
}
BreadboardTarget;

/*
 * BreadboardDiagnosticSeverity
 *
 * Severity level for compilation, recognition, and lowering diagnostics.
 */
typedef enum BreadboardDiagnosticSeverity
{
    BREADBOARD_DIAG_INFO    = 0,
    BREADBOARD_DIAG_WARNING = 1,
    BREADBOARD_DIAG_ERROR   = 2
}
BreadboardDiagnosticSeverity;

/*
 * BreadboardDiagnostic
 *
 * A single diagnostic message emitted during compilation.
 */
typedef struct BreadboardDiagnostic
{
    BreadboardDiagnosticSeverity severity;
    const char* message;
    /* Optional: source locus or stable ID references could be added here later */
}
BreadboardDiagnostic;

/*
 * BreadboardCompileOptions
 *
 * Configuration knobs for adjusting structural compilation behavior.
 */
typedef struct BreadboardCompileOptions
{
    /* If true, explicitly allow compilation even if some constructs are unsupported. */
    bool allow_placeholders;

    /* Other flags like strict_projection, deny_approximation, etc., can follow. */
}
BreadboardCompileOptions;

/*
 * BreadboardModule
 *
 * Opaque handle to the structural compilation environment.
 * Accepts structure, validates it, and tracks the compile state.
 */
typedef struct BreadboardModule BreadboardModule;

/*
 * BreadboardArtifactDraft
 *
 * Opaque handle to the preliminary executable artifact produced by compilation.
 * This represents the completed handoff object that can be loaded by Forge.
 */
typedef struct BreadboardArtifactDraft BreadboardArtifactDraft;

#endif /* BREADBOARD_TYPES_H */
