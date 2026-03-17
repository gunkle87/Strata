#ifndef FORGE_REGISTRY_H
#define FORGE_REGISTRY_H

#include <stdint.h>
#include "../../include/forge_types.h"
#include "../../include/forge_capabilities.h"

/*
 * forge_registry.h
 *
 * Internal-only header for the Forge backend registry.
 * Do not include this from public headers or test files.
 *
 * ForgeBackendRecord combines backend identity and capability data in one
 * static record so both can be looked up by ID in a single pass.
 */

typedef struct ForgeBackendRecord
{
    ForgeBackendId    id;
    ForgeBackendInfo  info;
    ForgeCapabilities capabilities;

}
ForgeBackendRecord;

uint32_t                  forge_registry_backend_count(void);
const ForgeBackendRecord *forge_registry_backend_at(uint32_t index);
const ForgeBackendRecord *forge_registry_backend_by_id(ForgeBackendId backend_id);

#endif /* FORGE_REGISTRY_H */
