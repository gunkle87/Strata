/*
 * test_projection_contract.c
 *
 * Minimal test that projection data contracts compile and have expected sizes.
 * This test does not exercise any behavior; it only validates the shape of
 * the new header types.
 */

#include <stdio.h>
#include <stddef.h>
#include "../../include/strata_projection.h"

int main(void)
{
	/* Verify enums have distinct values. */
	if (STRATA_PROJECTION_FAMILY_INITIALIZATION != 1)
	{
		fprintf(stderr, "FAIL: STRATA_PROJECTION_FAMILY_INITIALIZATION != 1\n");
		return 1;
	}

	if (STRATA_PROJECTION_FAMILY_STRENGTH_DISTINCTION != 2)
	{
		fprintf(stderr, "FAIL: STRATA_PROJECTION_FAMILY_STRENGTH_DISTINCTION != 2\n");
		return 1;
	}

	if (STRATA_PROJECTION_FAMILY_BACKEND_SPECIFIC != 3)
	{
		fprintf(stderr, "FAIL: STRATA_PROJECTION_FAMILY_BACKEND_SPECIFIC != 3\n");
		return 1;
	}

	if (STRATA_APPROX_CATEGORY_PROJECTION != 1)
	{
		fprintf(stderr, "FAIL: STRATA_APPROX_CATEGORY_PROJECTION != 1\n");
		return 1;
	}

	if (STRATA_APPROX_CATEGORY_COLLAPSE != 2)
	{
		fprintf(stderr, "FAIL: STRATA_APPROX_CATEGORY_COLLAPSE != 2\n");
		return 1;
	}

	if (STRATA_APPROX_CATEGORY_RESOLUTION != 3)
	{
		fprintf(stderr, "FAIL: STRATA_APPROX_CATEGORY_RESOLUTION != 3\n");
		return 1;
	}

	if (STRATA_PORTABLE_STATE_0 != 0 ||
		STRATA_PORTABLE_STATE_1 != 1 ||
		STRATA_PORTABLE_STATE_X != 2 ||
		STRATA_PORTABLE_STATE_Z != 3)
	{
		fprintf(stderr, "FAIL: portable state enum values mismatch\n");
		return 1;
	}

	/* Verify struct sizes are reasonable (non‑zero). */
	if (sizeof(StrataProjectionOutcome) == 0)
	{
		fprintf(stderr, "FAIL: StrataProjectionOutcome size is zero\n");
		return 1;
	}

	if (sizeof(StrataProjectionReport) == 0)
	{
		fprintf(stderr, "FAIL: StrataProjectionReport size is zero\n");
		return 1;
	}

	/* Verify offset of first field (target_backend_id) is zero. */
	if (offsetof(StrataProjectionOutcome, target_backend_id) != 0)
	{
		fprintf(stderr, "FAIL: target_backend_id offset != 0\n");
		return 1;
	}

	/* Verify that the report can hold up to 8 outcomes. */
	if (sizeof(((StrataProjectionReport*)0)->outcomes) / sizeof(StrataProjectionOutcome) != 8)
	{
		fprintf(stderr, "FAIL: outcomes array size mismatch\n");
		return 1;
	}

	/* Verify summary flags are distinct powers of two. */
	if (STRATA_PROJECTION_SUMMARY_INITIALIZATION_PROJECTED != (1u << 0) ||
		STRATA_PROJECTION_SUMMARY_STRENGTH_COLLAPSED != (1u << 1) ||
		STRATA_PROJECTION_SUMMARY_BACKEND_SPECIFIC_LOST != (1u << 2) ||
		STRATA_PROJECTION_SUMMARY_ANY_SEMANTIC_LOSS != (1u << 3))
	{
		fprintf(stderr, "FAIL: summary flag values mismatch\n");
		return 1;
	}

	printf("PASS: projection data contracts compile correctly\n");
	return 0;
}