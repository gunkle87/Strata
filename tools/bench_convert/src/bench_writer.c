#include "bench_writer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int compare_gate(const void* a, const void* b) {
    const Gate* ga = (const Gate*)a;
    const Gate* gb = (const Gate*)b;
    return strcmp(ga->output_name, gb->output_name);
}

static const char* op_to_string(GateOp op) {
    switch (op) {
        case OP_INPUT: return "INPUT";
        case OP_OUTPUT: return "OUTPUT";
        case OP_BUF: return "BUF";
        case OP_NOT: return "NOT";
        case OP_AND: return "AND";
        case OP_NAND: return "NAND";
        case OP_OR: return "OR";
        case OP_NOR: return "NOR";
        case OP_XOR: return "XOR";
        case OP_XNOR: return "XNOR";
        case OP_ALIAS: return "BUF"; // Alias materialized as BUF
        default: return "UNKNOWN";
    }
}

bool write_bench_stream(FILE* stream, BenchAst* ast) {
    if (!stream) {
        fprintf(stderr, "error: invalid BENCH output stream\n");
        return false;
    }

    // Print INPUTs in first-seen order
    for (size_t i = 0; i < ast->num_inputs; ++i) {
        fprintf(stream, "INPUT(%s)\n", ast->inputs[i]);
    }
    if (ast->num_inputs > 0) fprintf(stream, "\n");

    // Sort Gates deterministically by output name
    qsort(ast->gates, ast->num_gates, sizeof(Gate), compare_gate);

    for (size_t i = 0; i < ast->num_gates; ++i) {
        Gate* g = &ast->gates[i];
        
        switch (g->op) {
            case OP_BUF:
            case OP_NOT:
            case OP_ALIAS:
                fprintf(stream, "%s = %s(%s)\n", g->output_name, op_to_string(g->op), g->inputs[0]);
                break;
            case OP_AND:
            case OP_NAND:
            case OP_OR:
            case OP_NOR:
            case OP_XOR:
            case OP_XNOR:
                fprintf(stream, "%s = %s(", g->output_name, op_to_string(g->op));
                for (size_t j = 0; j < g->num_inputs; ++j) {
                    fprintf(stream, "%s%s", g->inputs[j], j == g->num_inputs - 1 ? "" : ",");
                }
                fprintf(stream, ")\n");
                break;
            default:
                break;
        }
    }

    // Print OUTPUTs in first-seen order at the end
    for (size_t i = 0; i < ast->num_outputs; ++i) {
        fprintf(stream, "OUTPUT(%s)\n", ast->outputs[i]);
    }

    return ferror(stream) == 0;
}

bool write_bench(const char* filepath, BenchAst* ast) {
    FILE* f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "error: cannot open output file '%s' for writing\n", filepath);
        return false;
    }

    bool ok = write_bench_stream(f, ast);
    fclose(f);
    return ok;
}
