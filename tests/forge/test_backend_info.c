/*
 * test_backend_info.c
 *
 * Verify that forge_backend_info fills a meaningful ForgeBackendInfo struct
 * for each registered backend, and that unknown IDs are rejected.
 */

#include <stdio.h>
#include <string.h>
#include "../../include/forge_api.h"

int main(void)
{
    uint32_t        count;
    uint32_t        i;
    ForgeBackendId  id;
    ForgeBackendInfo info;
    ForgeResult     result;

    count = forge_backend_count();

    for (i = 0; i < count; i++)
    {
        result = forge_backend_id_at(i, &id);

        if (result != FORGE_OK)
        {
            fprintf(stderr, "FAIL: forge_backend_id_at(%u) returned %d\n", i, (int)result);
            return 1;
        }

        memset(&info, 0, sizeof(info));
        result = forge_backend_info(id, &info);

        if (result != FORGE_OK)
        {
            fprintf(stderr,
                "FAIL: forge_backend_info for id %u returned %d\n",
                (unsigned)id, (int)result);
            return 1;
        }

        if (!info.name || info.name[0] == '\0')
        {
            fprintf(stderr, "FAIL: backend %u has empty name\n", (unsigned)id);
            return 1;
        }

        if (!info.description || info.description[0] == '\0')
        {
            fprintf(stderr, "FAIL: backend %u has empty description\n", (unsigned)id);
            return 1;
        }

        if ((int)info.backend_class < 0 || (int)info.state_model < 0)
        {
            fprintf(stderr, "FAIL: backend %u has invalid enum values\n", (unsigned)id);
            return 1;
        }
    }

    /* NULL out-param must be rejected. */
    result = forge_backend_id_at(0, &id);

    if (result == FORGE_OK)
    {
        result = forge_backend_info(id, NULL);

        if (result != FORGE_ERR_INVALID_ARGUMENT)
        {
            fprintf(stderr,
                "FAIL: NULL out_info expected FORGE_ERR_INVALID_ARGUMENT, got %d\n",
                (int)result);
            return 1;
        }
    }

    /* Unknown backend ID must be rejected. */
    result = forge_backend_info((ForgeBackendId)0xFFFF, &info);

    if (result != FORGE_ERR_BACKEND_UNAVAILABLE)
    {
        fprintf(stderr,
            "FAIL: unknown backend ID expected FORGE_ERR_BACKEND_UNAVAILABLE, got %d\n",
            (int)result);
        return 1;
    }

    printf("PASS: test_backend_info\n");
    return 0;
}
