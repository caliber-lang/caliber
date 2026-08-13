#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "codegen.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: suic <file.sui>\n");
        return 1;
    }

    const char *filename = argv[1];
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *source = malloc(size + 1);
    fread(source, 1, size, f);
    source[size] = '\0';
    fclose(f);

    node_t *ast = parser_parse(source);

    char base[512];
    strncpy(base, filename, sizeof(base) - 1);
    base[sizeof(base) - 1] = '\0';

    char *dot = strrchr(base, '.');
    if (dot && strcmp(dot, ".sui") == 0) *dot = '\0';

    char asm_path[600];
    char bin_path[600];
    snprintf(asm_path, sizeof(asm_path), "%s.s", base);
    snprintf(bin_path, sizeof(bin_path), "%s", base);

    codegen_generate(ast, asm_path);

    char cmd[1400];
    snprintf(cmd, sizeof(cmd), "cc -o %s %s", bin_path, asm_path);
    int rc = system(cmd);

    if (rc != 0) {
        fprintf(stderr, "assembly/link failed\n");
        return 1;
    }

    printf("compiled to %s\n", bin_path);

    free(source);
    return 0;
}
