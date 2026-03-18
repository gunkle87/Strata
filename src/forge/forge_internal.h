#ifndef FORGE_INTERNAL_H
#define FORGE_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

#include "../../include/strata_placeholder_artifact.h"
#include "forge_policy.h"
#include "forge_types.h"

typedef StrataPlaceholderArtifactHeader ForgeArtifactHeader;

struct ForgeArtifact
{
    ForgeBackendId backend_id;
    uint16_t format_version_major;
    uint16_t format_version_minor;
    uint32_t source_target_value;
    uint32_t source_has_placeholders;
    uint64_t source_approximate_size_bytes;
    uint64_t source_module_id;
    char source_module_name[STRATA_PLACEHOLDER_MODULE_NAME_CAPACITY];
    uint32_t source_declared_component_count;
    uint32_t source_declared_connection_count;
    uint32_t source_declared_stateful_node_count;
    uint32_t payload_size;
    uint32_t input_descriptor_count;
    uint32_t output_descriptor_count;
    uint32_t probe_descriptor_count;
    size_t source_size;
    uint32_t placeholder_flags;
    uint32_t required_extension_mask;
    uint32_t requires_advanced_controls;
    uint32_t requires_native_state_read;
    uint32_t requires_native_inputs;
    StrataPlaceholderSerializedDescriptor *descriptors;
    uint32_t structure_component_count;
    uint32_t structure_connection_count;
    StrataPlaceholderSerializedComponent *components;
    StrataPlaceholderSerializedConnection *connections;
    ForgeEffectiveProfile effective_profile;
};

struct ForgeSession
{
    ForgeArtifact *artifact;
    uint32_t placeholder_state;
    ForgeEffectiveProfile effective_profile;
};

#endif /* FORGE_INTERNAL_H */
