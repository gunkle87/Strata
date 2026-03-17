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
 *   4. [advance / read / reset — not yet implemented in skeleton]
 *   5. Free session via forge_session_free().
 *   6. Unload artifact via forge_artifact_unload().
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

typedef struct ForgeArtifactInfo
{
    ForgeBackendId backend_id;
    uint16_t format_version_major;
    uint16_t format_version_minor;
    uint32_t payload_size;
    uint32_t placeholder_flags;
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

/* -------------------------------------------------------------------------
 * Backend Discovery
 * ------------------------------------------------------------------------- */

/*
 * forge_backend_count
 *
 * Returns the number of registered backends.
 * This count is stable for the lifetime of the process.
 */
uint32_t forge_backend_count(void);

/*
 * forge_backend_id_at
 *
 * Returns the backend ID at the given zero-based index.
 * out_id must not be NULL.
 * Returns FORGE_ERR_INVALID_ARGUMENT if out_id is NULL.
 * Returns FORGE_ERR_OUT_OF_BOUNDS if index >= forge_backend_count().
 */
ForgeResult forge_backend_id_at(uint32_t index, ForgeBackendId *out_id);

/*
 * forge_backend_info
 *
 * Fills *out_info with identity information for the given backend ID.
 * out_info must not be NULL.
 * Returns FORGE_ERR_INVALID_ARGUMENT if out_info is NULL.
 * Returns FORGE_ERR_BACKEND_UNAVAILABLE if backend_id is not registered.
 */
ForgeResult forge_backend_info(ForgeBackendId backend_id, ForgeBackendInfo *out_info);

/*
 * forge_backend_capabilities
 *
 * Fills *out_caps with capability data for the given backend ID.
 * out_caps must not be NULL.
 * Returns FORGE_ERR_INVALID_ARGUMENT if out_caps is NULL.
 * Returns FORGE_ERR_BACKEND_UNAVAILABLE if backend_id is not registered.
 */
ForgeResult forge_backend_capabilities(ForgeBackendId backend_id, ForgeCapabilities *out_caps);

/* -------------------------------------------------------------------------
 * Executable Artifact Lifecycle
 * ------------------------------------------------------------------------- */

/*
 * forge_artifact_load
 *
 * Accepts a backend-targeted executable artifact as a byte buffer and
 * validates basic compatibility with the named backend.
 *
 * In this skeleton: returns a minimal stub ForgeArtifact for known backends
 * with non-NULL data and non-zero size. The artifact is marked not executable
 * until real decoding is implemented.
 *
 * out_artifact must not be NULL.
 * Returns FORGE_ERR_INVALID_ARGUMENT if out_artifact or data is NULL.
 * Returns FORGE_ERR_BACKEND_UNAVAILABLE if backend_id is not registered.
 * Returns FORGE_ERR_ARTIFACT_INCOMPATIBLE for malformed or wrong-target headers.
 * Returns FORGE_ERR_UNSUPPORTED for valid headers with unimplemented payloads.
 * Returns FORGE_OK and writes *out_artifact on stub success.
 */
ForgeResult forge_artifact_load(
    ForgeBackendId    backend_id,
    const void       *data,
    size_t            size,
    ForgeArtifact   **out_artifact);

/*
 * forge_artifact_info
 *
 * Fills *out_info with metadata captured at artifact load time.
 *
 * out_info must not be NULL.
 * Returns FORGE_ERR_INVALID_ARGUMENT if out_info is NULL.
 * Returns FORGE_ERR_INVALID_HANDLE if artifact is NULL.
 * Returns FORGE_OK on success.
 */
ForgeResult forge_artifact_info(
    const ForgeArtifact *artifact,
    ForgeArtifactInfo   *out_info);

/*
 * forge_artifact_unload
 *
 * Releases all resources held by the artifact handle.
 * The handle must not be used after this call.
 *
 * Returns FORGE_ERR_INVALID_HANDLE if artifact is NULL.
 * Returns FORGE_OK on success.
 */
ForgeResult forge_artifact_unload(ForgeArtifact *artifact);

/* -------------------------------------------------------------------------
 * Session Lifecycle
 * ------------------------------------------------------------------------- */

/*
 * forge_session_create
 *
 * Creates a runtime session from a loaded artifact.
 *
 * Stub behavior: allocates a minimal session handle bound to the loaded
 * artifact. This validates the session lifecycle boundary without exposing
 * any real execution controls yet.
 *
 * out_session must not be NULL.
 * Returns FORGE_ERR_INVALID_ARGUMENT if out_session is NULL.
 * Returns FORGE_ERR_INVALID_HANDLE if artifact is NULL.
 * Returns FORGE_OK on stub session creation success.
 */
ForgeResult forge_session_create(
    ForgeArtifact  *artifact,
    ForgeSession  **out_session);

/*
 * forge_session_info
 *
 * Fills *out_info with lifecycle metadata for a live session.
 *
 * out_info must not be NULL.
 * Returns FORGE_ERR_INVALID_ARGUMENT if out_info is NULL.
 * Returns FORGE_ERR_INVALID_HANDLE if session is NULL.
 * Returns FORGE_OK on success.
 */
ForgeResult forge_session_info(
    const ForgeSession *session,
    ForgeSessionInfo   *out_info);

/*
 * forge_session_reset
 *
 * Resets a live session to its initial state, preserving the loaded artifact.
 *
 * Stub behavior: resets placeholder session state only. No backend execution
 * state exists yet in this skeleton.
 *
 * Returns FORGE_ERR_INVALID_HANDLE if session is NULL.
 * Returns FORGE_OK on stub reset success.
 */
ForgeResult forge_session_reset(ForgeSession *session);

/*
 * forge_session_free
 *
 * Destroys a session and releases all resources it holds.
 * The handle must not be used after this call.
 *
 * Stub behavior: releases the placeholder session object only.
 *
 * Returns FORGE_ERR_INVALID_HANDLE if session is NULL.
 * Returns FORGE_OK on success.
 */
ForgeResult forge_session_free(ForgeSession *session);

/* -------------------------------------------------------------------------
 * Diagnostics
 * ------------------------------------------------------------------------- */

/*
 * forge_last_error_string
 *
 * Returns a human-readable description of the most recent Forge error on
 * this call path. The returned pointer is statically owned and must not be
 * freed. The string is overwritten on the next Forge call that sets an error.
 *
 * Returns an empty string if no error has been set.
 * Never returns NULL.
 */
const char *forge_last_error_string(void);

#ifdef __cplusplus
}
#endif

#endif /* FORGE_API_H */
