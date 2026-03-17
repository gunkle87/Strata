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
 * Placeholder artifact header validation for the first Forge slice.
 * The artifact must carry:
 * - shared magic
 * - format version
 * - target backend id
 * - payload size
 *
 * The payload is still stub-only. The current stub payload must be exactly
 * the 4-byte sequence "STB!".
 */
#define FORGE_ARTIFACT_MAGIC_LEN 4
#define FORGE_ARTIFACT_VERSION_MAJOR 0
#define FORGE_ARTIFACT_VERSION_MINOR 1
#define FORGE_STUB_PAYLOAD_LEN 4

static const unsigned char k_artifact_magic[FORGE_ARTIFACT_MAGIC_LEN] =
    { 0x46, 0x41, 0x52, 0x54 }; /* "FART" = Forge ARTifact placeholder magic */

static const unsigned char k_stub_payload[FORGE_STUB_PAYLOAD_LEN] =
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
    const ForgeArtifactHeader *header;
    const unsigned char *payload;

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

    if (size < sizeof(ForgeArtifactHeader))
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: artifact smaller than required header");
    }

    header = (const ForgeArtifactHeader *)data;

    if (memcmp(header->magic, k_artifact_magic, FORGE_ARTIFACT_MAGIC_LEN) != 0)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: artifact magic mismatch");
    }

    if (header->version_major != FORGE_ARTIFACT_VERSION_MAJOR ||
        header->version_minor != FORGE_ARTIFACT_VERSION_MINOR)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: unsupported artifact version");
    }

    if (header->target_backend_id != backend_id)
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: artifact target backend mismatch");
    }

    if ((size_t)header->payload_size != size - sizeof(ForgeArtifactHeader))
    {
        return forge_fail(FORGE_ERR_ARTIFACT_INCOMPATIBLE,
            "forge_artifact_load: payload size does not match artifact size");
    }

    payload = ((const unsigned char *)data) + sizeof(ForgeArtifactHeader);

    /*
     * Stub success path:
     * valid shared header + exact stub payload returns a minimal ForgeArtifact.
     * Any other payload remains unsupported until real decoding exists.
     */
    if (header->payload_size == FORGE_STUB_PAYLOAD_LEN &&
        memcmp(payload, k_stub_payload, FORGE_STUB_PAYLOAD_LEN) == 0)
    {
        art = (ForgeArtifact *)malloc(sizeof(ForgeArtifact));

        if (!art)
        {
            return forge_fail(FORGE_ERR_INTERNAL,
                "forge_artifact_load: allocation failed");
        }

        art->backend_id = backend_id;
        art->format_version_major = header->version_major;
        art->format_version_minor = header->version_minor;
        art->payload_size = header->payload_size;
        art->source_size = size;
        art->placeholder_flags = 1;

        *out_artifact = art;

        forge_diag_set("");
        return FORGE_OK;
    }

    return forge_fail(FORGE_ERR_UNSUPPORTED,
        "forge_artifact_load: artifact header valid but payload decoding is not implemented");
}

ForgeResult
forge_artifact_info(
    const ForgeArtifact *artifact,
    ForgeArtifactInfo   *out_info)
{
    if (!out_info)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_artifact_info: out_info is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_artifact_info: artifact is NULL");
    }

    out_info->backend_id = artifact->backend_id;
    out_info->format_version_major = artifact->format_version_major;
    out_info->format_version_minor = artifact->format_version_minor;
    out_info->payload_size = artifact->payload_size;
    out_info->placeholder_flags = artifact->placeholder_flags;
    out_info->source_size = artifact->source_size;

    forge_diag_set("");
    return FORGE_OK;
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
    ForgeSession *session;

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

    session = (ForgeSession *)malloc(sizeof(ForgeSession));

    if (!session)
    {
        return forge_fail(FORGE_ERR_INTERNAL,
            "forge_session_create: allocation failed");
    }

    session->artifact = artifact;
    session->placeholder_state = 0;

    *out_session = session;

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_session_info(
    const ForgeSession *session,
    ForgeSessionInfo   *out_info)
{
    if (!out_info)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_session_info: out_info is NULL");
    }

    if (!session)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_session_info: session is NULL");
    }

    out_info->backend_id = session->artifact->backend_id;
    out_info->lifecycle_state = FORGE_SESSION_STATE_READY;
    out_info->placeholder_state = session->placeholder_state;

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_session_reset(ForgeSession *session)
{
    if (!session)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_session_reset: session is NULL");
    }

    session->placeholder_state = 0;

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_session_free(ForgeSession *session)
{
    if (!session)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_session_free: session is NULL");
    }

    free(session);

    forge_diag_set("");
    return FORGE_OK;
}

/* -------------------------------------------------------------------------
 * Diagnostics
 * ------------------------------------------------------------------------- */

const char *
forge_last_error_string(void)
{
    return forge_diag_get();
}
