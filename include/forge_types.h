#ifndef FORGE_TYPES_H
#define FORGE_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include "strata_projection.h"

/*
 * forge_types.h
 *
 * Opaque public types for the Forge runtime boundary.
 * Public consumers must not depend on the internal layout of any of these.
 * Internal implementation files may include forge_internal_types.h for
 * the concrete definitions when they exist.
 */

/*
 * ForgeBackendId
 *
 * Stable numeric identity for a registered backend.
 * Returned by forge_backend_id_at() and used by all backend query calls.
 * Valid IDs are non-zero.
 */
typedef uint32_t ForgeBackendId;

/*
 * FORGE_BACKEND_ID_INVALID
 *
 * Sentinel value indicating an absent or unresolved backend ID.
 */
#define FORGE_BACKEND_ID_INVALID ((ForgeBackendId)0)

/*
 * ForgeProjectionMetadata
 *
 * Descriptive projection visibility surface exposed through Forge. This is a
 * reporting-only view of compiler-emitted metadata and does not imply any
 * runtime-side projection behavior.
 */
typedef struct ForgeProjectionMetadata
{
    uint32_t required_projection_families_mask;
    uint32_t lowered_projection_families_mask;
    uint32_t projection_occurred;
    uint32_t approximation_occurred;
    uint32_t reserved[4];
}
ForgeProjectionMetadata;

/*
 * ForgeArtifact
 *
 * Opaque handle to a loaded backend-targeted executable artifact.
 * Obtained from forge_artifact_load().
 * Must be released with forge_artifact_unload() when no longer needed.
 * Must not be accessed after unload.
 */
typedef struct ForgeArtifact ForgeArtifact;

/*
 * ForgeSession
 *
 * Opaque handle to a live runtime session.
 * Obtained from forge_session_create().
 * Must be released with forge_session_free() when no longer needed.
 */
typedef struct ForgeSession ForgeSession;

#endif /* FORGE_TYPES_H */
