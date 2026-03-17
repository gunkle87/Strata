/*
 * test_artifact_stub.c
 *
 * Verify the stub artifact load path:
 * - Known backend + stub magic bytes -> FORGE_OK, non-NULL handle.
 * - forge_artifact_unload frees it cleanly.
 * - Unknown backend ID is rejected.
 * - Non-stub data is rejected with FORGE_ERR_UNSUPPORTED.
 */

#include <stdio.h>
#include "../../include/forge_api.h"

/* Must match the magic bytes in forge_api.c. */
static const unsigned char k_stub_magic[] = { 0x53, 0x54, 0x42, 0x21 };

int main(void)
{
    ForgeBackendId  id   = FORGE_BACKEND_ID_INVALID;
    ForgeArtifact  *art  = NULL;
    ForgeResult     result;

    result = forge_backend_id_at(0, &id);

    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not get backend id at index 0: %d\n", (int)result);
        return 1;
    }

    /* Stub magic path: should return FORGE_OK and a non-NULL handle. */
    result = forge_artifact_load(id, k_stub_magic, sizeof(k_stub_magic), &art);

    if (result != FORGE_OK)
    {
        fprintf(stderr,
            "FAIL: stub load expected FORGE_OK, got %d (%s)\n",
            (int)result, forge_last_error_string());
        return 1;
    }

    if (!art)
    {
        fprintf(stderr, "FAIL: stub load returned FORGE_OK but out_artifact is NULL\n");
        return 1;
    }

    /* Unload must succeed. */
    result = forge_artifact_unload(art);
    art = NULL;

    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: forge_artifact_unload returned %d\n", (int)result);
        return 1;
    }

    /* Unknown backend ID must be rejected before reaching the data check. */
    result = forge_artifact_load(
        (ForgeBackendId)0xFFFF, k_stub_magic, sizeof(k_stub_magic), &art);

    if (result != FORGE_ERR_BACKEND_UNAVAILABLE)
    {
        fprintf(stderr,
            "FAIL: unknown backend expected FORGE_ERR_BACKEND_UNAVAILABLE, got %d\n",
            (int)result);
        return 1;
    }

    /* Non-stub data with a known backend must be rejected as unsupported. */
    {
        const unsigned char bad_data[] = { 0x00, 0x01, 0x02, 0x03 };

        result = forge_artifact_load(id, bad_data, sizeof(bad_data), &art);

        if (result != FORGE_ERR_UNSUPPORTED)
        {
            fprintf(stderr,
                "FAIL: non-stub data expected FORGE_ERR_UNSUPPORTED, got %d\n",
                (int)result);
            return 1;
        }
    }

    printf("PASS: test_artifact_stub\n");
    return 0;
}
