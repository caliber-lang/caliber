#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "caliber--.h"
#include "caliber--parse.h"
#include "x86_64.h"

static char *base_name(const char *filename) {
    char *base = strdup(filename);
    char *dot = strrchr(base, '.');

    if (dot)
        *dot = '\0';

    return base;
}

static void compile_caliber(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror(filename);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *source = malloc(size + 1);
    fread(source, 1, size, f);
    source[size] = '\0';
    fclose(f);

    node_t *ast = parser_parse(source);

    char *base = base_name(filename);
    char path[600];

    snprintf(path, sizeof(path), "%s.s--", base);

    FILE *out = fopen(path, "w");
    if (!out) {
        perror(path);
        exit(1);
    }

    caliber_emit(ast, out);
    fclose(out);

    printf("generated %s\n", path);

    free(base);
    free(source);
}

static void compile_caliber_ir(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror(filename);
        exit(1);
    }

    caliber_program_t *program = caliber_parse(f);
    fclose(f);

    char *base = base_name(filename);
    char path[600];

    snprintf(path, sizeof(path), "%s.s", base);

    FILE *out = fopen(path, "w");
    if (!out) {
        perror(path);
        exit(1);
    }

    x86_64_emit(program, out);
    fclose(out);

    printf("generated %s\n", path);

    caliber_free(program);
    free(base);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: calc <file.cal|file.cal-->\n");
        return 1;
    }

    const char *file = argv[1];
    const char *ext = strrchr(file, '.');

    if (ext && strcmp(ext, ".cal--") == 0)
        compile_caliber_ir(file);
    else
        compile_caliber(file);

    return 0;
}
