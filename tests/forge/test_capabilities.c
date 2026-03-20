/*
 * test_capabilities.c
 *
 * Verify that forge_backend_capabilities returns structured capability data
 * and that LXS and HighZ still differ where expected.
 */

#include <stdio.h>
#include "../../include/forge_api.h"
#include "../../include/forge_capabilities.h"

int main(void)
{
    uint32_t          count;
    uint32_t          i;
    ForgeBackendId    id;
    ForgeBackendInfo  info;
    ForgeCapabilities caps;
    ForgeResult       result;

    count = forge_backend_count();

    for (i = 0; i < count; i++)
    {
        result = forge_backend_id_at(i, &id);

        if (result != FORGE_OK)
        {
            fprintf(stderr, "FAIL: forge_backend_id_at(%u) returned %d\n", i, (int)result);
            return 1;
        }

        result = forge_backend_capabilities(id, &caps);

        if (result != FORGE_OK)
        {
            fprintf(stderr,
                "FAIL: forge_backend_capabilities for id %u returned %d\n",
                (unsigned)id, (int)result);
            return 1;
        }

        if (caps.lifecycle.artifact_load != FORGE_SUPPORT_FULL ||
            caps.lifecycle.session_create != FORGE_SUPPORT_FULL ||
            caps.lifecycle.session_reset != FORGE_SUPPORT_FULL ||
            caps.lifecycle.session_destroy != FORGE_SUPPORT_FULL ||
            caps.lifecycle.lifecycle_query != FORGE_SUPPORT_FULL)
        {
            fprintf(stderr,
                "FAIL: backend %u does not report full lifecycle support\n",
                (unsigned)id);
            return 1;
        }

        if (caps.common_single_step_advance != FORGE_SUPPORT_FULL ||
            caps.common_multi_step_advance != FORGE_SUPPORT_FULL)
        {
            fprintf(stderr,
                "FAIL: backend %u does not report expected common advancement support\n",
                (unsigned)id);
            return 1;
        }

        if (caps.reads.output_read == FORGE_SUPPORT_NONE ||
            caps.reads.descriptor_enumeration == FORGE_SUPPORT_NONE ||
            caps.reads.name_lookup == FORGE_SUPPORT_NONE ||
            caps.reads.id_lookup == FORGE_SUPPORT_NONE)
        {
            fprintf(stderr,
                "FAIL: backend %u reports missing required common read support\n",
                (unsigned)id);
            return 1;
        }

        if (caps.extension_family_count > FORGE_MAX_EXTENSION_FAMILIES)
        {
            fprintf(stderr,
                "FAIL: backend %u extension_family_count %u exceeds max %u\n",
                (unsigned)id,
                caps.extension_family_count,
                FORGE_MAX_EXTENSION_FAMILIES);
            return 1;
        }
    }

    result = forge_backend_id_at(0, &id);

    if (result == FORGE_OK)
    {
        result = forge_backend_info(id, &info);

        if (result == FORGE_OK && info.state_model == FORGE_STATE_MODEL_4STATE)
        {
            result = forge_backend_capabilities(id, &caps);

            if (result == FORGE_OK && caps.temporal_substep != 0)
            {
                fprintf(stderr,
                    "FAIL: 4-state backend should have temporal_substep == 0\n");
                return 1;
            }
        }
    }

    result = forge_backend_id_at(1, &id);

    if (result == FORGE_OK)
    {
        result = forge_backend_info(id, &info);

        if (result == FORGE_OK && info.state_model == FORGE_STATE_MODEL_7STATE)
        {
            result = forge_backend_capabilities(id, &caps);

            if (result == FORGE_OK && caps.temporal_substep == 0)
            {
                fprintf(stderr,
                    "FAIL: 7-state backend should have temporal_substep != 0\n");
                return 1;
            }
        }
    }

    result = forge_backend_id_at(0, &id);

    if (result == FORGE_OK)
    {
        result = forge_backend_capabilities(id, NULL);

        if (result != FORGE_ERR_INVALID_ARGUMENT)
        {
            fprintf(stderr,
                "FAIL: NULL out_caps expected FORGE_ERR_INVALID_ARGUMENT, got %d\n",
                (int)result);
            return 1;
        }
    }

    result = forge_backend_capabilities((ForgeBackendId)0xFFFF, &caps);

    if (result != FORGE_ERR_BACKEND_UNAVAILABLE)
    {
        fprintf(stderr,
            "FAIL: unknown backend expected FORGE_ERR_BACKEND_UNAVAILABLE, got %d\n",
            (int)result);
        return 1;
    }

    printf("PASS: test_capabilities\n");
    return 0;
}
