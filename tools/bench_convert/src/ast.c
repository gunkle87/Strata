#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

BenchAst* ast_create(void) {
    BenchAst* ast = (BenchAst*)malloc(sizeof(BenchAst));
    ast->inputs = NULL;
    ast->num_inputs = 0;
    ast->capacity_inputs = 0;
    
    ast->outputs = NULL;
    ast->num_outputs = 0;
    ast->capacity_outputs = 0;
    
    ast->wires = NULL;
    ast->num_wires = 0;
    ast->capacity_wires = 0;

    ast->gates = NULL;
    ast->num_gates = 0;
    ast->capacity_gates = 0;
    
    ast->next_helper_id = 1;
    return ast;
}

static void free_string_array(char** array, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        free(array[i]);
    }
    free(array);
}

void ast_free(BenchAst* ast) {
    if (!ast) return;
    
    free_string_array(ast->inputs, ast->num_inputs);
    free_string_array(ast->outputs, ast->num_outputs);
    free_string_array(ast->wires, ast->num_wires);
    
    for (size_t i = 0; i < ast->num_gates; ++i) {
        free(ast->gates[i].output_name);
        free_string_array(ast->gates[i].inputs, ast->gates[i].num_inputs);
    }
    free(ast->gates);
    free(ast);
}

char* copy_string(const char* src) {
    if (!src) return NULL;
    size_t len = strlen(src);
    char* dest = (char*)malloc(len + 1);
    strcpy(dest, src);
    return dest;
}

int string_compare(const void* a, const void* b) {
    const char* str_a = *(const char**)a;
    const char* str_b = *(const char**)b;
    return strcmp(str_a, str_b);
}

static bool find_in_array(char** arr, size_t len, const char* name) {
    for (size_t i = 0; i < len; i++) {
        if (strcmp(arr[i], name) == 0) return true;
    }
    return false;
}

bool ast_add_input(BenchAst* ast, const char* name) {
    if (find_in_array(ast->inputs, ast->num_inputs, name)) return true; // tolerate exact duplicate
    if (find_in_array(ast->outputs, ast->num_outputs, name)) return false; // conflict
    if (find_in_array(ast->wires, ast->num_wires, name)) return false; // conflict

    if (ast->num_inputs == ast->capacity_inputs) {
        ast->capacity_inputs = ast->capacity_inputs == 0 ? 16 : ast->capacity_inputs * 2;
        ast->inputs = (char**)realloc(ast->inputs, ast->capacity_inputs * sizeof(char*));
    }
    ast->inputs[ast->num_inputs++] = copy_string(name);
    return true;
}

bool ast_add_output(BenchAst* ast, const char* name) {
    if (find_in_array(ast->outputs, ast->num_outputs, name)) return true; // tolerate exact duplicate
    if (find_in_array(ast->inputs, ast->num_inputs, name)) return false; // conflict
    if (find_in_array(ast->wires, ast->num_wires, name)) return false; // conflict

    if (ast->num_outputs == ast->capacity_outputs) {
        ast->capacity_outputs = ast->capacity_outputs == 0 ? 16 : ast->capacity_outputs * 2;
        ast->outputs = (char**)realloc(ast->outputs, ast->capacity_outputs * sizeof(char*));
    }
    ast->outputs[ast->num_outputs++] = copy_string(name);
    return true;
}

bool ast_add_wire(BenchAst* ast, const char* name) {
    if (find_in_array(ast->wires, ast->num_wires, name)) return true; // tolerate exact duplicate
    if (find_in_array(ast->inputs, ast->num_inputs, name)) return false; // conflict
    if (find_in_array(ast->outputs, ast->num_outputs, name)) return false; // conflict

    if (ast->num_wires == ast->capacity_wires) {
        ast->capacity_wires = ast->capacity_wires == 0 ? 16 : ast->capacity_wires * 2;
        ast->wires = (char**)realloc(ast->wires, ast->capacity_wires * sizeof(char*));
    }
    ast->wires[ast->num_wires++] = copy_string(name);
    return true;
}

void ast_add_gate(BenchAst* ast, const char* output_name, GateOp op, const char** inputs, size_t num_inputs, size_t source_line) {
    if (ast->num_gates == ast->capacity_gates) {
        ast->capacity_gates = ast->capacity_gates == 0 ? 64 : ast->capacity_gates * 2;
        ast->gates = (Gate*)realloc(ast->gates, ast->capacity_gates * sizeof(Gate));
    }
    
    Gate* gate = &ast->gates[ast->num_gates++];
    gate->output_name = copy_string(output_name);
    gate->op = op;
    gate->num_inputs = num_inputs;
    gate->source_line = source_line;
    
    if (num_inputs > 0) {
        gate->inputs = (char**)malloc(num_inputs * sizeof(char*));
        for (size_t i = 0; i < num_inputs; ++i) {
            gate->inputs[i] = copy_string(inputs[i]);
        }
    } else {
        gate->inputs = NULL;
    }
}

char* ast_generate_helper(BenchAst* ast) {
    char buf[32];
    snprintf(buf, sizeof(buf), "__t_%06zu", ast->next_helper_id++);
    return copy_string(buf);
}
