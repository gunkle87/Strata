#include "blif_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 4096
#define MAX_TOKENS 1024

// Trim trailing whitespace and newlines
static void trim_trailing(char* str) {
    size_t len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len-1])) {
        str[len-1] = '\0';
        len--;
    }
}

// Check if line is empty or comment
static bool is_empty_or_comment(const char* line) {
    while (*line && isspace((unsigned char)*line)) line++;
    return *line == '\0' || *line == '#';
}

// Split line into tokens
static int tokenize(char* line, char* tokens[]) {
    int count = 0;
    char* p = line;
    while (*p) {
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == '\0') break;
        if (*p == '#') break; // inline comment
        tokens[count++] = p;
        if (count >= MAX_TOKENS) break;
        while (*p && !isspace((unsigned char)*p)) p++;
        if (*p != '\0') {
            *p = '\0';
            p++;
        }
    }
    return count;
}

static bool string_equals(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}

// Handle SOP lowering into AST
static bool process_names(BenchAst* ast, const char* filepath, int line_num, char** inputs, int num_inputs, const char* output, char rows[][MAX_LINE], int num_rows) {
    if (num_inputs == 0) {
        // Constant. E.g. ".names y" \n "1"
        fprintf(stderr, "error[N2B_BLIF_UNSUPPORTED]: %s:%d: unsupported constant construct in phase 1 scope\n", filepath, line_num);
        return false;
    }

    if (num_rows == 0) {
        fprintf(stderr, "error[N2B_BLIF_UNSUPPORTED]: %s:%d: empty truth table\n", filepath, line_num);
        return false;
    }

    // Determine if ON-set or OFF-set (all rows should match the last character)
    char set_type = rows[0][num_inputs + 1]; // format: "row_string result", space is at num_inputs
    for (int i = 0; i < num_rows; i++) {
        if (rows[i][num_inputs] != ' ' && rows[i][num_inputs] != '\t') {
            fprintf(stderr, "error[N2B_BLIF_MALFORMED]: %s:%d: malformed row spacing\n", filepath, line_num);
            return false;
        }
        char val = rows[i][strlen(rows[i]) - 1]; // The last non-space character
        if (val != '0' && val != '1') {
            fprintf(stderr, "error[N2B_BLIF_MALFORMED]: %s:%d: unsupported row output set '%c'\n", filepath, line_num, val);
            return false;
        }
        if (val != set_type) {
            fprintf(stderr, "error[N2B_BLIF_UNSUPPORTED]: %s:%d: mixed ON-set and OFF-set not supported\n", filepath, line_num);
            return false;
        }
    }

    // Exact gate recognition first
    if (num_inputs == 1 && num_rows == 1) {
        if (rows[0][0] == '1' && set_type == '1') {
            ast_add_gate(ast, output, OP_BUF, (const char**)inputs, 1, line_num);
            return true;
        }
        if (rows[0][0] == '0' && set_type == '1') {
            ast_add_gate(ast, output, OP_NOT, (const char**)inputs, 1, line_num);
            return true;
        }
        if (rows[0][0] == '0' && set_type == '0') {
            ast_add_gate(ast, output, OP_BUF, (const char**)inputs, 1, line_num);
            return true;
        }
        if (rows[0][0] == '1' && set_type == '0') {
            ast_add_gate(ast, output, OP_NOT, (const char**)inputs, 1, line_num);
            return true;
        }
    }

    // SOP Lowering
    char** row_outputs = (char**)calloc(num_rows, sizeof(char*));
    for (int i = 0; i < num_rows; i++) {
        int literals = 0;
        for (int j = 0; j < num_inputs; j++) {
            if (rows[i][j] != '-') literals++;
        }

        if (literals == 0) {
            // Constant row! unsupported
            free(row_outputs);
            fprintf(stderr, "error[N2B_BLIF_UNSUPPORTED]: %s:%d: unsupported constant row in SOP\n", filepath, line_num);
            return false;
        }

        if (literals == 1) {
            // single literal, BUF or NOT
            for (int j = 0; j < num_inputs; j++) {
                if (rows[i][j] == '1') {
                    row_outputs[i] = copy_string(inputs[j]);
                } else if (rows[i][j] == '0') {
                    char* t = ast_generate_helper(ast);
                    ast_add_gate(ast, t, OP_NOT, (const char**)&inputs[j], 1, line_num);
                    row_outputs[i] = t;
                }
            }
        } else {
            // AND the literals
            const char** and_ins = (const char**)malloc(literals * sizeof(char*));
            int idx = 0;
            for (int j = 0; j < num_inputs; j++) {
                if (rows[i][j] == '1') {
                    and_ins[idx++] = inputs[j];
                } else if (rows[i][j] == '0') {
                    char* t = ast_generate_helper(ast);
                    ast_add_gate(ast, t, OP_NOT, (const char**)&inputs[j], 1, line_num);
                    and_ins[idx++] = t;
                }
            }
            char* t = ast_generate_helper(ast);
            ast_add_gate(ast, t, OP_AND, and_ins, literals, line_num);
            row_outputs[i] = t;
            free(and_ins);
        }
    }

    if (num_rows == 1) {
        if (set_type == '1') {
            ast_add_gate(ast, output, OP_BUF, (const char**)&row_outputs[0], 1, line_num);
        } else {
            ast_add_gate(ast, output, OP_NOT, (const char**)&row_outputs[0], 1, line_num);
        }
    } else {
        if (set_type == '1') {
            ast_add_gate(ast, output, OP_OR, (const char**)row_outputs, num_rows, line_num);
        } else {
            ast_add_gate(ast, output, OP_NOR, (const char**)row_outputs, num_rows, line_num);
        }
    }

    for (int i = 0; i < num_rows; i++) free(row_outputs[i]);
    free(row_outputs);

    return true;
}

bool read_blif(const char* filepath, BenchAst* ast) {
    FILE* f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "error: cannot open input file '%s'\n", filepath);
        return false;
    }

    char line_buf[MAX_LINE];
    int line_num = 0;
    char* tokens[MAX_TOKENS];

    bool in_names = false;
    char** names_inputs = NULL;
    int names_num_inputs = 0;
    char* names_output = NULL;
    char rows[256][MAX_LINE];
    int num_rows = 0;
    bool has_end = false;
    int model_count = 0;

    while (fgets(line_buf, sizeof(line_buf), f)) {
        line_num++;
        trim_trailing(line_buf);

        if (is_empty_or_comment(line_buf) && !in_names) {
            continue;
        }

        // We copy line_buf because tokenization modifies it
        char line_copy[MAX_LINE];
        strcpy(line_copy, line_buf);
        int tok_count = tokenize(line_copy, tokens);

        if (tok_count == 0) {
            continue;
        }

        if (tokens[0][0] == '.') {
            // End of previous .names processing
            if (in_names) {
                if (!process_names(ast, filepath, line_num, names_inputs, names_num_inputs, names_output, rows, num_rows)) {
                    fclose(f);
                    return false;
                }
                free(names_output);
                for (int i = 0; i < names_num_inputs; i++) free(names_inputs[i]);
                free(names_inputs);
                in_names = false;
            }

            if (string_equals(tokens[0], ".model")) {
                model_count++;
                if (model_count > 1) {
                    fprintf(stderr, "error[N2B_BLIF_UNSUPPORTED]: %s:%d: multiple .model declarations not supported\n", filepath, line_num);
                    fclose(f);
                    return false;
                }
            } else if (string_equals(tokens[0], ".inputs")) {
                for (int i = 1; i < tok_count; i++) {
                    if (!ast_add_input(ast, tokens[i])) {
                        fprintf(stderr, "error[N2B_BLIF_MALFORMED]: %s:%d: conflicting redeclaration of '%s'\n", filepath, line_num, tokens[i]);
                        fclose(f);
                        return false;
                    }
                }
            } else if (string_equals(tokens[0], ".outputs")) {
                for (int i = 1; i < tok_count; i++) {
                    if (!ast_add_output(ast, tokens[i])) {
                        fprintf(stderr, "error[N2B_BLIF_MALFORMED]: %s:%d: conflicting redeclaration of '%s'\n", filepath, line_num, tokens[i]);
                        fclose(f);
                        return false;
                    }
                }
            } else if (string_equals(tokens[0], ".names")) {
                in_names = true;
                names_num_inputs = tok_count - 2;
                if (names_num_inputs < 0) {
                    fprintf(stderr, "error[N2B_BLIF_MALFORMED]: %s:%d: malformed .names declaration\n", filepath, line_num);
                    fclose(f);
                    return false;
                }
                names_output = copy_string(tokens[tok_count - 1]);
                if (names_num_inputs > 0) {
                    names_inputs = (char**)malloc(names_num_inputs * sizeof(char*));
                    for (int i = 0; i < names_num_inputs; i++) {
                        names_inputs[i] = copy_string(tokens[i + 1]);
                    }
                } else {
                    names_inputs = NULL;
                }
                num_rows = 0;
            } else if (string_equals(tokens[0], ".end")) {
                has_end = true;
                break;
            } else {
                fprintf(stderr, "error[N2B_BLIF_UNSUPPORTED]: %s:%d: unsupported BLIF directive '%s'\n", filepath, line_num, tokens[0]);
                fclose(f);
                return false;
            }
        } else {
            // row inside .names
            if (!in_names) {
                fprintf(stderr, "error[N2B_BLIF_MALFORMED]: %s:%d: unexpected truth table row outside of .names\n", filepath, line_num);
                fclose(f);
                return false;
            }
            if (num_rows < 256) {
                // Reconstruct row space
                strcpy(rows[num_rows], tokens[0]);
                for (int i = 1; i < tok_count; i++) {
                    strcat(rows[num_rows], " ");
                    strcat(rows[num_rows], tokens[i]);
                }
                num_rows++;
            } else {
                fprintf(stderr, "error[N2B_BLIF_UNSUPPORTED]: %s:%d: too many rows in .names block\n", filepath, line_num);
                fclose(f);
                return false;
            }
        }
    }

    if (in_names) {
        if (!process_names(ast, filepath, line_num, names_inputs, names_num_inputs, names_output, rows, num_rows)) {
            fclose(f);
            return false;
        }
        free(names_output);
        for (int i = 0; i < names_num_inputs; i++) free(names_inputs[i]);
        free(names_inputs);
    }

    if (!has_end) {
        fprintf(stderr, "error[N2B_BLIF_MALFORMED]: %s: missing .end directive\n", filepath);
        fclose(f);
        return false;
    }
    
    // Check for trailing junk after .end
    while (fgets(line_buf, sizeof(line_buf), f)) {
        line_num++;
        if (!is_empty_or_comment(line_buf)) {
            fprintf(stderr, "error[N2B_BLIF_UNSUPPORTED]: %s:%d: unexpected content after .end (multiple models?)\n", filepath, line_num);
            fclose(f);
            return false;
        }
    }

    fclose(f);
    return true;
}
