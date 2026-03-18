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
 * BreadboardTargetMask
 *
 * Bitmask representing a set of allowed execution targets.
 */
typedef uint32_t BreadboardTargetMask;

#define BREADBOARD_TARGET_MASK_NONE        (0u)
#define BREADBOARD_TARGET_MASK_FAST_4STATE (1u << BREADBOARD_TARGET_FAST_4STATE)
#define BREADBOARD_TARGET_MASK_TEMPORAL    (1u << BREADBOARD_TARGET_TEMPORAL)
#define BREADBOARD_TARGET_MASK_ALL         (BREADBOARD_TARGET_MASK_FAST_4STATE | BREADBOARD_TARGET_MASK_TEMPORAL)

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
    BREADBOARD_DIAG_CODE_INTERNAL_ERROR = 3,
    BREADBOARD_DIAG_CODE_TARGET_DENIED_BY_POLICY = 4
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
 * BreadboardDescriptorSpec
 *
 * Public declaration shape for authored draft-visible descriptors.
 */
typedef struct BreadboardDescriptorSpec
{
    uint64_t id;
    const char* name;
    uint32_t width;
}
BreadboardDescriptorSpec;

/*
 * BreadboardModuleIdentity
 *
 * Optional authored identity metadata for a module. This is coarse source-side
 * information that can be propagated through temporary draft/export surfaces.
 */
typedef struct BreadboardModuleIdentity
{
    uint64_t module_id;
    const char* module_name;
}
BreadboardModuleIdentity;

/*
 * BreadboardRequirementProfile
 *
 * Optional coarse authored requirement profile for a module. This remains
 * placeholder-oriented scaffolding, but allows module authors to declare which
 * temporary admission class a draft should carry.
 */
typedef struct BreadboardRequirementProfile
{
    uint32_t extension_flags;
    bool requires_advanced_controls;
    bool requires_native_state_read;
    bool requires_native_inputs;
}
BreadboardRequirementProfile;

/*
 * BreadboardStructureSummary
 *
 * Optional coarse authored structural summary for a module. This is source-side
 * metadata only and does not imply real lowering or netlist ingestion.
 */
typedef struct BreadboardStructureSummary
{
    uint32_t declared_component_count;
    uint32_t declared_connection_count;
    uint32_t declared_stateful_node_count;
}
BreadboardStructureSummary;

/*
 * BreadboardComponentSpec
 *
 * Minimal authored structural component declaration used by the current
 * scaffolding path to derive coarse structure summary counts.
 */
typedef struct BreadboardComponentSpec
{
    uint64_t id;
    const char* kind_name;
    bool is_stateful;
}
BreadboardComponentSpec;

/*
 * BreadboardConnectionSpec
 *
 * Minimal authored structural connection declaration used by the current
 * scaffolding path to derive coarse structure summary counts.
 */
typedef struct BreadboardConnectionSpec
{
    uint64_t source_component_id;
    uint64_t sink_component_id;
}
BreadboardConnectionSpec;

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
 * This is a stable coarse summary of the generated draft.
 */
typedef struct BreadboardDraftInfo
{
    BreadboardTarget target;
    bool has_placeholders;
    size_t approximate_size_bytes;
    uint64_t source_module_id;
    const char* source_module_name;
    uint32_t declared_component_count;
    uint32_t declared_connection_count;
    uint32_t declared_stateful_node_count;
}
BreadboardDraftInfo;

/*
 * BreadboardDraftAdmissionInfo
 *
 * Metadata surface describing a constructed compilation artifact draft,
 * oriented towards Forge runtime admission considerations.
 */
typedef struct BreadboardDraftAdmissionInfo
{
    BreadboardTarget target;
    bool is_placeholder;
    size_t approximate_size_bytes;
    uint32_t extension_flags;
    bool requires_advanced_controls;
    bool requires_native_state_read;
    bool requires_native_inputs;
    bool native_only_behavior;
}
BreadboardDraftAdmissionInfo;


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
