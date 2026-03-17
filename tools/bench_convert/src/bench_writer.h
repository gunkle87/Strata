#ifndef STRATA_BENCH_CONVERT_BENCH_WRITER_H
#define STRATA_BENCH_CONVERT_BENCH_WRITER_H

#include "ast.h"
#include <stdbool.h>
#include <stdio.h>

// Writes canonical BENCH file from AST.
// Returns true on success, false on failure (with error written to stderr).
bool write_bench(const char* filepath, BenchAst* ast);
bool write_bench_stream(FILE* stream, BenchAst* ast);

#endif // STRATA_BENCH_CONVERT_BENCH_WRITER_H
