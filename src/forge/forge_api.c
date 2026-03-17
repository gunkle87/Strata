#include <stdlib.h>
#include <string.h>
#include "../../include/forge_api.h"
#include "../common/forge_diagnostic.h"
#include "forge_internal.h"
#include "forge_registry.h"

/*
 * forge_api.c
 *
 * Stub artifact and session lifecycle for Forge.
 * No real backend execution is implemented here.
 * All execution paths return explicit result codes.
 *
 * Backend discovery functions (forge_backend_count, forge_backend_id_at,
 * forge_backend_info) delegate directly to the registry. Capability queries
 * delegate to forge_capabilities.c through the public API.
 */

/* -------------------------------------------------------------------------
 * Internal convenience: set diagnostic and return a result in one call.
 * ------------------------------------------------------------------------- */
static ForgeResult
forge_fail(ForgeResult result, const char *msg)
{
    forge_diag_set(msg);
    return result;
}

/*
 * Stub magic bytes.
 * Callers pass this 4-byte sequence to trigger the stub artifact success path.
 * All other data returns FORGE_ERR_UNSUPPORTED until real decoding exists.
 */
#define FORGE_STUB_MAGIC_LEN 4
static const unsigned char k_stub_magic[FORGE_STUB_MAGIC_LEN] =
    { 0x53, 0x54, 0x42, 0x21 }; /* "STB!" */

/* -------------------------------------------------------------------------
 * Backend Discovery
 * ------------------------------------------------------------------------- */

uint32_t
forge_backend_count(void)
{
    return forge_registry_backend_count();
}

ForgeResult
forge_backend_id_at(uint32_t index, ForgeBackendId *out_id)
{
    const ForgeBackendRecord *rec;

    if (!out_id)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_backend_id_at: out_id is NULL");
    }

    rec = forge_registry_backend_at(index);

    if (!rec)
    {
        return forge_fail(FORGE_ERR_OUT_OF_BOUNDS,
            "forge_backend_id_at: index out of bounds");
    }

    *out_id = rec->id;

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_backend_info(ForgeBackendId backend_id, ForgeBackendInfo *out_info)
{
    const ForgeBackendRecord *rec;

    if (!out_info)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_backend_info: out_info is NULL");
    }

    rec = forge_registry_backend_by_id(backend_id);

    if (!rec)
    {
        return forge_fail(FORGE_ERR_BACKEND_UNAVAILABLE,
            "forge_backend_info: backend_id not registered");
    }

    *out_info = rec->info;

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_backend_capabilities(ForgeBackendId backend_id, ForgeCapabilities *out_caps)
{
    const ForgeBackendRecord *rec;

    if (!out_caps)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_backend_capabilities: out_caps is NULL");
    }

    rec = forge_registry_backend_by_id(backend_id);

    if (!rec)
    {
        return forge_fail(FORGE_ERR_BACKEND_UNAVAILABLE,
            "forge_backend_capabilities: backend_id not registered");
    }

    *out_caps = rec->capabilities;

    forge_diag_set("");
    return FORGE_OK;
}

/* -------------------------------------------------------------------------
 * Artifact Lifecycle
 * ------------------------------------------------------------------------- */

ForgeResult
forge_artifact_load(
    ForgeBackendId   backend_id,
    const void      *data,
    size_t           size,
    ForgeArtifact  **out_artifact)
{
    const ForgeBackendRecord *rec;
    ForgeArtifact            *art;

    if (!out_artifact || !data)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_artifact_load: NULL argument");
    }

    *out_artifact = NULL;

    rec = forge_registry_backend_by_id(backend_id);

    if (!rec)
    {
        return forge_fail(FORGE_ERR_BACKEND_UNAVAILABLE,
            "forge_artifact_load: backend_id not registered");
    }

    /*
     * Stub success path: exactly the stub magic sequence returns a minimal
     * ForgeArtifact marked as a stub. All other data is unsupported until
     * real artifact decoding is implemented.
     */
    if (size == FORGE_STUB_MAGIC_LEN &&
        memcmp(data, k_stub_magic, FORGE_STUB_MAGIC_LEN) == 0)
    {
        art = (ForgeArtifact *)malloc(sizeof(ForgeArtifact));

        if (!art)
        {
            return forge_fail(FORGE_ERR_INTERNAL,
                "forge_artifact_load: allocation failed");
        }

        art->backend_id = backend_id;
        art->source_size = size;
        art->placeholder_flags = 1;

        *out_artifact = art;

        forge_diag_set("");
        return FORGE_OK;
    }

    return forge_fail(FORGE_ERR_UNSUPPORTED,
        "forge_artifact_load: artifact decoding not yet implemented");
}

ForgeResult
forge_artifact_unload(ForgeArtifact *artifact)
{
    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_artifact_unload: artifact is NULL");
    }

    free(artifact);

    forge_diag_set("");
    return FORGE_OK;
}

/* -------------------------------------------------------------------------
 * Session Lifecycle
 * ------------------------------------------------------------------------- */

ForgeResult
forge_session_create(
    ForgeArtifact  *artifact,
    ForgeSession  **out_session)
{
    if (!out_session)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_session_create: out_session is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_session_create: artifact is NULL");
    }

    *out_session = NULL;

    return forge_fail(FORGE_ERR_UNSUPPORTED,
        "forge_session_create: runtime execution not yet implemented");
}

ForgeResult
forge_session_reset(ForgeSession *session)
{
    if (!session)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_session_reset: session is NULL");
    }

    return forge_fail(FORGE_ERR_UNSUPPORTED,
        "forge_session_reset: runtime execution not yet implemented");
}

ForgeResult
forge_session_free(ForgeSession *session)
{
    if (!session)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_session_free: session is NULL");
    }

    return forge_fail(FORGE_ERR_UNSUPPORTED,
        "forge_session_free: runtime execution not yet implemented");
}

/* -------------------------------------------------------------------------
 * Diagnostics
 * ------------------------------------------------------------------------- */

const char *
forge_last_error_string(void)
{
    return forge_diag_get();
}
