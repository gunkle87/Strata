#include "forge_registry.h"

enum
{
    FORGE_BACKEND_ID_LXS = 1,
    FORGE_BACKEND_ID_HIGHZ = 2
};

static const ForgeBackendRecord s_backend_records[] =
{
    {
        FORGE_BACKEND_ID_LXS,
        {
            "LXS",
            "Fast 4-state zero-delay backend",
            0,
            1,
            FORGE_BACKEND_CLASS_FAST,
            FORGE_STATE_MODEL_4STATE
        },
        {
            FORGE_SUPPORT_FULL,
            FORGE_SUPPORT_FULL,
            FORGE_SUPPORT_FULL,
            FORGE_SUPPORT_FULL,
            FORGE_SUPPORT_FULL,
            0,
            0,
            2,
            {
                FORGE_EXT_PERFORMANCE_PROFILE,
                FORGE_EXT_RUNTIME_DIAGNOSTICS
            }
        }
    },
    {
        FORGE_BACKEND_ID_HIGHZ,
        {
            "HighZ",
            "Temporal 7-state 3-phase delta backend",
            0,
            1,
            FORGE_BACKEND_CLASS_TEMPORAL,
            FORGE_STATE_MODEL_7STATE
        },
        {
            FORGE_SUPPORT_FULL,
            FORGE_SUPPORT_FULL,
            FORGE_SUPPORT_PARTIAL,
            FORGE_SUPPORT_FULL,
            FORGE_SUPPORT_FULL,
            1,
            1,
            3,
            {
                FORGE_EXT_TEMPORAL_CONTROL,
                FORGE_EXT_NATIVE_STATE_READ,
                FORGE_EXT_RUNTIME_DIAGNOSTICS
            }
        }
    }
};

uint32_t
forge_registry_backend_count(void)
{
    return (uint32_t)(sizeof(s_backend_records) / sizeof(s_backend_records[0]));
}

const ForgeBackendRecord *
forge_registry_backend_at(uint32_t index)
{
    if (index >= forge_registry_backend_count())
    {
        return NULL;
    }

    return &s_backend_records[index];
}

const ForgeBackendRecord *
forge_registry_backend_by_id(ForgeBackendId backend_id)
{
    uint32_t index;

    for (index = 0; index < forge_registry_backend_count(); ++index)
    {
        if (s_backend_records[index].id == backend_id)
        {
            return &s_backend_records[index];
        }
    }

    return NULL;
}
