#include <stdlib.h>
#include <string.h>
#include "../../include/forge_api.h"
#include "../common/forge_diagnostic.h"
#include "forge_internal.h"
#include "forge_registry.h"

/*
 * forge_api.c
 *
 * Stub artifact, descriptor, and session lifecycle for Forge.
 * No real backend execution is implemented here.
 * All execution paths return explicit result codes.
 *
 * Backend discovery functions (forge_backend_count, forge_backend_id_at,
 * forge_backend_info, and forge_backend_capabilities) delegate directly to the
 * registry.
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

static const ForgeDescriptor k_placeholder_output_descriptors[] =
{
    { 1u, "out0", 1u, FORGE_DESCRIPTOR_CLASS_OUTPUT, 1u },
    { 2u, "out1", 1u, FORGE_DESCRIPTOR_CLASS_OUTPUT, 1u }
};

static const ForgeDescriptor k_placeholder_input_descriptors[] =
{
    { 101u, "in0", 1u, FORGE_DESCRIPTOR_CLASS_INPUT, 1u },
    { 102u, "in1", 1u, FORGE_DESCRIPTOR_CLASS_INPUT, 1u }
};

static const ForgeDescriptor k_placeholder_probe_descriptors[] =
{
    { 1001u, "probe_step_count", 64u, FORGE_DESCRIPTOR_CLASS_PROBE, 1u }
};

static uint32_t
forge_descriptor_array_count(const ForgeDescriptor *descriptors, uint32_t count)
{
    (void)descriptors;
    return count;
}

static ForgeResult
forge_descriptor_find_by_id(
    const ForgeDescriptor *descriptors,
    uint32_t               count,
    uint32_t               descriptor_id,
    ForgeDescriptor       *out_descriptor,
    const char            *null_descriptor_msg,
    const char            *not_found_msg)
{
    uint32_t index;

    if (!out_descriptor)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT, null_descriptor_msg);
    }

    for (index = 0; index < count; ++index)
    {
        if (descriptors[index].id == descriptor_id)
        {
            *out_descriptor = descriptors[index];
            forge_diag_set("");
            return FORGE_OK;
        }
    }

    return forge_fail(FORGE_ERR_OUT_OF_BOUNDS, not_found_msg);
}

static ForgeResult
forge_descriptor_find_by_name(
    const ForgeDescriptor *descriptors,
    uint32_t               count,
    const char            *name,
    ForgeDescriptor       *out_descriptor,
    const char            *null_argument_msg,
    const char            *not_found_msg)
{
    uint32_t index;

    if (!name || !out_descriptor)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT, null_argument_msg);
    }

    for (index = 0; index < count; ++index)
    {
        if (strcmp(descriptors[index].name, name) == 0)
        {
            *out_descriptor = descriptors[index];
            forge_diag_set("");
            return FORGE_OK;
        }
    }

    return forge_fail(FORGE_ERR_OUT_OF_BOUNDS, not_found_msg);
}

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
forge_apply_inputs(
    ForgeSession *session,
    const ForgeSignalValue *values,
    uint32_t count)
{
    if (!session)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_apply_inputs: session is NULL");
    }

    if (!values || count == 0)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_apply_inputs: values is NULL or count is zero");
    }

    return forge_fail(FORGE_ERR_UNSUPPORTED,
        "forge_apply_inputs: runtime input mapping not yet implemented");
}

ForgeResult
forge_step(
    ForgeSession *session,
    uint32_t step_count)
{
    if (!session)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_step: session is NULL");
    }

    if (step_count == 0)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_step: step_count is zero");
    }

    return forge_fail(FORGE_ERR_UNSUPPORTED,
        "forge_step: runtime advancement not yet implemented");
}

ForgeResult
forge_read_outputs(
    const ForgeSession *session,
    ForgeSignalValue   *values,
    uint32_t count)
{
    if (!session)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_read_outputs: session is NULL");
    }

    if (!values || count == 0)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_read_outputs: values is NULL or count is zero");
    }

    return forge_fail(FORGE_ERR_UNSUPPORTED,
        "forge_read_outputs: runtime output mapping not yet implemented");
}

ForgeResult
forge_input_descriptor_count(
    const ForgeArtifact *artifact,
    uint32_t            *out_count)
{
    if (!out_count)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_input_descriptor_count: out_count is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_input_descriptor_count: artifact is NULL");
    }

    *out_count = forge_descriptor_array_count(
        k_placeholder_input_descriptors,
        (uint32_t)(sizeof(k_placeholder_input_descriptors) /
        sizeof(k_placeholder_input_descriptors[0])));

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_input_descriptor_at(
    const ForgeArtifact *artifact,
    uint32_t             index,
    ForgeDescriptor     *out_descriptor)
{
    uint32_t count;

    if (!out_descriptor)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_input_descriptor_at: out_descriptor is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_input_descriptor_at: artifact is NULL");
    }

    count = (uint32_t)(sizeof(k_placeholder_input_descriptors) /
        sizeof(k_placeholder_input_descriptors[0]));

    if (index >= count)
    {
        return forge_fail(FORGE_ERR_OUT_OF_BOUNDS,
            "forge_input_descriptor_at: index out of bounds");
    }

    *out_descriptor = k_placeholder_input_descriptors[index];

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_input_descriptor_by_id(
    const ForgeArtifact *artifact,
    uint32_t             descriptor_id,
    ForgeDescriptor     *out_descriptor)
{
    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_input_descriptor_by_id: artifact is NULL");
    }

    return forge_descriptor_find_by_id(
        k_placeholder_input_descriptors,
        (uint32_t)(sizeof(k_placeholder_input_descriptors) /
        sizeof(k_placeholder_input_descriptors[0])),
        descriptor_id,
        out_descriptor,
        "forge_input_descriptor_by_id: out_descriptor is NULL",
        "forge_input_descriptor_by_id: descriptor_id not found");
}

ForgeResult
forge_input_descriptor_by_name(
    const ForgeArtifact *artifact,
    const char          *name,
    ForgeDescriptor     *out_descriptor)
{
    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_input_descriptor_by_name: artifact is NULL");
    }

    return forge_descriptor_find_by_name(
        k_placeholder_input_descriptors,
        (uint32_t)(sizeof(k_placeholder_input_descriptors) /
        sizeof(k_placeholder_input_descriptors[0])),
        name,
        out_descriptor,
        "forge_input_descriptor_by_name: name or out_descriptor is NULL",
        "forge_input_descriptor_by_name: descriptor name not found");
}

ForgeResult
forge_output_descriptor_count(
    const ForgeArtifact *artifact,
    uint32_t            *out_count)
{
    if (!out_count)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_output_descriptor_count: out_count is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_output_descriptor_count: artifact is NULL");
    }

    *out_count = forge_descriptor_array_count(
        k_placeholder_output_descriptors,
        (uint32_t)(sizeof(k_placeholder_output_descriptors) /
        sizeof(k_placeholder_output_descriptors[0])));

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_output_descriptor_at(
    const ForgeArtifact *artifact,
    uint32_t             index,
    ForgeDescriptor     *out_descriptor)
{
    uint32_t count;

    if (!out_descriptor)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_output_descriptor_at: out_descriptor is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_output_descriptor_at: artifact is NULL");
    }

    count = (uint32_t)(sizeof(k_placeholder_output_descriptors) /
        sizeof(k_placeholder_output_descriptors[0]));

    if (index >= count)
    {
        return forge_fail(FORGE_ERR_OUT_OF_BOUNDS,
            "forge_output_descriptor_at: index out of bounds");
    }

    *out_descriptor = k_placeholder_output_descriptors[index];

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_output_descriptor_by_id(
    const ForgeArtifact *artifact,
    uint32_t             descriptor_id,
    ForgeDescriptor     *out_descriptor)
{
    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_output_descriptor_by_id: artifact is NULL");
    }

    return forge_descriptor_find_by_id(
        k_placeholder_output_descriptors,
        (uint32_t)(sizeof(k_placeholder_output_descriptors) /
        sizeof(k_placeholder_output_descriptors[0])),
        descriptor_id,
        out_descriptor,
        "forge_output_descriptor_by_id: out_descriptor is NULL",
        "forge_output_descriptor_by_id: descriptor_id not found");
}

ForgeResult
forge_output_descriptor_by_name(
    const ForgeArtifact *artifact,
    const char          *name,
    ForgeDescriptor     *out_descriptor)
{
    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_output_descriptor_by_name: artifact is NULL");
    }

    return forge_descriptor_find_by_name(
        k_placeholder_output_descriptors,
        (uint32_t)(sizeof(k_placeholder_output_descriptors) /
        sizeof(k_placeholder_output_descriptors[0])),
        name,
        out_descriptor,
        "forge_output_descriptor_by_name: name or out_descriptor is NULL",
        "forge_output_descriptor_by_name: descriptor name not found");
}

ForgeResult
forge_probe_descriptor_count(
    const ForgeArtifact *artifact,
    uint32_t            *out_count)
{
    if (!out_count)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_probe_descriptor_count: out_count is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_probe_descriptor_count: artifact is NULL");
    }

    *out_count = forge_descriptor_array_count(
        k_placeholder_probe_descriptors,
        (uint32_t)(sizeof(k_placeholder_probe_descriptors) /
        sizeof(k_placeholder_probe_descriptors[0])));

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_probe_descriptor_at(
    const ForgeArtifact *artifact,
    uint32_t             index,
    ForgeDescriptor     *out_descriptor)
{
    uint32_t count;

    if (!out_descriptor)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_probe_descriptor_at: out_descriptor is NULL");
    }

    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_probe_descriptor_at: artifact is NULL");
    }

    count = (uint32_t)(sizeof(k_placeholder_probe_descriptors) /
        sizeof(k_placeholder_probe_descriptors[0]));

    if (index >= count)
    {
        return forge_fail(FORGE_ERR_OUT_OF_BOUNDS,
            "forge_probe_descriptor_at: index out of bounds");
    }

    *out_descriptor = k_placeholder_probe_descriptors[index];

    forge_diag_set("");
    return FORGE_OK;
}

ForgeResult
forge_probe_descriptor_by_id(
    const ForgeArtifact *artifact,
    uint32_t             descriptor_id,
    ForgeDescriptor     *out_descriptor)
{
    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_probe_descriptor_by_id: artifact is NULL");
    }

    return forge_descriptor_find_by_id(
        k_placeholder_probe_descriptors,
        (uint32_t)(sizeof(k_placeholder_probe_descriptors) /
        sizeof(k_placeholder_probe_descriptors[0])),
        descriptor_id,
        out_descriptor,
        "forge_probe_descriptor_by_id: out_descriptor is NULL",
        "forge_probe_descriptor_by_id: descriptor_id not found");
}

ForgeResult
forge_probe_descriptor_by_name(
    const ForgeArtifact *artifact,
    const char          *name,
    ForgeDescriptor     *out_descriptor)
{
    if (!artifact)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_probe_descriptor_by_name: artifact is NULL");
    }

    return forge_descriptor_find_by_name(
        k_placeholder_probe_descriptors,
        (uint32_t)(sizeof(k_placeholder_probe_descriptors) /
        sizeof(k_placeholder_probe_descriptors[0])),
        name,
        out_descriptor,
        "forge_probe_descriptor_by_name: name or out_descriptor is NULL",
        "forge_probe_descriptor_by_name: descriptor name not found");
}

ForgeResult
forge_read_probes(
    const ForgeSession *session,
    ForgeProbeValue    *values,
    uint32_t            count)
{
    if (!session)
    {
        return forge_fail(FORGE_ERR_INVALID_HANDLE,
            "forge_read_probes: session is NULL");
    }

    if (!values || count == 0)
    {
        return forge_fail(FORGE_ERR_INVALID_ARGUMENT,
            "forge_read_probes: values is NULL or count is zero");
    }

    return forge_fail(FORGE_ERR_UNSUPPORTED,
        "forge_read_probes: runtime probe mapping not yet implemented");
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
