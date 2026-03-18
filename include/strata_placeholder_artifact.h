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
#define STRATA_PLACEHOLDER_ARTIFACT_VERSION_MINOR 8u
#define STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN 4u
#define STRATA_PLACEHOLDER_DESCRIPTOR_NAME_CAPACITY 32u
#define STRATA_PLACEHOLDER_MODULE_NAME_CAPACITY 64u

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

typedef struct StrataPlaceholderDraftSummary
{
    uint32_t source_target_value;
    uint32_t has_placeholders;
    uint64_t approximate_size_bytes;
    uint64_t source_module_id;
    char source_module_name[STRATA_PLACEHOLDER_MODULE_NAME_CAPACITY];
}
StrataPlaceholderDraftSummary;

typedef struct StrataPlaceholderArtifactHeader
{
    uint8_t magic[4];
    uint16_t version_major;
    uint16_t version_minor;
    uint32_t target_backend_id;
    uint32_t input_descriptor_count;
    uint32_t output_descriptor_count;
    uint32_t probe_descriptor_count;
    uint32_t section_table_offset;
    uint32_t section_count;
    uint32_t descriptor_bytes;
    uint32_t descriptor_offset;
    uint32_t payload_size;
    uint32_t payload_offset;
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

typedef enum StrataPlaceholderSectionKind
{
    STRATA_PLACEHOLDER_SECTION_INVALID       = 0,
    STRATA_PLACEHOLDER_SECTION_ADMISSION     = 1,
    STRATA_PLACEHOLDER_SECTION_DRAFT_SUMMARY = 2,
    STRATA_PLACEHOLDER_SECTION_DESCRIPTORS   = 3,
    STRATA_PLACEHOLDER_SECTION_PAYLOAD       = 4
}
StrataPlaceholderSectionKind;

typedef struct StrataPlaceholderSectionEntry
{
    uint32_t section_kind;
    uint32_t section_offset;
    uint32_t section_size;
}
StrataPlaceholderSectionEntry;

typedef struct StrataPlaceholderDescriptorSpec
{
    uint32_t id;
    const char *name;
    uint32_t width;
    uint32_t class_type;
}
StrataPlaceholderDescriptorSpec;

typedef struct StrataPlaceholderSerializedDescriptor
{
    uint32_t id;
    uint32_t width;
    uint32_t class_type;
    uint32_t placeholder_flags;
    char name[STRATA_PLACEHOLDER_DESCRIPTOR_NAME_CAPACITY];
}
StrataPlaceholderSerializedDescriptor;

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
strata_placeholder_total_descriptor_count(
    uint32_t input_count,
    uint32_t output_count,
    uint32_t probe_count)
{
    return (size_t)input_count + (size_t)output_count + (size_t)probe_count;
}

static inline size_t
strata_placeholder_descriptor_bytes_for_counts(
    uint32_t input_count,
    uint32_t output_count,
    uint32_t probe_count)
{
    return strata_placeholder_total_descriptor_count(
        input_count,
        output_count,
        probe_count) * sizeof(StrataPlaceholderSerializedDescriptor);
}

static inline size_t
strata_placeholder_section_table_bytes(uint32_t section_count)
{
    return (size_t)section_count * sizeof(StrataPlaceholderSectionEntry);
}

static inline size_t
strata_placeholder_artifact_size_for_counts(
    uint32_t input_count,
    uint32_t output_count,
    uint32_t probe_count)
{
    return sizeof(StrataPlaceholderArtifactHeader) +
        strata_placeholder_section_table_bytes(4u) +
        sizeof(StrataPlaceholderAdmissionInfo) +
        sizeof(StrataPlaceholderDraftSummary) +
        strata_placeholder_descriptor_bytes_for_counts(
            input_count,
            output_count,
            probe_count) +
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

static inline size_t
strata_placeholder_artifact_size(void)
{
    return strata_placeholder_artifact_size_for_counts(
        strata_placeholder_input_descriptor_count(),
        strata_placeholder_output_descriptor_count(),
        strata_placeholder_probe_descriptor_count());
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
strata_placeholder_expected_draft_summary_for_backend(
    uint32_t backend_id,
    StrataPlaceholderDraftSummary* out_summary)
{
    if (!out_summary)
    {
        return 0;
    }

    out_summary->has_placeholders = 1u;
    out_summary->approximate_size_bytes = 1024u;
    out_summary->source_module_id = 0u;
    memset(out_summary->source_module_name, 0, sizeof(out_summary->source_module_name));

    switch (backend_id)
    {
        case STRATA_PLACEHOLDER_BACKEND_ID_LXS:
            out_summary->source_target_value = 1u;
            return 1;
        case STRATA_PLACEHOLDER_BACKEND_ID_HIGHZ:
            out_summary->source_target_value = 2u;
            return 1;
        default:
            out_summary->source_target_value = 0u;
            out_summary->has_placeholders = 0u;
            out_summary->approximate_size_bytes = 0u;
            return 0;
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

static inline void
strata_placeholder_fill_serialized_descriptor_from_spec(
    StrataPlaceholderSerializedDescriptor* out_descriptor,
    const StrataPlaceholderDescriptorSpec* spec)
{
    size_t copy_len;

    if (!out_descriptor || !spec)
    {
        return;
    }

    out_descriptor->id = spec->id;
    out_descriptor->width = spec->width;
    out_descriptor->class_type = spec->class_type;
    out_descriptor->placeholder_flags = 1u;
    memset(out_descriptor->name, 0, sizeof(out_descriptor->name));

    if (!spec->name)
    {
        return;
    }

    copy_len = strlen(spec->name);
    if (copy_len >= sizeof(out_descriptor->name))
    {
        copy_len = sizeof(out_descriptor->name) - 1u;
    }

    memcpy(out_descriptor->name, spec->name, copy_len);
}

static inline void
strata_placeholder_fill_default_serialized_descriptors(
    StrataPlaceholderSerializedDescriptor* out_descriptors)
{
    uint32_t index;
    uint32_t output_offset;
    uint32_t probe_offset;

    if (!out_descriptors)
    {
        return;
    }

    for (index = 0u; index < strata_placeholder_input_descriptor_count(); ++index)
    {
        strata_placeholder_fill_serialized_descriptor_from_spec(
            &out_descriptors[index],
            &k_strata_placeholder_input_descriptors[index]);
    }

    output_offset = strata_placeholder_input_descriptor_count();
    for (index = 0u; index < strata_placeholder_output_descriptor_count(); ++index)
    {
        strata_placeholder_fill_serialized_descriptor_from_spec(
            &out_descriptors[output_offset + index],
            &k_strata_placeholder_output_descriptors[index]);
    }

    probe_offset = output_offset + strata_placeholder_output_descriptor_count();
    for (index = 0u; index < strata_placeholder_probe_descriptor_count(); ++index)
    {
        strata_placeholder_fill_serialized_descriptor_from_spec(
            &out_descriptors[probe_offset + index],
            &k_strata_placeholder_probe_descriptors[index]);
    }
}

static inline const uint8_t*
strata_placeholder_find_section_data(
    const StrataPlaceholderArtifactHeader* header,
    StrataPlaceholderSectionKind section_kind)
{
    const StrataPlaceholderSectionEntry* sections;
    uint32_t index;

    if (!header)
    {
        return NULL;
    }

    sections = (const StrataPlaceholderSectionEntry*)
        (((const uint8_t*)header) + header->section_table_offset);

    for (index = 0u; index < header->section_count; ++index)
    {
        if (sections[index].section_kind == (uint32_t)section_kind)
        {
            return
                (((const uint8_t*)header) + sections[index].section_offset);
        }
    }

    return NULL;
}

static inline const StrataPlaceholderSectionEntry*
strata_placeholder_find_section_entry(
    const StrataPlaceholderArtifactHeader* header,
    StrataPlaceholderSectionKind section_kind)
{
    const StrataPlaceholderSectionEntry* sections;
    uint32_t index;

    if (!header)
    {
        return NULL;
    }

    sections = (const StrataPlaceholderSectionEntry*)
        (((const uint8_t*)header) + header->section_table_offset);

    for (index = 0u; index < header->section_count; ++index)
    {
        if (sections[index].section_kind == (uint32_t)section_kind)
        {
            return &sections[index];
        }
    }

    return NULL;
}

static inline const StrataPlaceholderSerializedDescriptor*
strata_placeholder_artifact_descriptors(
    const StrataPlaceholderArtifactHeader* header)
{
    return (const StrataPlaceholderSerializedDescriptor*)
        strata_placeholder_find_section_data(
        header,
        STRATA_PLACEHOLDER_SECTION_DESCRIPTORS);
}

static inline const uint8_t*
strata_placeholder_artifact_payload(
    const StrataPlaceholderArtifactHeader* header)
{
    return strata_placeholder_find_section_data(
        header,
        STRATA_PLACEHOLDER_SECTION_PAYLOAD);
}

static inline const StrataPlaceholderAdmissionInfo*
strata_placeholder_artifact_admission_info(
    const StrataPlaceholderArtifactHeader* header)
{
    return (const StrataPlaceholderAdmissionInfo*)
        strata_placeholder_find_section_data(
            header,
            STRATA_PLACEHOLDER_SECTION_ADMISSION);
}

static inline const StrataPlaceholderDraftSummary*
strata_placeholder_artifact_draft_summary(
    const StrataPlaceholderArtifactHeader* header)
{
    return (const StrataPlaceholderDraftSummary*)
        strata_placeholder_find_section_data(
            header,
            STRATA_PLACEHOLDER_SECTION_DRAFT_SUMMARY);
}

static inline int
strata_placeholder_artifact_write_with_descriptors(
    void* buffer,
    size_t buffer_size,
    uint32_t backend_id,
    StrataPlaceholderPayloadKind payload_kind,
    const StrataPlaceholderAdmissionInfo* admission_info,
    const StrataPlaceholderDraftSummary* draft_summary,
    const StrataPlaceholderSerializedDescriptor* descriptors,
    uint32_t input_descriptor_count,
    uint32_t output_descriptor_count,
    uint32_t probe_descriptor_count,
    size_t* out_size)
{
    StrataPlaceholderArtifactHeader header;
    StrataPlaceholderSectionEntry sections[4];
    const uint8_t* payload_bytes;
    uint32_t admission_offset;
    uint32_t draft_summary_offset;
    size_t descriptor_bytes;
    size_t section_table_bytes;
    size_t required_size;

    if (!buffer || !out_size || !admission_info || !draft_summary || !descriptors ||
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

    descriptor_bytes = strata_placeholder_descriptor_bytes_for_counts(
        input_descriptor_count,
        output_descriptor_count,
        probe_descriptor_count);
    section_table_bytes = strata_placeholder_section_table_bytes(4u);
    required_size = sizeof(StrataPlaceholderArtifactHeader) +
        section_table_bytes +
        sizeof(StrataPlaceholderAdmissionInfo) +
        sizeof(StrataPlaceholderDraftSummary) +
        descriptor_bytes +
        STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN;

    if (buffer_size < required_size)
    {
        return 0;
    }

    memcpy(header.magic, k_strata_placeholder_artifact_magic, sizeof(header.magic));
    header.version_major = STRATA_PLACEHOLDER_ARTIFACT_VERSION_MAJOR;
    header.version_minor = STRATA_PLACEHOLDER_ARTIFACT_VERSION_MINOR;
    header.target_backend_id = backend_id;
    header.input_descriptor_count = input_descriptor_count;
    header.output_descriptor_count = output_descriptor_count;
    header.probe_descriptor_count = probe_descriptor_count;
    header.section_table_offset = (uint32_t)sizeof(StrataPlaceholderArtifactHeader);
    header.section_count = 4u;
    header.descriptor_bytes = (uint32_t)descriptor_bytes;
    admission_offset = (uint32_t)(sizeof(StrataPlaceholderArtifactHeader) +
        section_table_bytes);
    draft_summary_offset = (uint32_t)(admission_offset +
        sizeof(StrataPlaceholderAdmissionInfo));
    header.descriptor_offset = (uint32_t)(draft_summary_offset +
        sizeof(StrataPlaceholderDraftSummary));
    header.payload_size = STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN;
    header.payload_offset = (uint32_t)(header.descriptor_offset + descriptor_bytes);
    header.payload_kind = (uint32_t)payload_kind;
    header.admission_info = *admission_info;

    sections[0].section_kind = STRATA_PLACEHOLDER_SECTION_ADMISSION;
    sections[0].section_offset = admission_offset;
    sections[0].section_size = sizeof(StrataPlaceholderAdmissionInfo);
    sections[1].section_kind = STRATA_PLACEHOLDER_SECTION_DRAFT_SUMMARY;
    sections[1].section_offset = draft_summary_offset;
    sections[1].section_size = sizeof(StrataPlaceholderDraftSummary);
    sections[2].section_kind = STRATA_PLACEHOLDER_SECTION_DESCRIPTORS;
    sections[2].section_offset = header.descriptor_offset;
    sections[2].section_size = header.descriptor_bytes;
    sections[3].section_kind = STRATA_PLACEHOLDER_SECTION_PAYLOAD;
    sections[3].section_offset = header.payload_offset;
    sections[3].section_size = header.payload_size;

    memcpy(buffer, &header, sizeof(header));
    memcpy(
        ((uint8_t*)buffer) + header.section_table_offset,
        sections,
        section_table_bytes);
    memcpy(
        ((uint8_t*)buffer) + admission_offset,
        admission_info,
        sizeof(StrataPlaceholderAdmissionInfo));
    memcpy(
        ((uint8_t*)buffer) + draft_summary_offset,
        draft_summary,
        sizeof(StrataPlaceholderDraftSummary));
    memcpy(
        ((uint8_t*)buffer) + header.descriptor_offset,
        descriptors,
        descriptor_bytes);
    memcpy(
        ((uint8_t*)buffer) + header.payload_offset,
        payload_bytes,
        STRATA_PLACEHOLDER_ARTIFACT_PAYLOAD_LEN);

    *out_size = required_size;
    return 1;
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
    StrataPlaceholderDraftSummary draft_summary;
    StrataPlaceholderSerializedDescriptor descriptors[
        sizeof(k_strata_placeholder_input_descriptors) /
        sizeof(k_strata_placeholder_input_descriptors[0]) +
        sizeof(k_strata_placeholder_output_descriptors) /
        sizeof(k_strata_placeholder_output_descriptors[0]) +
        sizeof(k_strata_placeholder_probe_descriptors) /
        sizeof(k_strata_placeholder_probe_descriptors[0])];

    strata_placeholder_fill_default_serialized_descriptors(descriptors);
    if (!strata_placeholder_expected_draft_summary_for_backend(
        backend_id,
        &draft_summary))
    {
        return 0;
    }

    return strata_placeholder_artifact_write_with_descriptors(
        buffer,
        buffer_size,
        backend_id,
        payload_kind,
        admission_info,
        &draft_summary,
        descriptors,
        strata_placeholder_input_descriptor_count(),
        strata_placeholder_output_descriptor_count(),
        strata_placeholder_probe_descriptor_count(),
        out_size);
}

#endif /* STRATA_PLACEHOLDER_ARTIFACT_H */
