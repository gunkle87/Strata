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
#define STRATA_PLACEHOLDER_ARTIFACT_VERSION_MINOR 2u
#define STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN 4u

#define STRATA_PLACEHOLDER_BACKEND_ID_INVALID 0u
#define STRATA_PLACEHOLDER_BACKEND_ID_LXS 1u
#define STRATA_PLACEHOLDER_BACKEND_ID_HIGHZ 2u

#define STRATA_PLACEHOLDER_REQUIREMENT_NONE             0u
#define STRATA_PLACEHOLDER_REQUIREMENT_ADVANCED_CONTROL (1u << 0)
#define STRATA_PLACEHOLDER_REQUIREMENT_NATIVE_STATE     (1u << 1)

typedef struct StrataPlaceholderAdmissionInfo
{
    uint32_t requirement_flags;
    uint32_t requires_advanced_controls;
    uint32_t requires_native_state_read;
    uint32_t requires_native_inputs;
}
StrataPlaceholderAdmissionInfo;

typedef struct StrataPlaceholderArtifactHeader
{
    uint8_t magic[4];
    uint16_t version_major;
    uint16_t version_minor;
    uint32_t target_backend_id;
    uint32_t payload_size;
    uint32_t payload_kind;
    StrataPlaceholderAdmissionInfo admission_info;
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

typedef struct StrataPlaceholderDescriptorSpec
{
    uint32_t id;
    const char *name;
    uint32_t width;
    uint32_t class_type;
}
StrataPlaceholderDescriptorSpec;

static const uint8_t k_strata_placeholder_artifact_magic[STRATA_PLACEHOLDER_ARTIFACT_MAGIC_LEN] =
    { 0x46, 0x41, 0x52, 0x54 }; /* "FART" */

static const uint8_t k_strata_placeholder_payload_baseline[STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN] =
    { 0x53, 0x54, 0x42, 0x21 }; /* "STB!" */

static const uint8_t k_strata_placeholder_payload_advanced[STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN] =
    { 0x41, 0x44, 0x56, 0x21 }; /* "ADV!" */

static const uint8_t k_strata_placeholder_payload_native[STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN] =
    { 0x4E, 0x41, 0x54, 0x21 }; /* "NAT!" */

static const StrataPlaceholderDescriptorSpec k_strata_placeholder_input_descriptors[] =
{
    { 100u, "placeholder_in_0", 1u, 1u },
    { 101u, "placeholder_in_1", 8u, 1u }
};

static const StrataPlaceholderDescriptorSpec k_strata_placeholder_output_descriptors[] =
{
    { 200u, "placeholder_out_0", 1u, 2u },
    { 201u, "placeholder_out_1", 32u, 2u }
};

static const StrataPlaceholderDescriptorSpec k_strata_placeholder_probe_descriptors[] =
{
    { 300u, "placeholder_probe_0", 1u, 3u }
};

static inline size_t
strata_placeholder_artifact_size(void)
{
    return sizeof(StrataPlaceholderArtifactHeader) +
        STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN;
}

static inline uint32_t
strata_placeholder_input_descriptor_count(void)
{
    return (uint32_t)(sizeof(k_strata_placeholder_input_descriptors) /
        sizeof(k_strata_placeholder_input_descriptors[0]));
}

static inline uint32_t
strata_placeholder_output_descriptor_count(void)
{
    return (uint32_t)(sizeof(k_strata_placeholder_output_descriptors) /
        sizeof(k_strata_placeholder_output_descriptors[0]));
}

static inline uint32_t
strata_placeholder_probe_descriptor_count(void)
{
    return (uint32_t)(sizeof(k_strata_placeholder_probe_descriptors) /
        sizeof(k_strata_placeholder_probe_descriptors[0]));
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
strata_placeholder_expected_admission_info(
    StrataPlaceholderPayloadKind payload_kind,
    StrataPlaceholderAdmissionInfo* out_info)
{
    if (!out_info)
    {
        return 0;
    }

    out_info->requirement_flags = STRATA_PLACEHOLDER_REQUIREMENT_NONE;
    out_info->requires_advanced_controls = 0u;
    out_info->requires_native_state_read = 0u;
    out_info->requires_native_inputs = 0u;

    switch (payload_kind)
    {
        case STRATA_PLACEHOLDER_PAYLOAD_BASELINE:
            return 1;
        case STRATA_PLACEHOLDER_PAYLOAD_ADVANCED:
            out_info->requirement_flags =
                STRATA_PLACEHOLDER_REQUIREMENT_ADVANCED_CONTROL;
            out_info->requires_advanced_controls = 1u;
            return 1;
        case STRATA_PLACEHOLDER_PAYLOAD_NATIVE:
            out_info->requirement_flags =
                STRATA_PLACEHOLDER_REQUIREMENT_NATIVE_STATE;
            out_info->requires_native_state_read = 1u;
            out_info->requires_native_inputs = 1u;
            return 1;
        default:
            return 0;
    }
}

static inline int
strata_placeholder_admission_matches_payload_kind(
    StrataPlaceholderPayloadKind payload_kind,
    const StrataPlaceholderAdmissionInfo* info)
{
    StrataPlaceholderAdmissionInfo expected;

    if (!info || !strata_placeholder_expected_admission_info(payload_kind, &expected))
    {
        return 0;
    }

    return expected.requirement_flags == info->requirement_flags &&
        expected.requires_advanced_controls == info->requires_advanced_controls &&
        expected.requires_native_state_read == info->requires_native_state_read &&
        expected.requires_native_inputs == info->requires_native_inputs;
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
    const StrataPlaceholderAdmissionInfo* admission_info,
    size_t* out_size)
{
    StrataPlaceholderArtifactHeader header;
    const uint8_t* payload_bytes;
    size_t required_size;

    if (!buffer || !out_size || !admission_info ||
        backend_id == STRATA_PLACEHOLDER_BACKEND_ID_INVALID)
    {
        return 0;
    }

    payload_bytes = strata_placeholder_payload_bytes(payload_kind);
    if (!payload_bytes ||
        !strata_placeholder_admission_matches_payload_kind(payload_kind, admission_info))
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
    header.payload_kind = (uint32_t)payload_kind;
    header.admission_info = *admission_info;

    memcpy(buffer, &header, sizeof(header));
    memcpy(((uint8_t*)buffer) + sizeof(header), payload_bytes,
        STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN);

    *out_size = required_size;
    return 1;
}

#endif /* STRATA_PLACEHOLDER_ARTIFACT_H */
