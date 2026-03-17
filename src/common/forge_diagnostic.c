#include "forge_diagnostic.h"

/*
 * forge_diagnostic.c
 *
 * Minimal diagnostic string store for Forge.
 * Stores a pointer to the last error message set by any Forge call.
 * The stored pointer must have static lifetime.
 */

static const char *s_last_message = "";

void
forge_diag_set(const char *msg)
{
    if (msg)
    {
        s_last_message = msg;
    }
    else
    {
        s_last_message = "";
    }
}

const char *
forge_diag_get(void)
{
    return s_last_message;
}
