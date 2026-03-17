#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "ast.h"
#include "blif_reader.h"
#include "verilog_reader.h"
#include "bench_writer.h"

typedef enum {
    FORMAT_AUTO,
    FORMAT_BLIF,
    FORMAT_VERILOG
} InputFormat;

typedef enum {
    OVERWRITE_ERROR,
    OVERWRITE_REPLACE
} OverwritePolicy;

typedef struct {
    const char* input_path;
    const char* output_path;
    InputFormat format;
    bool strict;
    OverwritePolicy overwrite;
    bool stdout_mode;
    const char* report_path;
} CliConfig;

static const char* format_to_string(InputFormat format) {
    switch (format) {
        case FORMAT_BLIF: return "blif";
        case FORMAT_VERILOG: return "verilog";
        default: return "auto";
    }
}

static bool write_report(const char* report_path, const CliConfig* config, InputFormat format, const BenchAst* ast) {
    FILE* f = fopen(report_path, "w");
    if (!f) {
        fprintf(stderr, "error: cannot open report file '%s' for writing\n", report_path);
        return false;
    }

    fprintf(f, "status=success\n");
    fprintf(f, "input=%s\n", config->input_path);
    fprintf(f, "output=%s\n", config->output_path);
    fprintf(f, "format=%s\n", format_to_string(format));
    fprintf(f, "strict=%s\n", config->strict ? "true" : "false");
    fprintf(f, "inputs=%zu\n", ast->num_inputs);
    fprintf(f, "outputs=%zu\n", ast->num_outputs);
    fprintf(f, "gates=%zu\n", ast->num_gates);
    fprintf(f, "warnings=0\n");

    fclose(f);
    return true;
}

static void print_usage(const char* prog_name) {
    printf("Usage: %s --input <path> --output <path> [options]\n\n", prog_name);
    printf("Required:\n");
    printf("  --input <path>      Input structural file path\n");
    printf("  --output <path>     Output generic BENCH file path\n\n");
    printf("Options:\n");
    printf("  --from <format>     auto, blif, verilog (default: auto)\n");
    printf("  --strict            Enable strict conversion (default behavior)\n");
    printf("  --overwrite <policy> error, replace (default: error)\n");
    printf("  --stdout            Print output to stdout as well\n");
    printf("  --report <path>     Write conversion report to path\n");
    printf("  --help              Show this help\n");
    printf("  --version           Show version\n");
}

static bool parse_args(int argc, char** argv, CliConfig* config) {
    config->input_path = NULL;
    config->output_path = NULL;
    config->format = FORMAT_AUTO;
    config->strict = true;
    config->overwrite = OVERWRITE_ERROR;
    config->stdout_mode = false;
    config->report_path = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            exit(0);
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("Strata bench_convert tool v1.0\n");
            exit(0);
        } else if (strcmp(argv[i], "--input") == 0 && i + 1 < argc) {
            config->input_path = argv[++i];
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            config->output_path = argv[++i];
        } else if (strcmp(argv[i], "--from") == 0 && i + 1 < argc) {
            i++;
            if (strcmp(argv[i], "auto") == 0) config->format = FORMAT_AUTO;
            else if (strcmp(argv[i], "blif") == 0) config->format = FORMAT_BLIF;
            else if (strcmp(argv[i], "verilog") == 0) config->format = FORMAT_VERILOG;
            else {
                fprintf(stderr, "error: unknown format '%s'\n", argv[i]);
                return false;
            }
        } else if (strcmp(argv[i], "--strict") == 0) {
            config->strict = true;
        } else if (strcmp(argv[i], "--overwrite") == 0 && i + 1 < argc) {
            i++;
            if (strcmp(argv[i], "error") == 0) config->overwrite = OVERWRITE_ERROR;
            else if (strcmp(argv[i], "replace") == 0) config->overwrite = OVERWRITE_REPLACE;
            else {
                fprintf(stderr, "error: unknown overwrite policy '%s'\n", argv[i]);
                return false;
            }
        } else if (strcmp(argv[i], "--stdout") == 0) {
            config->stdout_mode = true;
        } else if (strcmp(argv[i], "--report") == 0 && i + 1 < argc) {
            config->report_path = argv[++i];
        } else {
            fprintf(stderr, "error: unknown or incomplete argument '%s'\n", argv[i]);
            return false;
        }
    }

    if (!config->input_path) {
        fprintf(stderr, "error: --input is required\n");
        return false;
    }
    if (!config->output_path) {
        fprintf(stderr, "error: --output is required\n");
        return false;
    }

    return true;
}

static bool check_overwrite(const char* path, OverwritePolicy policy) {
    if (policy == OVERWRITE_REPLACE) return true;
    FILE* f = fopen(path, "r");
    if (f) {
        fclose(f);
        fprintf(stderr, "error: output file '%s' already exists and overwrite is set to error.\n", path);
        return false;
    }
    return true;
}

static InputFormat infer_format(const char* path) {
    size_t len = strlen(path);
    if (len >= 5 && strcmp(path + len - 5, ".blif") == 0) return FORMAT_BLIF;
    if (len >= 2 && strcmp(path + len - 2, ".v") == 0) return FORMAT_VERILOG;
    return FORMAT_AUTO;
}

int main(int argc, char** argv) {
    CliConfig config;
    if (!parse_args(argc, argv, &config)) {
        return 1;
    }

    // Determine format
    InputFormat fmt = config.format;
    if (fmt == FORMAT_AUTO) {
        fmt = infer_format(config.input_path);
        if (fmt == FORMAT_AUTO) {
            fprintf(stderr, "error: could not infer format for '%s'. Please specify --from <format>.\n", config.input_path);
            return 1;
        }
    }

    // Check overwrite
    if (!check_overwrite(config.output_path, config.overwrite)) {
        return 1;
    }

    BenchAst* ast = ast_create();
    bool success = false;

    if (fmt == FORMAT_BLIF) {
        success = read_blif(config.input_path, ast);
    } else if (fmt == FORMAT_VERILOG) {
        success = read_verilog(config.input_path, ast);
    }

    if (success) {
        success = write_bench(config.output_path, ast);
    }

    if (success && config.stdout_mode) {
        success = write_bench_stream(stdout, ast);
    }

    if (success && config.report_path) {
        success = write_report(config.report_path, &config, fmt, ast);
    }

    ast_free(ast);

    if (!success) {
        // Remove output file if we failed during writing to preserve atomicity and determinism rules
        remove(config.output_path);
        return 1;
    }

    return 0;
}
