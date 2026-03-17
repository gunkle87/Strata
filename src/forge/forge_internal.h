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
    uint32_t payload_size;
    size_t source_size;
    uint32_t placeholder_flags;
    uint32_t required_extension_mask;
    uint32_t requires_advanced_controls;
    uint32_t requires_native_state_read;
    uint32_t requires_native_inputs;
    ForgeEffectiveProfile effective_profile;
};

struct ForgeSession
{
    ForgeArtifact *artifact;
    uint32_t placeholder_state;
    ForgeEffectiveProfile effective_profile;
};

#endif /* FORGE_INTERNAL_H */
