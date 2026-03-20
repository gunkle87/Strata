#ifndef FORGE_CAPABILITIES_H
#define FORGE_CAPABILITIES_H

#include <stdint.h>

/*
 * forge_capabilities.h
 *
 * Machine-readable capability types for Forge backend discovery.
 * These structs are value types: callers own the storage and Forge fills them.
 * All enums use explicit integer values to remain stable across builds.
 */

/* -------------------------------------------------------------------------
 * ForgeSupportLevel
 *
 * Indicates how completely a backend supports a given capability family.
 * Use FORGE_SUPPORT_NONE when the capability is absent.
 * Use FORGE_SUPPORT_PARTIAL when support exists but has known constraints.
 * Use FORGE_SUPPORT_FULL when the capability is fully covered.
 * ------------------------------------------------------------------------- */
typedef enum ForgeSupportLevel
{
    FORGE_SUPPORT_NONE    = 0,
    FORGE_SUPPORT_PARTIAL = 1,
    FORGE_SUPPORT_FULL    = 2

}
ForgeSupportLevel;

/* -------------------------------------------------------------------------
 * ForgeStateModelClass
 *
 * Native state model class for a backend.
 * This describes the execution truth the backend preserves natively.
 * ------------------------------------------------------------------------- */
typedef enum ForgeStateModelClass
{
    /* 4-state: 0, 1, X, Z -- LXS native model. */
    FORGE_STATE_MODEL_4STATE = 0,

    /* 7-state: richer model including strength distinctions -- HighZ native model. */
    FORGE_STATE_MODEL_7STATE = 1

}
ForgeStateModelClass;

/* -------------------------------------------------------------------------
 * ForgeBackendClass
 *
 * High-level class description for a backend.
 * ------------------------------------------------------------------------- */
typedef enum ForgeBackendClass
{
    /* Fast backend: zero-delay, deterministic, performance-first. */
    FORGE_BACKEND_CLASS_FAST     = 0,

    /* Temporal backend: 3-phase delta, temporal, granularity-first. */
    FORGE_BACKEND_CLASS_TEMPORAL = 1

}
ForgeBackendClass;

/* -------------------------------------------------------------------------
 * ForgeExtensionFamily
 *
 * Stable identifiers for known backend extension families.
 * Extension families are backend-specific capabilities that go beyond the
 * common Forge API. Callers must query support before using them.
 * ------------------------------------------------------------------------- */
typedef enum ForgeExtensionFamily
{
    FORGE_EXT_NONE                = 0,
    FORGE_EXT_PERFORMANCE_PROFILE = 1,
    FORGE_EXT_TEMPORAL_CONTROL    = 2,
    FORGE_EXT_NATIVE_STATE_READ   = 3,
    FORGE_EXT_RUNTIME_DIAGNOSTICS = 4

}
ForgeExtensionFamily;

/*
 * FORGE_MAX_EXTENSION_FAMILIES
 *
 * Maximum number of extension families a backend may report in one query.
 */
#define FORGE_MAX_EXTENSION_FAMILIES 8

/* -------------------------------------------------------------------------
 * ForgeLifecycleCapabilities
 *
 * Support levels for the common Forge lifecycle families.
 * These fields describe support for the public runtime lifecycle, not
 * backend-private lifecycle mechanics.
 * ------------------------------------------------------------------------- */
typedef struct ForgeLifecycleCapabilities
{
    ForgeSupportLevel artifact_load;
    ForgeSupportLevel session_create;
    ForgeSupportLevel session_reset;
    ForgeSupportLevel session_destroy;
    ForgeSupportLevel lifecycle_query;

}
ForgeLifecycleCapabilities;

/* -------------------------------------------------------------------------
 * ForgeReadCapabilities
 *
 * Support levels for the common Forge read and lookup families.
 * These fields describe portable common-runtime observation only.
 * ------------------------------------------------------------------------- */
typedef struct ForgeReadCapabilities
{
    ForgeSupportLevel output_read;
    ForgeSupportLevel portable_signal_read;
    ForgeSupportLevel descriptor_enumeration;
    ForgeSupportLevel name_lookup;
    ForgeSupportLevel id_lookup;

}
ForgeReadCapabilities;

/* -------------------------------------------------------------------------
 * ForgeBackendInfo
 *
 * Identity information for a registered backend.
 * Filled by forge_backend_info(). All string fields are statically owned
 * by the registry and must not be freed or modified by the caller.
 * ------------------------------------------------------------------------- */
typedef struct ForgeBackendInfo
{
    /* Short stable name, e.g. "LXS" or "HighZ". */
    const char        *name;

    /* Human-readable description of the backend. */
    const char        *description;

    uint32_t           version_major;
    uint32_t           version_minor;

    ForgeBackendClass  backend_class;
    ForgeStateModelClass state_model;

}
ForgeBackendInfo;

/* -------------------------------------------------------------------------
 * ForgeCapabilities
 *
 * Machine-readable capability summary for a backend.
 * Filled by forge_backend_capabilities(). Caller allocates and owns this.
 * ------------------------------------------------------------------------- */
typedef struct ForgeCapabilities
{
    /* Common Forge lifecycle families. */
    ForgeLifecycleCapabilities lifecycle;

    /* Common advancement support. */
    ForgeSupportLevel common_single_step_advance;
    ForgeSupportLevel common_multi_step_advance;

    /* Common observation and lookup support. */
    ForgeReadCapabilities reads;

    /* Common probe enumeration and value read support. */
    ForgeSupportLevel probe_support;

    /* Common storage inspection (registers, RAM, ROM) support. */
    ForgeSupportLevel storage_inspection;

    /*
     * Whether the backend supports temporal sub-step control.
     * True for HighZ, false for LXS.
     */
    uint32_t temporal_substep;

    /*
     * Whether the backend supports delta-phase stepping.
     * True for HighZ, false for LXS.
     */
    uint32_t delta_phase_stepping;

    /* Number of valid entries in extension_families[]. */
    uint32_t extension_family_count;

    /* Extension families supported by this backend. */
    ForgeExtensionFamily extension_families[FORGE_MAX_EXTENSION_FAMILIES];

}
ForgeCapabilities;

#endif /* FORGE_CAPABILITIES_H */
