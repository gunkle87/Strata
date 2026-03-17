#ifndef FORGE_DIAGNOSTIC_H
#define FORGE_DIAGNOSTIC_H

/*
 * forge_diagnostic.h
 *
 * Internal-only diagnostic string helpers for Forge.
 * Do not include this from public headers or test files.
 * Public callers use forge_last_error_string() from forge_api.h.
 */

/*
 * forge_diag_set
 *
 * Sets the current diagnostic message. msg must be a string literal or
 * otherwise have static lifetime — the pointer is stored, not copied.
 */
void forge_diag_set(const char *msg);

/*
 * forge_diag_get
 *
 * Returns the most recently set diagnostic message.
 * Returns an empty string if none has been set.
 * Never returns NULL.
 */
const char *forge_diag_get(void);

#endif /* FORGE_DIAGNOSTIC_H */
