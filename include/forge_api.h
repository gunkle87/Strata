#ifndef FORGE_API_H
#define FORGE_API_H

#include <stddef.h>
#include <stdint.h>
#include "forge_result.h"
#include "forge_types.h"
#include "forge_capabilities.h"

/*
 * forge_api.h
 *
 * Public Forge runtime API.
 * All runtime interaction must go through this surface.
 * No UI, tool, or test may call backend internals directly.
 *
 * Lifecycle order:
 *   1. Discover backend via forge_backend_* calls.
 *   2. Load artifact via forge_artifact_load().
 *   3. Create session via forge_session_create().
 *   4. Stage inputs via forge_apply_inputs().
 *   5. Advance common runtime via forge_step().
 *   6. Read outputs and probes through the common boundary.
 *   7. Reset or free session.
 *   8. Unload artifact via forge_artifact_unload().
 *
 * All functions return ForgeResult. FORGE_OK means success.
 * On any failure, forge_last_error_string() may return a human-readable
 * description of the most recent error on this thread.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ForgeSessionLifecycleState
{
    FORGE_SESSION_STATE_INVALID = 0,
    FORGE_SESSION_STATE_READY   = 1

}
ForgeSessionLifecycleState;

typedef enum ForgeProductProfileKind
{
    FORGE_PRODUCT_PROFILE_UNRESTRICTED = 0,
    FORGE_PRODUCT_PROFILE_LXS_ONLY     = 1,
    FORGE_PRODUCT_PROFILE_COMMON_ONLY  = 2

}
ForgeProductProfileKind;

typedef enum ForgeSessionProfileKind
{
    FORGE_SESSION_PROFILE_DEFAULT     = 0,
    FORGE_SESSION_PROFILE_COMMON_ONLY = 1,
    FORGE_SESSION_PROFILE_NO_PROBES   = 2

}
ForgeSessionProfileKind;

typedef struct ForgeArtifactInfo
{
    ForgeBackendId backend_id;
    uint16_t format_version_major;
    uint16_t format_version_minor;
    uint32_t source_target_value;
    uint32_t source_has_placeholders;
    uint64_t source_approximate_size_bytes;
    uint64_t source_module_id;
    const char* source_module_name;
    uint32_t source_declared_component_count;
    uint32_t source_declared_connection_count;
    uint32_t source_declared_stateful_node_count;
    uint32_t payload_size;
    uint32_t placeholder_flags;
    uint32_t required_extension_mask;
    uint32_t requires_advanced_controls;
    uint32_t requires_native_state_read;
    uint32_t requires_native_inputs;
    size_t source_size;

}
ForgeArtifactInfo;

typedef struct ForgeSessionInfo
{
    ForgeBackendId backend_id;
    ForgeSessionLifecycleState lifecycle_state;
    uint32_t placeholder_state;

}
ForgeSessionInfo;

typedef enum ForgeLogicValue
{
    FORGE_LOGIC_0 = 0,
    FORGE_LOGIC_1 = 1,
    FORGE_LOGIC_X = 2,
    FORGE_LOGIC_Z = 3

}
ForgeLogicValue;

typedef struct ForgeSignalValue
{
    uint32_t signal_id;
    ForgeLogicValue value;

}
ForgeSignalValue;

typedef enum ForgeDescriptorClass
{
    FORGE_DESCRIPTOR_CLASS_INVALID = 0,
    FORGE_DESCRIPTOR_CLASS_INPUT   = 1,
    FORGE_DESCRIPTOR_CLASS_OUTPUT  = 2,
    FORGE_DESCRIPTOR_CLASS_PROBE   = 3

}
ForgeDescriptorClass;

typedef struct ForgeDescriptor
{
    uint32_t id;
    const char *name;
    uint32_t width;
    ForgeDescriptorClass descriptor_class;
    uint32_t placeholder_flags;

}
ForgeDescriptor;

typedef struct ForgeStructureComponent
{
    uint64_t id;
    const char* kind_name;
    uint32_t stateful_flags;
}
ForgeStructureComponent;

typedef struct ForgeStructureConnection
{
    uint64_t source_component_id;
    uint64_t sink_component_id;
}
ForgeStructureConnection;

typedef struct ForgeProbeValue
{
    uint32_t probe_id;
    uint64_t value;

}
ForgeProbeValue;

uint32_t forge_backend_count(void);

/*
 * forge_install_product_profile
 *
 * Installs the active product exposure profile for subsequent discovery,
 * artifact admission, and session creation.
 *
 * Loaded artifacts retain the effective profile that was active at load time
 * for artifact-side descriptor visibility. Session creation and runtime access
 * are evaluated against the product profile active at the time the session is
 * created.
 */
ForgeResult forge_install_product_profile(ForgeProductProfileKind profile_kind);

ForgeResult forge_backend_id_at(uint32_t index, ForgeBackendId *out_id);

ForgeResult forge_backend_info(ForgeBackendId backend_id, ForgeBackendInfo *out_info);

ForgeResult forge_backend_capabilities(ForgeBackendId backend_id, ForgeCapabilities *out_caps);

ForgeResult forge_extension_family_count(
    ForgeBackendId backend_id,
    uint32_t *out_count);

ForgeResult forge_extension_family_at(
    ForgeBackendId backend_id,
    uint32_t index,
    ForgeExtensionFamily *out_family);

ForgeResult forge_backend_supports_extension(
    ForgeBackendId backend_id,
    ForgeExtensionFamily extension_family,
    uint32_t *out_supported);

/*
 * forge_artifact_load
 *
 * Accepts a backend-targeted executable artifact as a byte buffer and
 * validates basic compatibility with the named backend.
 *
 * In this phase: accepts both temporary placeholder artifacts and the
 * admitted real fast-path executable artifacts when their declared contracts
 * are coherent.
 */
ForgeResult forge_artifact_load(
    ForgeBackendId    backend_id,
    const void       *data,
    size_t            size,
    ForgeArtifact   **out_artifact);

ForgeResult forge_artifact_info(
    const ForgeArtifact *artifact,
    ForgeArtifactInfo   *out_info);

ForgeResult forge_artifact_unload(ForgeArtifact *artifact);

ForgeResult forge_session_create(
    ForgeArtifact  *artifact,
    ForgeSession  **out_session);

/*
 * forge_session_create_with_profile
 *
 * Creates a session using a session-level narrowing profile. The requested
 * session profile may narrow the currently installed product profile but may
 * never widen it.
 */
ForgeResult forge_session_create_with_profile(
    ForgeArtifact            *artifact,
    ForgeSessionProfileKind   profile_kind,
    ForgeSession            **out_session);

ForgeResult forge_session_info(
    const ForgeSession *session,
    ForgeSessionInfo   *out_info);

ForgeResult forge_apply_inputs(
    ForgeSession *session,
    const ForgeSignalValue *values,
    uint32_t count);

ForgeResult forge_step(
    ForgeSession *session,
    uint32_t step_count);

ForgeResult forge_read_outputs(
    const ForgeSession *session,
    ForgeSignalValue   *values,
    uint32_t count);

ForgeResult forge_input_descriptor_count(
    const ForgeArtifact *artifact,
    uint32_t            *out_count);

ForgeResult forge_input_descriptor_at(
    const ForgeArtifact *artifact,
    uint32_t             index,
    ForgeDescriptor     *out_descriptor);

ForgeResult forge_input_descriptor_by_id(
    const ForgeArtifact *artifact,
    uint32_t             descriptor_id,
    ForgeDescriptor     *out_descriptor);

ForgeResult forge_input_descriptor_by_name(
    const ForgeArtifact *artifact,
    const char          *name,
    ForgeDescriptor     *out_descriptor);

ForgeResult forge_output_descriptor_count(
    const ForgeArtifact *artifact,
    uint32_t            *out_count);

ForgeResult forge_output_descriptor_at(
    const ForgeArtifact *artifact,
    uint32_t             index,
    ForgeDescriptor     *out_descriptor);

ForgeResult forge_output_descriptor_by_id(
    const ForgeArtifact *artifact,
    uint32_t             descriptor_id,
    ForgeDescriptor     *out_descriptor);

ForgeResult forge_output_descriptor_by_name(
    const ForgeArtifact *artifact,
    const char          *name,
    ForgeDescriptor     *out_descriptor);

ForgeResult forge_probe_descriptor_count(
    const ForgeArtifact *artifact,
    uint32_t            *out_count);

ForgeResult forge_probe_descriptor_at(
    const ForgeArtifact *artifact,
    uint32_t             index,
    ForgeDescriptor     *out_descriptor);

ForgeResult forge_probe_descriptor_by_id(
    const ForgeArtifact *artifact,
    uint32_t             descriptor_id,
    ForgeDescriptor     *out_descriptor);

ForgeResult forge_probe_descriptor_by_name(
    const ForgeArtifact *artifact,
    const char          *name,
    ForgeDescriptor     *out_descriptor);

ForgeResult forge_structure_component_count(
    const ForgeArtifact *artifact,
    uint32_t            *out_count);

ForgeResult forge_structure_component_at(
    const ForgeArtifact       *artifact,
    uint32_t                   index,
    ForgeStructureComponent   *out_component);

ForgeResult forge_structure_component_by_id(
    const ForgeArtifact       *artifact,
    uint64_t                   component_id,
    ForgeStructureComponent   *out_component);

ForgeResult forge_structure_connection_count(
    const ForgeArtifact *artifact,
    uint32_t            *out_count);

ForgeResult forge_structure_connection_at(
    const ForgeArtifact        *artifact,
    uint32_t                    index,
    ForgeStructureConnection   *out_connection);

ForgeResult forge_read_probes(
    const ForgeSession *session,
    ForgeProbeValue    *values,
    uint32_t            count);

ForgeResult forge_session_reset(ForgeSession *session);

ForgeResult forge_session_free(ForgeSession *session);

const char *forge_last_error_string(void);

#ifdef __cplusplus
}
#endif

#endif /* FORGE_API_H */


