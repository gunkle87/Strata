/*
 * test_backend_enum.c
 *
 * Verify that backend enumeration returns at least two entries
 * and that each index yields a valid non-zero backend ID.
 */

#include <stdio.h>
#include "../../include/forge_api.h"

int main(void)
{
    uint32_t       count;
    uint32_t       i;
    ForgeBackendId id;
    ForgeResult    result;

    count = forge_backend_count();

    if (count < 2)
    {
        fprintf(stderr, "FAIL: expected at least 2 backends, got %u\n", count);
        return 1;
    }

    for (i = 0; i < count; i++)
    {
        id     = FORGE_BACKEND_ID_INVALID;
        result = forge_backend_id_at(i, &id);

        if (result != FORGE_OK)
        {
            fprintf(stderr, "FAIL: forge_backend_id_at(%u) returned %d\n", i, (int)result);
            return 1;
        }

        if (id == FORGE_BACKEND_ID_INVALID)
        {
            fprintf(stderr, "FAIL: index %u returned invalid ID\n", i);
            return 1;
        }
    }

    /* Out-of-bounds index must fail. */
    result = forge_backend_id_at(count, &id);

    if (result != FORGE_ERR_OUT_OF_BOUNDS)
    {
        fprintf(stderr,
            "FAIL: out-of-bounds index expected FORGE_ERR_OUT_OF_BOUNDS, got %d\n",
            (int)result);
        return 1;
    }

    /* NULL out-param must fail. */
    result = forge_backend_id_at(0, NULL);

    if (result != FORGE_ERR_INVALID_ARGUMENT)
    {
        fprintf(stderr,
            "FAIL: NULL out_id expected FORGE_ERR_INVALID_ARGUMENT, got %d\n",
            (int)result);
        return 1;
    }

    printf("PASS: test_backend_enum\n");
    return 0;
}
