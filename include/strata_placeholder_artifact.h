#ifndef STRATA_PLACEHOLDER_ARTIFACT_H
#define STRATA_PLACEHOLDER_ARTIFACT_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/*
 * strata_placeholder_artifact.h
 *
 * Temporary shared placeholder artifact contract for Breadboard -> Forge
 * vertical integration. This file does not define the final Strata artifact
 * format. It exists only to keep the current placeholder handoff rules in one
 * place while the real format is still being designed.
 */

#define STRATA_PLACEHOLDER_ARTIFACT_MAGIC_LEN 4u
#define STRATA_PLACEHOLDER_ARTIFACT_VERSION_MAJOR 0u
#define STRATA_PLACEHOLDER_ARTIFACT_VERSION_MINOR 1u
#define STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN 4u

#define STRATA_PLACEHOLDER_BACKEND_ID_INVALID 0u
#define STRATA_PLACEHOLDER_BACKEND_ID_LXS 1u
#define STRATA_PLACEHOLDER_BACKEND_ID_HIGHZ 2u

typedef struct StrataPlaceholderArtifactHeader
{
    uint8_t magic[4];
    uint16_t version_major;
    uint16_t version_minor;
    uint32_t target_backend_id;
    uint32_t payload_size;
}
StrataPlaceholderArtifactHeader;

typedef enum StrataPlaceholderPayloadKind
{
    STRATA_PLACEHOLDER_PAYLOAD_INVALID  = 0,
    STRATA_PLACEHOLDER_PAYLOAD_BASELINE = 1,
    STRATA_PLACEHOLDER_PAYLOAD_ADVANCED = 2,
    STRATA_PLACEHOLDER_PAYLOAD_NATIVE   = 3
}
StrataPlaceholderPayloadKind;

static const uint8_t k_strata_placeholder_artifact_magic[STRATA_PLACEHOLDER_ARTIFACT_MAGIC_LEN] =
    { 0x46, 0x41, 0x52, 0x54 }; /* "FART" */

static const uint8_t k_strata_placeholder_payload_baseline[STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN] =
    { 0x53, 0x54, 0x42, 0x21 }; /* "STB!" */

static const uint8_t k_strata_placeholder_payload_advanced[STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN] =
    { 0x41, 0x44, 0x56, 0x21 }; /* "ADV!" */

static const uint8_t k_strata_placeholder_payload_native[STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN] =
    { 0x4E, 0x41, 0x54, 0x21 }; /* "NAT!" */

static inline size_t
strata_placeholder_artifact_size(void)
{
    return sizeof(StrataPlaceholderArtifactHeader) +
        STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN;
}

static inline uint32_t
strata_placeholder_backend_id_for_target_value(uint32_t target_value)
{
    switch (target_value)
    {
        case 1u:
            return STRATA_PLACEHOLDER_BACKEND_ID_LXS;
        case 2u:
            return STRATA_PLACEHOLDER_BACKEND_ID_HIGHZ;
        default:
            return STRATA_PLACEHOLDER_BACKEND_ID_INVALID;
    }
}

static inline const uint8_t*
strata_placeholder_payload_bytes(StrataPlaceholderPayloadKind payload_kind)
{
    switch (payload_kind)
    {
        case STRATA_PLACEHOLDER_PAYLOAD_BASELINE:
            return k_strata_placeholder_payload_baseline;
        case STRATA_PLACEHOLDER_PAYLOAD_ADVANCED:
            return k_strata_placeholder_payload_advanced;
        case STRATA_PLACEHOLDER_PAYLOAD_NATIVE:
            return k_strata_placeholder_payload_native;
        default:
            return NULL;
    }
}

static inline int
strata_placeholder_payload_matches(
    const void* payload,
    StrataPlaceholderPayloadKind payload_kind)
{
    const uint8_t* expected;

    expected = strata_placeholder_payload_bytes(payload_kind);
    if (!payload || !expected)
    {
        return 0;
    }

    return memcmp(payload, expected, STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN) == 0;
}

static inline int
strata_placeholder_artifact_write(
    void* buffer,
    size_t buffer_size,
    uint32_t backend_id,
    StrataPlaceholderPayloadKind payload_kind,
    size_t* out_size)
{
    StrataPlaceholderArtifactHeader header;
    const uint8_t* payload_bytes;
    size_t required_size;

    if (!buffer || !out_size || backend_id == STRATA_PLACEHOLDER_BACKEND_ID_INVALID)
    {
        return 0;
    }

    payload_bytes = strata_placeholder_payload_bytes(payload_kind);
    if (!payload_bytes)
    {
        return 0;
    }

    required_size = strata_placeholder_artifact_size();
    if (buffer_size < required_size)
    {
        return 0;
    }

    memcpy(header.magic, k_strata_placeholder_artifact_magic, sizeof(header.magic));
    header.version_major = STRATA_PLACEHOLDER_ARTIFACT_VERSION_MAJOR;
    header.version_minor = STRATA_PLACEHOLDER_ARTIFACT_VERSION_MINOR;
    header.target_backend_id = backend_id;
    header.payload_size = STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN;

    memcpy(buffer, &header, sizeof(header));
    memcpy(((uint8_t*)buffer) + sizeof(header), payload_bytes,
        STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN);

    *out_size = required_size;
    return 1;
}

#endif /* STRATA_PLACEHOLDER_ARTIFACT_H */
