#ifndef STRATA_BENCH_CONVERT_AST_H
#define STRATA_BENCH_CONVERT_AST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * BENCH AST
 *
 * Represents generic primitive BENCH logic.
 */

typedef enum {
    OP_INPUT,
    OP_OUTPUT,
    OP_BUF,
    OP_NOT,
    OP_AND,
    OP_NAND,
    OP_OR,
    OP_NOR,
    OP_XOR,
    OP_XNOR,
    OP_ALIAS
} GateOp;

typedef struct {
    char* name;
} Net;

typedef struct {
    char* output_name;
    GateOp op;
    char** inputs;
    size_t num_inputs;
    size_t source_line;
} Gate;

typedef struct {
    char** inputs;
    size_t num_inputs;
    size_t capacity_inputs;

    char** outputs;
    size_t num_outputs;
    size_t capacity_outputs;

    char** wires;
    size_t num_wires;
    size_t capacity_wires;

    Gate* gates;
    size_t num_gates;
    size_t capacity_gates;
    
    // For deterministic helper generation
    size_t next_helper_id;
} BenchAst;

// AST operations
BenchAst* ast_create(void);
void ast_free(BenchAst* ast);

bool ast_add_input(BenchAst* ast, const char* name);
bool ast_add_output(BenchAst* ast, const char* name);
bool ast_add_wire(BenchAst* ast, const char* name);
void ast_add_gate(BenchAst* ast, const char* output_name, GateOp op, const char** inputs, size_t num_inputs, size_t source_line);

// Helper names
char* ast_generate_helper(BenchAst* ast);

// String utils
char* copy_string(const char* src);
int string_compare(const void* a, const void* b);

#endif // STRATA_BENCH_CONVERT_AST_H
