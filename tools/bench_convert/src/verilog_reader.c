#include "verilog_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKEN 256
#define MAX_ARGS 1024

typedef enum {
    TOK_EOF,
    TOK_ID,    // identifiers, keywords, numbers, vectors like a[1]
    TOK_SYM    // ;, ( ) , = ~ [ ] :
} TokenType;

typedef struct {
    char text[MAX_TOKEN];
    TokenType type;
    int line;
} Token;

typedef struct {
    FILE* f;
    int line;
    int ungot_char;
    Token peek_tok;
    bool has_peek;
} Lexer;

static void lexer_init(Lexer* lex, FILE* f) {
    lex->f = f;
    lex->line = 1;
    lex->ungot_char = -1;
    lex->has_peek = false;
}

static int next_char(Lexer* lex) {
    if (lex->ungot_char != -1) {
        int c = lex->ungot_char;
        lex->ungot_char = -1;
        return c;
    }
    int c = fgetc(lex->f);
    if (c == '\n') lex->line++;
    return c;
}

static void unget_char(Lexer* lex, int c) {
    if (c == '\n') lex->line--;
    lex->ungot_char = c;
}

static bool is_id_char(int c) {
    return isalnum(c) || c == '_' || c == '$' || c == '\\' || c == '[' || c == ']'; 
    // we include brackets so a[0] parses as a single token, which simplifies life
}

static bool next_token(Lexer* lex, Token* out) {
    if (lex->has_peek) {
        *out = lex->peek_tok;
        lex->has_peek = false;
        return true;
    }

    int c;
    while (true) {
        c = next_char(lex);
        if (c == EOF) {
            out->type = TOK_EOF;
            out->text[0] = '\0';
            out->line = lex->line;
            return true;
        }

        if (isspace(c)) continue;

        if (c == '/') {
            int c2 = next_char(lex);
            if (c2 == '/') {
                // line comment
                while ((c = next_char(lex)) != EOF && c != '\n');
                continue;
            } else if (c2 == '*') {
                // block comment
                while (true) {
                    c = next_char(lex);
                    if (c == EOF) break;
                    if (c == '*') {
                        int c3 = next_char(lex);
                        if (c3 == '/') break;
                        unget_char(lex, c3);
                    }
                }
                continue;
            } else {
                unget_char(lex, c2);
            }
        }
        break;
    }

    out->line = lex->line;

    if (strchr("(),;=~", c)) {
        out->type = TOK_SYM;
        out->text[0] = (char)c;
        out->text[1] = '\0';
        return true;
    }

    if (is_id_char(c)) {
        int len = 0;
        out->type = TOK_ID;
        out->text[len++] = (char)c;
        while (len < MAX_TOKEN - 1) {
            c = next_char(lex);
            if (c == EOF || (!is_id_char(c) && c != ':')) { // include colon for [1:0]
                if (c != EOF) unget_char(lex, c);
                break;
            }
            out->text[len++] = (char)c;
        }
        out->text[len] = '\0';
        return true;
    }

    fprintf(stderr, "error[N2B_VLOG_MALFORMED]: line %d: unexpected character '%c'\n", lex->line, c);
    return false;
}

static bool require_sym(Lexer* lex, char sym, const char* filepath) {
    Token t;
    if (!next_token(lex, &t)) return false;
    if (t.type != TOK_SYM || t.text[0] != sym) {
        fprintf(stderr, "error[N2B_VLOG_MALFORMED]: %s:%d: expected '%c', got '%s'\n", filepath, t.line, sym, t.text);
        return false;
    }
    return true;
}

static bool expand_vector_decls(BenchAst* ast, const char* range, char** args, int num_args, int decl_type, const char* filepath, int line) {
    // Decl types: 0=input, 1=output, 2=wire
    if (!range) {
        for (int i = 0; i < num_args; i++) {
            bool ok = true;
            if (decl_type == 0) ok = ast_add_input(ast, args[i]);
            else if (decl_type == 1) ok = ast_add_output(ast, args[i]);
            else ok = ast_add_wire(ast, args[i]);
            if (!ok) {
                fprintf(stderr, "error[N2B_VLOG_MALFORMED]: %s:%d: conflicting redeclaration of '%s'\n", filepath, line, args[i]);
                return false;
            }
        }
        return true;
    }

    int high, low;
    if (sscanf(range, "[%d:%d]", &high, &low) != 2) {
        fprintf(stderr, "error[N2B_VLOG_MALFORMED]: %s:%d: unsupported vector range %s\n", filepath, line, range);
        return false;
    }

    int step = high >= low ? -1 : 1;
    for (int i = 0; i < num_args; i++) {
        int curr = high;
        while (true) {
            char buf[MAX_TOKEN];
            snprintf(buf, sizeof(buf), "%s[%d]", args[i], curr);
            
            bool ok = true;
            if (decl_type == 0) ok = ast_add_input(ast, buf);
            else if (decl_type == 1) ok = ast_add_output(ast, buf);
            else ok = ast_add_wire(ast, buf);
            
            if (!ok) {
                fprintf(stderr, "error[N2B_VLOG_MALFORMED]: %s:%d: conflicting redeclaration of '%s'\n", filepath, line, buf);
                return false;
            }

            if (curr == low) break;
            curr += step;
        }
    }
    return true;
}

static bool parse_gate(Lexer* lex, BenchAst* ast, const char* gate_type, const char* filepath) {
    GateOp op;
    if (strcmp(gate_type, "and") == 0) op = OP_AND;
    else if (strcmp(gate_type, "nand") == 0) op = OP_NAND;
    else if (strcmp(gate_type, "or") == 0) op = OP_OR;
    else if (strcmp(gate_type, "nor") == 0) op = OP_NOR;
    else if (strcmp(gate_type, "xor") == 0) op = OP_XOR;
    else if (strcmp(gate_type, "xnor") == 0) op = OP_XNOR;
    else if (strcmp(gate_type, "not") == 0) op = OP_NOT;
    else if (strcmp(gate_type, "buf") == 0) op = OP_BUF;
    else return false;

    Token t;
    if (!next_token(lex, &t)) return false;

    // Optional instance name
    if (t.type == TOK_ID) {
        if (!next_token(lex, &t)) return false;
    }

    if (t.type != TOK_SYM || t.text[0] != '(') {
        fprintf(stderr, "error[N2B_VLOG_MALFORMED]: %s:%d: expected '(' after gate type\n", filepath, t.line);
        return false;
    }

    char* args[MAX_ARGS];
    int num_args = 0;

    while (true) {
        if (!next_token(lex, &t)) return false;
        if (t.type == TOK_ID) {
            args[num_args++] = copy_string(t.text);
        } else {
            fprintf(stderr, "error[N2B_VLOG_MALFORMED]: %s:%d: expected identifier in gate port list\n", filepath, t.line);
            return false;
        }

        if (!next_token(lex, &t)) return false;
        if (t.type == TOK_SYM && t.text[0] == ')') break;
        if (t.type != TOK_SYM || t.text[0] != ',') {
            fprintf(stderr, "error[N2B_VLOG_MALFORMED]: %s:%d: expected ',' or ')' in gate port list\n", filepath, t.line);
            return false;
        }
    }

    if (!require_sym(lex, ';', filepath)) return false;

    if (num_args < 2) {
        fprintf(stderr, "error[N2B_VLOG_MALFORMED]: %s:%d: gate must have at least output and one input\n", filepath, t.line);
        return false;
    }

    const char* output = args[0];
    const char** inputs = (const char**)&args[1];
    int num_inputs = num_args - 1;

    ast_add_gate(ast, output, op, inputs, num_inputs, t.line);

    for (int i = 0; i < num_args; i++) free(args[i]);

    return true;
}

static bool parse_continuous_assign(Lexer* lex, BenchAst* ast, const char* filepath) {
    Token t;
    if (!next_token(lex, &t) || t.type != TOK_ID) return false;
    char out_name[MAX_TOKEN];
    strcpy(out_name, t.text);

    if (!require_sym(lex, '=', filepath)) return false;

    Token in_tok;
    if (!next_token(lex, &in_tok)) return false;
    
    bool inverted = false;
    if (in_tok.type == TOK_SYM && in_tok.text[0] == '~') {
        inverted = true;
        if (!next_token(lex, &in_tok) || in_tok.type != TOK_ID) return false;
    }

    if (in_tok.type != TOK_ID) {
        fprintf(stderr, "error[N2B_VLOG_UNSUPPORTED]: %s:%d: unsupported expression in continuous assignment\n", filepath, in_tok.line);
        return false;
    }

    if (!require_sym(lex, ';', filepath)) return false;

    const char* in_arr[] = { in_tok.text };
    ast_add_gate(ast, out_name, inverted ? OP_NOT : OP_ALIAS, in_arr, 1, in_tok.line);

    return true;
}

bool read_verilog(const char* filepath, BenchAst* ast) {
    FILE* f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "error: cannot open input file '%s'\n", filepath);
        return false;
    }

    Lexer lex;
    lexer_init(&lex, f);

    Token t;
    while (next_token(&lex, &t)) {
        if (t.type == TOK_EOF) break;

        if (strcmp(t.text, "module") == 0) {
            // skip until ';'
            while (next_token(&lex, &t)) {
                if (t.type == TOK_SYM && t.text[0] == ';') break;
            }
        } else if (strcmp(t.text, "endmodule") == 0) {
            // done
        } else if (strcmp(t.text, "input") == 0 || strcmp(t.text, "output") == 0 || strcmp(t.text, "wire") == 0) {
            bool is_input = strcmp(t.text, "input") == 0;
            bool is_output = strcmp(t.text, "output") == 0;
            
            char range[MAX_TOKEN] = "";
            if (!next_token(&lex, &t)) return false;
            if (t.type == TOK_ID && strchr(t.text, ':')) {
                strcpy(range, t.text);
                if (!next_token(&lex, &t)) return false;
            }

            char* args[MAX_ARGS];
            int num_args = 0;
            while (true) {
                if (t.type != TOK_ID) {
                    fprintf(stderr, "error[N2B_VLOG_MALFORMED]: %s:%d: expected identifier in declaration\n", filepath, t.line);
                    return false;
                }
                args[num_args++] = copy_string(t.text);

                if (!next_token(&lex, &t)) return false;
                if (t.type == TOK_SYM && t.text[0] == ';') break;
                if (t.type != TOK_SYM || t.text[0] != ',') {
                    fprintf(stderr, "error[N2B_VLOG_MALFORMED]: %s:%d: expected ',' or ';' in declaration\n", filepath, t.line);
                    return false;
                }
                if (!next_token(&lex, &t)) return false;
            }

                if (!expand_vector_decls(ast, range[0] ? range : NULL, args, num_args, is_input ? 0 : (is_output ? 1 : 2), filepath, t.line)) {
                    for (int i = 0; i < num_args; i++) free(args[i]);
                    return false;
                }
            for (int i = 0; i < num_args; i++) free(args[i]);
        } else if (strcmp(t.text, "assign") == 0) {
            if (!parse_continuous_assign(&lex, ast, filepath)) return false;
        } else if (strcmp(t.text, "always") == 0 || strcmp(t.text, "initial") == 0 || strcmp(t.text, "reg") == 0 || strcmp(t.text, "defparam") == 0) {
            fprintf(stderr, "error[N2B_VLOG_UNSUPPORTED]: %s:%d: unsupported Verilog construct in phase 2 scope: %s\n", filepath, t.line, t.text);
            return false;
        } else {
            // Assumed to be a gate instantiation
            if (!parse_gate(&lex, ast, t.text, filepath)) {
                fprintf(stderr, "error[N2B_VLOG_UNSUPPORTED]: %s:%d: unknown or unsupported construct: '%s'\n", filepath, t.line, t.text);
                return false;
            }
        }
    }

    fclose(f);
    return true;
}
