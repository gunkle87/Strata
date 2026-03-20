#ifndef STRATA_PROJECTION_H
#define STRATA_PROJECTION_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
 * strata_projection.h
 *
 * Shared data contracts for projection policy reporting.
 * This header defines the enums, records, and reporting surfaces for
 * projection families, approximation categories, and target-specific
 * projection outcomes.
 *
 * This is a pure data‑contract header; it contains no implementation
 * and no behavior.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * StrataProjectionFamily
 *
 * Identifies the semantic family of a projection operation.
 * ------------------------------------------------------------------------- */
typedef enum StrataProjectionFamily
{
	STRATA_PROJECTION_FAMILY_INVALID = 0,

	/* Initialization‑state values (e.g., UNINIT). */
	STRATA_PROJECTION_FAMILY_INITIALIZATION = 1,

	/* Strength‑distinction values (e.g., WEAK_0, WEAK_1). */
	STRATA_PROJECTION_FAMILY_STRENGTH_DISTINCTION = 2,

	/* Backend‑specific native values with no common equivalent. */
	STRATA_PROJECTION_FAMILY_BACKEND_SPECIFIC = 3

}
StrataProjectionFamily;

/* -------------------------------------------------------------------------
 * StrataApproximationCategory
 *
 * Describes the kind of semantic approximation applied during projection.
 * ------------------------------------------------------------------------- */
typedef enum StrataApproximationCategory
{
	STRATA_APPROX_CATEGORY_INVALID = 0,

	/* Direct projection: a richer state is mapped to a portable state
	   without loss of drive/direction certainty. */
	STRATA_APPROX_CATEGORY_PROJECTION = 1,

	/* Collapse: multiple distinct source states collapse into a single
	   portable state because the target cannot distinguish them. */
	STRATA_APPROX_CATEGORY_COLLAPSE = 2,

	/* Resolution: an unstable or transient source state is resolved to a
	   stable portable state (e.g., UNINIT -> 0/1/X/Z). */
	STRATA_APPROX_CATEGORY_RESOLUTION = 3

}
StrataApproximationCategory;

/* -------------------------------------------------------------------------
 * StrataPortableState
 *
 * Portable four‑state observation set exposed through the common Forge API.
 * This is the stable common denominator across backends.
 * ------------------------------------------------------------------------- */
typedef enum StrataPortableState
{
	STRATA_PORTABLE_STATE_0 = 0,
	STRATA_PORTABLE_STATE_1 = 1,
	STRATA_PORTABLE_STATE_X = 2,
	STRATA_PORTABLE_STATE_Z = 3

}
StrataPortableState;

/* -------------------------------------------------------------------------
 * StrataProjectionOutcome
 *
 * Records a single projection decision for a specific target backend.
 * All fields are machine‑readable; human‑readable explanations belong in
 * separate diagnostic messages.
 * ------------------------------------------------------------------------- */
typedef struct StrataProjectionOutcome
{
	/* Target backend for which this projection was performed. */
	uint32_t target_backend_id;

	/* Original state identifier (backend‑specific encoding). */
	uint32_t original_state_id;

	/* Projected portable state (common observation contract). */
	StrataPortableState projected_portable_state;

	/* Semantic family of the original state. */
	StrataProjectionFamily family;

	/* Kind of approximation applied. */
	StrataApproximationCategory category;

	/* Optional detail flags (reserved for future extension). */
	uint32_t detail_flags;

}
StrataProjectionOutcome;

/* -------------------------------------------------------------------------
 * StrataProjectionReport
 *
 * Aggregates projection outcomes for a compiled artifact.
 * This is a fixed‑size summary; a full log would be stored elsewhere.
 * ------------------------------------------------------------------------- */
typedef struct StrataProjectionReport
{
	/* Number of valid entries in outcomes[]. */
	uint32_t outcome_count;

	/* Up to N distinct projection outcomes. */
	StrataProjectionOutcome outcomes[8];

	/* Summary flags (bitmask). */
	uint32_t summary_flags;

	/* Reserved for future expansion. */
	uint32_t reserved[4];

}
StrataProjectionReport;

/* -------------------------------------------------------------------------
 * Summary flag bits for StrataProjectionReport.
 * ------------------------------------------------------------------------- */
#define STRATA_PROJECTION_SUMMARY_INITIALIZATION_PROJECTED   (1u << 0)
#define STRATA_PROJECTION_SUMMARY_STRENGTH_COLLAPSED         (1u << 1)
#define STRATA_PROJECTION_SUMMARY_BACKEND_SPECIFIC_LOST      (1u << 2)
#define STRATA_PROJECTION_SUMMARY_ANY_SEMANTIC_LOSS          (1u << 3)

#ifdef __cplusplus
}
#endif

#endif /* STRATA_PROJECTION_H */