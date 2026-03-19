#include "../../include/strata_placeholder_artifact.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
fail(const char* message)
{
    fprintf(stderr, "FAIL: %s\n", message);
    exit(1);
}

int
main(void)
{
    uint8_t artifact_bytes[
        sizeof(StrataPlaceholderArtifactHeader) +
        sizeof(StrataPlaceholderFastExecutablePayloadHeader) +
        (3u * sizeof(StrataPlaceholderFastSignalRecord)) +
        (2u * sizeof(StrataPlaceholderFastPrimitiveRecord)) +
        sizeof(StrataPlaceholderFastInputBinding) +
        sizeof(StrataPlaceholderFastOutputBinding)];
    StrataPlaceholderArtifactHeader* artifact_header;
    StrataPlaceholderFastExecutablePayloadHeader* payload_header;
    StrataPlaceholderFastSignalRecord* signals;
    StrataPlaceholderFastPrimitiveRecord* primitives;
    StrataPlaceholderFastInputBinding* input_bindings;
    StrataPlaceholderFastOutputBinding* output_bindings;
    const StrataPlaceholderFastExecutablePayloadHeader* read_payload_header;

    if (!strata_placeholder_payload_kind_is_stub(
            STRATA_PLACEHOLDER_PAYLOAD_BASELINE) ||
        strata_placeholder_payload_kind_is_stub(
            STRATA_PLACEHOLDER_PAYLOAD_FAST_EXECUTABLE_V1))
    {
        fail("payload kind classification mismatch");
    }

    if (!strata_placeholder_payload_kind_is_real_fast_executable(
            STRATA_PLACEHOLDER_PAYLOAD_FAST_EXECUTABLE_V1) ||
        strata_placeholder_payload_kind_is_real_fast_executable(
            STRATA_PLACEHOLDER_PAYLOAD_ADVANCED))
    {
        fail("real fast payload kind classification mismatch");
    }

    if (strata_placeholder_fast_payload_bytes_for_counts(2u, 3u, 1u, 1u) !=
        sizeof(StrataPlaceholderFastExecutablePayloadHeader) +
            (3u * sizeof(StrataPlaceholderFastSignalRecord)) +
            (2u * sizeof(StrataPlaceholderFastPrimitiveRecord)) +
            sizeof(StrataPlaceholderFastInputBinding) +
            sizeof(StrataPlaceholderFastOutputBinding))
    {
        fail("fast payload byte sizing mismatch");
    }

    memset(artifact_bytes, 0, sizeof(artifact_bytes));
    artifact_header = (StrataPlaceholderArtifactHeader*)artifact_bytes;
    memcpy(
        artifact_header->magic,
        k_strata_placeholder_artifact_magic,
        sizeof(artifact_header->magic));
    artifact_header->version_major = STRATA_PLACEHOLDER_ARTIFACT_VERSION_MAJOR;
    artifact_header->version_minor = STRATA_PLACEHOLDER_ARTIFACT_VERSION_MINOR;
    artifact_header->target_backend_id = STRATA_PLACEHOLDER_BACKEND_ID_LXS;
    artifact_header->input_descriptor_count = 1u;
    artifact_header->output_descriptor_count = 1u;
    artifact_header->probe_descriptor_count = 0u;
    artifact_header->payload_kind = STRATA_PLACEHOLDER_PAYLOAD_FAST_EXECUTABLE_V1;
    artifact_header->payload_offset = (uint32_t)sizeof(StrataPlaceholderArtifactHeader);
    artifact_header->payload_size =
        (uint32_t)strata_placeholder_fast_payload_bytes_for_counts(2u, 3u, 1u, 1u);

    payload_header = (StrataPlaceholderFastExecutablePayloadHeader*)
        (artifact_bytes + artifact_header->payload_offset);
    payload_header->primitive_count = 2u;
    payload_header->signal_count = 3u;
    payload_header->input_binding_count = 1u;
    payload_header->output_binding_count = 1u;

    signals = (StrataPlaceholderFastSignalRecord*)
        (((uint8_t*)payload_header) + sizeof(*payload_header));
    signals[0].source_kind = STRATA_FAST_SIGNAL_SOURCE_MODULE_INPUT;
    signals[0].source_record_index = 0u;
    signals[0].source_output_slot = 0u;
    signals[1].source_kind = STRATA_FAST_SIGNAL_SOURCE_PRIMITIVE_OUTPUT;
    signals[1].source_record_index = 0u;
    signals[1].source_output_slot = 0u;
    signals[2].source_kind = STRATA_FAST_SIGNAL_SOURCE_PRIMITIVE_OUTPUT;
    signals[2].source_record_index = 1u;
    signals[2].source_output_slot = 0u;

    primitives = (StrataPlaceholderFastPrimitiveRecord*)(signals + payload_header->signal_count);
    primitives[0].opcode = STRATA_FAST_EXEC_OPCODE_NOT;
    primitives[0].input_signal_index_a = 0u;
    primitives[0].input_signal_index_b = STRATA_PLACEHOLDER_FAST_EXECUTABLE_UNUSED_INDEX;
    primitives[0].output_signal_index = 1u;
    primitives[1].opcode = STRATA_FAST_EXEC_OPCODE_BUF;
    primitives[1].input_signal_index_a = 1u;
    primitives[1].input_signal_index_b = STRATA_PLACEHOLDER_FAST_EXECUTABLE_UNUSED_INDEX;
    primitives[1].output_signal_index = 2u;

    input_bindings = (StrataPlaceholderFastInputBinding*)(primitives + payload_header->primitive_count);
    input_bindings[0].descriptor_id = 1000u;
    input_bindings[0].signal_index = 0u;

    output_bindings = (StrataPlaceholderFastOutputBinding*)(input_bindings + payload_header->input_binding_count);
    output_bindings[0].descriptor_id = 2000u;
    output_bindings[0].signal_index = 2u;

    read_payload_header = strata_placeholder_fast_payload_header(artifact_header);
    if (!read_payload_header)
    {
        fail("fast payload header accessor returned NULL");
    }

    if (strata_placeholder_fast_payload_signals(read_payload_header) != signals ||
        strata_placeholder_fast_payload_primitives(read_payload_header) != primitives ||
        strata_placeholder_fast_payload_input_bindings(read_payload_header) != input_bindings ||
        strata_placeholder_fast_payload_output_bindings(read_payload_header) != output_bindings)
    {
        fail("fast payload accessors returned unexpected pointers");
    }

    if (!strata_placeholder_fast_payload_header_is_coherent(artifact_header))
    {
        fail("coherent fast payload rejected");
    }

    artifact_header->probe_descriptor_count = 1u;
    if (strata_placeholder_fast_payload_header_is_coherent(artifact_header))
    {
        fail("probe-bearing fast payload should reject");
    }
    artifact_header->probe_descriptor_count = 0u;

    artifact_header->payload_size -= 1u;
    if (strata_placeholder_fast_payload_header_is_coherent(artifact_header))
    {
        fail("short payload size should reject");
    }
    artifact_header->payload_size += 1u;

    artifact_header->target_backend_id = STRATA_PLACEHOLDER_BACKEND_ID_HIGHZ;
    if (strata_placeholder_fast_payload_header_is_coherent(artifact_header))
    {
        fail("non-fast backend payload should reject");
    }
    artifact_header->target_backend_id = STRATA_PLACEHOLDER_BACKEND_ID_LXS;

    primitives[1].opcode = 99u;
    if (strata_placeholder_fast_payload_header_is_coherent(artifact_header))
    {
        fail("invalid opcode should reject");
    }

    printf("PASS: test_executable_payload_contract\n");
    return 0;
}
