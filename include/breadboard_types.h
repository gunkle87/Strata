#ifndef BREADBOARD_TYPES_H
#define BREADBOARD_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "strata_projection.h"

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
    BREADBOARD_DIAG_CODE_TARGET_DENIED_BY_POLICY = 4,
    BREADBOARD_DIAG_CODE_EXECUTABLE_SUBSET_REQUIRED = 5,
    BREADBOARD_DIAG_CODE_EXECUTABLE_SUBSET_INVALID = 6,
    BREADBOARD_DIAG_CODE_EXECUTABLE_LOWERING_UNAVAILABLE = 7,
    BREADBOARD_DIAG_CODE_STATE_DISTINCTION_UNSUPPORTED = 8
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
 * BreadboardExecutableSubset
 *
 * Identifies a frozen admitted executable subset contract for a target. This
 * does not imply that the subset is already implemented; it defines the
 * contract later tasks are expected to satisfy.
 */
typedef enum BreadboardExecutableSubset
{
    BREADBOARD_EXECUTABLE_SUBSET_NONE = 0,
    BREADBOARD_EXECUTABLE_SUBSET_FAST_COMBINATIONAL_V1 = 1
}
BreadboardExecutableSubset;

/*
 * BreadboardPrimitiveKind
 *
 * Admitted primitive kinds for the first real executable fast-path contract.
 */
typedef enum BreadboardPrimitiveKind
{
    BREADBOARD_PRIMITIVE_INVALID = 0,
    BREADBOARD_PRIMITIVE_BUF     = 1,
    BREADBOARD_PRIMITIVE_NOT     = 2,
    BREADBOARD_PRIMITIVE_AND     = 3,
    BREADBOARD_PRIMITIVE_OR      = 4,
    BREADBOARD_PRIMITIVE_XOR     = 5,
    BREADBOARD_PRIMITIVE_UNINIT  = 6
}
BreadboardPrimitiveKind;

/*
 * BreadboardEndpointClass
 *
 * Explicit endpoint classes for the first admitted executable subset model.
 */
typedef enum BreadboardEndpointClass
{
    BREADBOARD_ENDPOINT_INVALID = 0,
    BREADBOARD_ENDPOINT_MODULE_INPUT_SOURCE = 1,
    BREADBOARD_ENDPOINT_COMPONENT_OUTPUT_SOURCE = 2,
    BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK = 3,
    BREADBOARD_ENDPOINT_MODULE_OUTPUT_SINK = 4
}
BreadboardEndpointClass;

typedef uint32_t BreadboardPrimitiveMask;

#define BREADBOARD_PRIMITIVE_MASK_NONE (0u)
#define BREADBOARD_PRIMITIVE_MASK_BUF  (1u << BREADBOARD_PRIMITIVE_BUF)
#define BREADBOARD_PRIMITIVE_MASK_NOT  (1u << BREADBOARD_PRIMITIVE_NOT)
#define BREADBOARD_PRIMITIVE_MASK_AND  (1u << BREADBOARD_PRIMITIVE_AND)
#define BREADBOARD_PRIMITIVE_MASK_OR   (1u << BREADBOARD_PRIMITIVE_OR)
#define BREADBOARD_PRIMITIVE_MASK_XOR  (1u << BREADBOARD_PRIMITIVE_XOR)
#define BREADBOARD_PRIMITIVE_MASK_ALL_FIRST_EXECUTABLE \
    (BREADBOARD_PRIMITIVE_MASK_BUF | \
     BREADBOARD_PRIMITIVE_MASK_NOT | \
     BREADBOARD_PRIMITIVE_MASK_AND | \
     BREADBOARD_PRIMITIVE_MASK_OR | \
     BREADBOARD_PRIMITIVE_MASK_XOR)

/*
 * BreadboardExecutableSubsetInfo
 *
 * Public contract summary for the currently admitted executable subset of a
 * target. This freezes the intended first real path without leaking compiler
 * internals.
 */
typedef struct BreadboardExecutableSubsetInfo
{
    BreadboardExecutableSubset subset;
    BreadboardTarget target;
    bool has_real_executable_subset;
    bool flat_only;
    bool single_bit_only;
    bool combinational_only;
    bool allows_stateful_components;
    bool allows_cycles;
    bool real_path_hard_rejects_out_of_subset;
    bool placeholder_fallback_requires_explicit_allowance;
    BreadboardPrimitiveMask admitted_primitive_mask;
    BreadboardEndpointClass module_input_endpoint_class;
    BreadboardEndpointClass component_output_endpoint_class;
    BreadboardEndpointClass component_input_endpoint_class;
    BreadboardEndpointClass module_output_endpoint_class;
}
BreadboardExecutableSubsetInfo;

/*
 * BreadboardPrimitiveSignature
 *
 * Frozen signature shape for one admitted primitive in the first executable
 * subset contract.
 */
typedef struct BreadboardPrimitiveSignature
{
    BreadboardPrimitiveKind primitive_kind;
    BreadboardTarget target;
    uint32_t input_count;
    uint32_t output_count;
    const char* input_name_0;
    const char* input_name_1;
    const char* output_name_0;
    bool is_stateful;
    bool single_bit_only;
}
BreadboardPrimitiveSignature;

/*
 * BreadboardEndpointSpec
 *
 * Explicit endpoint declaration for the first admitted executable subset.
 * Module endpoints use descriptor_id; component endpoints use component_id and
 * slot_index. Unused identity fields must be zeroed.
 */
typedef struct BreadboardEndpointSpec
{
    BreadboardEndpointClass endpoint_class;
    uint64_t descriptor_id;
    uint64_t component_id;
    uint32_t slot_index;
}
BreadboardEndpointSpec;

/*
 * BreadboardExecutableConnectionSpec
 *
 * Endpoint-aware connection declaration for executable-subset legality
 * analysis. This is still structural contract data only, not lowered IR.
 */
typedef struct BreadboardExecutableConnectionSpec
{
    BreadboardEndpointSpec source;
    BreadboardEndpointSpec sink;
}
BreadboardExecutableConnectionSpec;

/*
 * BreadboardExecutableAssessmentStatus
 *
 * Outcome of assessing a module against the frozen admitted executable subset.
 */
typedef enum BreadboardExecutableAssessmentStatus
{
    BREADBOARD_EXECUTABLE_ASSESSMENT_INVALID = 0,
    BREADBOARD_EXECUTABLE_ASSESSMENT_PLACEHOLDER_ONLY = 1,
    BREADBOARD_EXECUTABLE_ASSESSMENT_EXECUTABLE = 2
}
BreadboardExecutableAssessmentStatus;

/*
 * BreadboardExecutableAssessmentReason
 *
 * Coarse explanation for why a module is executable, placeholder-only, or
 * structurally invalid for the admitted fast-path subset.
 */
typedef enum BreadboardExecutableAssessmentReason
{
    BREADBOARD_EXEC_REASON_NONE = 0,
    BREADBOARD_EXEC_REASON_TARGET_UNSUPPORTED = 1,
    BREADBOARD_EXEC_REASON_PROFILE_UNSUPPORTED = 2,
    BREADBOARD_EXEC_REASON_PROBES_UNSUPPORTED = 3,
    BREADBOARD_EXEC_REASON_DESCRIPTOR_WIDTH_UNSUPPORTED = 4,
    BREADBOARD_EXEC_REASON_STATEFUL_COMPONENT_UNSUPPORTED = 5,
    BREADBOARD_EXEC_REASON_PRIMITIVE_UNSUPPORTED = 6,
    BREADBOARD_EXEC_REASON_EXECUTABLE_CONNECTIONS_REQUIRED = 7,
    BREADBOARD_EXEC_REASON_INVALID_ENDPOINT = 8,
    BREADBOARD_EXEC_REASON_DUPLICATE_SINK_DRIVER = 9,
    BREADBOARD_EXEC_REASON_MISSING_REQUIRED_DRIVER = 10,
    BREADBOARD_EXEC_REASON_CYCLE_DETECTED = 11,
    BREADBOARD_EXEC_REASON_INITIALIZATION_STATE_UNSUPPORTED = 12
}
BreadboardExecutableAssessmentReason;

/*
 * BreadboardExecutableAssessment
 *
 * Public legality assessment surface for the admitted first executable subset.
 */
typedef struct BreadboardExecutableAssessment
{
    BreadboardExecutableSubset subset;
    BreadboardExecutableAssessmentStatus status;
    BreadboardExecutableAssessmentReason reason;
    uint64_t failing_component_id;
    size_t failing_connection_index;
}
BreadboardExecutableAssessment;

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
    uint32_t required_projection_families_mask;
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
 * BreadboardComponent
 *
 * Query surface for authored structural components retained on a compiled
 * draft. This remains temporary scaffolding metadata, not final IR.
 */
typedef struct BreadboardComponent
{
    uint64_t id;
    const char* kind_name;
    bool is_stateful;
}
BreadboardComponent;

/*
 * BreadboardConnection
 *
 * Query surface for authored structural connections retained on a compiled
 * draft. This remains temporary scaffolding metadata, not final IR.
 */
typedef struct BreadboardConnection
{
    uint64_t source_component_id;
    uint64_t sink_component_id;
}
BreadboardConnection;

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

    /* If true, require the first real executable path and disallow placeholder fallback. */
    bool require_real_executable;

    /* Bitmask of projection families allowed for this compile (0 = use module policy) */
    uint32_t allowed_projection_families_mask;

    /* If true, generate projection report metadata for this compile */
    bool generate_projection_report;

    /* Reserved for future extension */
    uint32_t reserved[2];
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
    /* Bitmask of projection families natively supported by this target */
    uint32_t allowed_projection_families_mask;
    /* Reserved for future target capability fields */
    uint32_t reserved[3];
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
 * BreadboardProjectionPolicy
 *
 * Compile-time projection policy configuration.
 * This structure defines how state projection should be applied during
 * lowering for backend-targeted compilation.
 */
typedef struct BreadboardProjectionPolicy
{
    /* Bitmask of allowed projection families (StrataProjectionFamily). */
    uint32_t allowed_families_mask;

    /* If true, reject any semantic approximation (collapse/resolution). */
    bool deny_approximation;

    /* If true, enforce strict projection rules (no backend-specific loss). */
    bool strict_projection;

    /* If true, generate projection report metadata. */
    bool generate_report;

    /* Reserved for future extension. */
    uint32_t reserved[4];
}
BreadboardProjectionPolicy;

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
