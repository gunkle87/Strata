#ifndef STRATA_BENCH_CONVERT_BLIF_READER_H
#define STRATA_BENCH_CONVERT_BLIF_READER_H

#include "ast.h"
#include <stdbool.h>

// Parse a restricted combinational BLIF file into a BenchAst.
// Returns true on success, false on failure (with error written to stderr).
bool read_blif(const char* filepath, BenchAst* ast);

#endif // STRATA_BENCH_CONVERT_BLIF_READER_H
