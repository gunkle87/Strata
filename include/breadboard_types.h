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
 * BreadboardDiagnosticCode
 *
 * Specific diagnostic identity codes for known failure or warning conditions.
 */
typedef enum BreadboardDiagnosticCode
{
    BREADBOARD_DIAG_CODE_NONE = 0,
    BREADBOARD_DIAG_CODE_UNSUPPORTED_TARGET = 1,
    BREADBOARD_DIAG_CODE_UNSUPPORTED_CONSTRUCT = 2,
    BREADBOARD_DIAG_CODE_INTERNAL_ERROR = 3
}
BreadboardDiagnosticCode;

/*
 * BreadboardDiagnostic
 *
 * A single diagnostic message emitted during compilation.
 */
typedef struct BreadboardDiagnostic
{
    BreadboardDiagnosticSeverity severity;
    BreadboardDiagnosticCode code;
    const char* message;
    /* Optional: source locus or stable ID references could be added here later */
}
BreadboardDiagnostic;

/*
 * BreadboardDescriptorClass
 *
 * Represents the fundamental runtime-visible classification of an exported object.
 */
typedef enum BreadboardDescriptorClass
{
    BREADBOARD_DESC_INPUT  = 1,
    BREADBOARD_DESC_OUTPUT = 2,
    BREADBOARD_DESC_PROBE  = 3
}
BreadboardDescriptorClass;

/*
 * BreadboardDescriptor
 *
 * A deterministic placeholder or real descriptor representing an exported object.
 */
typedef struct BreadboardDescriptor
{
    uint64_t id;
    const char* name;
    uint32_t width;
    BreadboardDescriptorClass class_type;
    bool is_placeholder;
}
BreadboardDescriptor;

/*
 * BreadboardCompileOptions
 *
 * Configuration knobs for adjusting structural compilation behavior.
 */
typedef struct BreadboardCompileOptions
{
    /* If true, explicitly allow compilation even if some constructs are unsupported. */
    bool allow_placeholders;

    /* If true, explicitly deny execution-time approximation or semantic loss. */
    bool deny_approximation;

    /* If true, explicitly enforce strict state projection rules. */
    bool strict_projection;
}
BreadboardCompileOptions;

/*
 * BreadboardTargetInfo
 *
 * Information about a specified compiler target.
 */
typedef struct BreadboardTargetInfo
{
    BreadboardTarget target;
    /* Target capabilities could be added here later. */
}
BreadboardTargetInfo;

/*
 * BreadboardDraftInfo
 *
 * Metadata surface describing a constructed compilation artifact draft.
 */
typedef struct BreadboardDraftInfo
{
    BreadboardTarget target;
    bool has_placeholders;
    size_t approximate_size_bytes;
}
BreadboardDraftInfo;


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
