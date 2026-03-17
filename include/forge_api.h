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
 * Returns FORGE_ERR_UNSUPPORTED if data/size does not pass placeholder validation.
 * Returns FORGE_OK and writes *out_artifact on stub success.
 */
ForgeResult forge_artifact_load(
    ForgeBackendId    backend_id,
    const void       *data,
    size_t            size,
    ForgeArtifact   **out_artifact);

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
 * Stub behavior: returns FORGE_ERR_UNSUPPORTED as no real execution backend
 * is implemented yet. The out_session parameter is not written in this case.
 *
 * out_session must not be NULL.
 * Returns FORGE_ERR_INVALID_ARGUMENT if out_session is NULL.
 * Returns FORGE_ERR_INVALID_HANDLE if artifact is NULL.
 * Returns FORGE_ERR_UNSUPPORTED in this skeleton build.
 */
ForgeResult forge_session_create(
    ForgeArtifact  *artifact,
    ForgeSession  **out_session);

/*
 * forge_session_reset
 *
 * Resets a live session to its initial state, preserving the loaded artifact.
 *
 * Stub behavior: returns FORGE_ERR_INVALID_HANDLE while sessions cannot be
 * created. Will return FORGE_OK once session creation is implemented.
 *
 * Returns FORGE_ERR_INVALID_HANDLE if session is NULL.
 */
ForgeResult forge_session_reset(ForgeSession *session);

/*
 * forge_session_free
 *
 * Destroys a session and releases all resources it holds.
 * The handle must not be used after this call.
 *
 * Stub behavior: returns FORGE_ERR_INVALID_HANDLE while sessions cannot be
 * created.
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
