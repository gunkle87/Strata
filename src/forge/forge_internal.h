#ifndef FORGE_INTERNAL_H
#define FORGE_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

#include "forge_types.h"

struct ForgeArtifact
{
    ForgeBackendId backend_id;
    size_t source_size;
    uint32_t placeholder_flags;
};

struct ForgeSession
{
    ForgeArtifact *artifact;
    uint32_t placeholder_state;
};

#endif /* FORGE_INTERNAL_H */
