#ifndef FORGE_INTERNAL_H
#define FORGE_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

#include "forge_policy.h"
#include "forge_types.h"

typedef struct ForgeArtifactHeader
{
    uint8_t magic[4];
    uint16_t version_major;
    uint16_t version_minor;
    uint32_t target_backend_id;
    uint32_t payload_size;
}
ForgeArtifactHeader;

struct ForgeArtifact
{
    ForgeBackendId backend_id;
    uint16_t format_version_major;
    uint16_t format_version_minor;
    uint32_t payload_size;
    size_t source_size;
    uint32_t placeholder_flags;
    ForgeEffectiveProfile effective_profile;
};

struct ForgeSession
{
    ForgeArtifact *artifact;
    uint32_t placeholder_state;
    ForgeEffectiveProfile effective_profile;
};

#endif /* FORGE_INTERNAL_H */
