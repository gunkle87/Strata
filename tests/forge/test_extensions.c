/*
 * test_extensions.c
 *
 * Verify that explicit extension-family reporting and lookup boundaries
 * return accurate data matching the capabilities record.
 */

#include <stdio.h>
#include "../../include/forge_api.h"
#include "../../include/forge_capabilities.h"

int main(void)
{
    uint32_t count = 0;
    uint32_t ext_idx = 0;
    ForgeBackendId id;
    ForgeResult result;

    result = forge_backend_id_at(0, &id);
    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: Could not get backend 0\n");
        return 1;
    }

    /* Test 1: Count */
    result = forge_extension_family_count(id, &count);
    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: forge_extension_family_count returned %d\n", (int)result);
        return 1;
    }

    /* Test 2: Iterate over count */
    for (ext_idx = 0; ext_idx < count; ++ext_idx)
    {
        ForgeExtensionFamily family;
        uint32_t is_supported = 0;

        result = forge_extension_family_at(id, ext_idx, &family);
        if (result != FORGE_OK)
        {
            fprintf(stderr, "FAIL: forge_extension_family_at(%u) returned %d\n", ext_idx, (int)result);
            return 1;
        }

        /* Test 3: Explicit support lookup */
        result = forge_backend_supports_extension(id, family, &is_supported);
        if (result != FORGE_OK)
        {
            fprintf(stderr, "FAIL: forge_backend_supports_extension returned %d\n", (int)result);
            return 1;
        }

        if (is_supported == 0)
        {
            fprintf(stderr, "FAIL: Family %u reported by _at but not by _supports_extension\n", (unsigned)family);
            return 1;
        }
    }

    /* Test 4: Query invalid extension (assuming 0xFFFF is not a valid extension family) */
    {
        uint32_t is_supported = 1;
        result = forge_backend_supports_extension(id, (ForgeExtensionFamily)0xFFFF, &is_supported);
        if (result != FORGE_OK)
        {
            fprintf(stderr, "FAIL: forge_backend_supports_extension for unknown family returned %d\n", (int)result);
            return 1;
        }

        if (is_supported != 0)
        {
            fprintf(stderr, "FAIL: Unknown extension family reported as supported!\n");
            return 1;
        }
    }

    printf("PASS: test_extensions\n");
    return 0;
}
