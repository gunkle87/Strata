#ifndef STRATA_BENCH_CONVERT_VERILOG_READER_H
#define STRATA_BENCH_CONVERT_VERILOG_READER_H

#include "ast.h"
#include <stdbool.h>

// Parse a structural Verilog file into a BenchAst.
// Returns true on success, false on failure (with error written to stderr).
bool read_verilog(const char* filepath, BenchAst* ast);

#endif // STRATA_BENCH_CONVERT_VERILOG_READER_H
