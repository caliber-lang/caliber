#define _POSIX_C_SOURCE 200809L
#include "caliber--parse.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char *dupstr(const char *s) {
    char *r = strdup(s);
    if (!r) {
        fprintf(stderr, "caliber--: out of memory\n");
        exit(1);
    }
    return r;
}

static void add(caliber_program_t *p, caliber_inst_t in) {
    if (p->count == p->cap) {
        p->cap = p->cap ? p->cap * 2 : 64;
        p->items = realloc(p->items, sizeof(caliber_inst_t) * p->cap);
        if (!p->items) {
            fprintf(stderr, "caliber--: out of memory\n");
            exit(1);
        }
    }
    p->items[p->count++] = in;
}

static char *next(char **p) {
    char *s = strtok_r(*p, " \t\r\n", p);
    return s;
}

caliber_program_t *caliber_parse(FILE *f) {
    caliber_program_t *program = calloc(1, sizeof(caliber_program_t));
    char line[4096];

    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        char *op = next(&p);

        if (!op) continue;

        caliber_inst_t in = {0};

        if (strcmp(op, "f") == 0) {
            in.type = CI_FUNC;
            in.a = dupstr(next(&p));
            in.n = atoi(next(&p));
            in.value = atol(next(&p));
        } else if (strcmp(op, "e") == 0) {
            in.type = CI_END;
        } else if (strcmp(op, "i") == 0) {
            in.type = CI_INT;
            in.a = dupstr(next(&p));
            in.b = dupstr(next(&p));
            in.value = atol(next(&p));
        } else if (strcmp(op, "s") == 0) {
            in.type = CI_STRING;
            in.value = atol(next(&p));
        } else if (strcmp(op, "r") == 0) {
            in.type = CI_LOAD;
            in.a = dupstr(next(&p));
        } else if (strcmp(op, "v") == 0) {
            in.type = CI_STORE;
            in.a = dupstr(next(&p));
        } else if (strcmp(op, "b") == 0) {
            in.type = CI_BINOP;
            in.a = dupstr(next(&p));
        } else if (strcmp(op, "c") == 0) {
            in.type = CI_CMP;
            in.a = dupstr(next(&p));
        } else if (strcmp(op, "g") == 0) {
            in.type = CI_GET;
            in.a = dupstr(next(&p));
        } else if (strcmp(op, "m") == 0) {
            in.type = CI_SET;
            in.a = dupstr(next(&p));
        } else if (strcmp(op, "a") == 0) {
            in.type = CI_ALLOC;
            in.a = dupstr(next(&p));
            in.b = dupstr(next(&p));
            in.n = atoi(next(&p));
        } else if (strcmp(op, "x") == 0) {
            in.type = CI_CALL;
            in.a = dupstr(next(&p));
            in.n = atoi(next(&p));
        } else if (strcmp(op, "p") == 0) {
            in.type = CI_PRINT;
        } else if (strcmp(op, "q") == 0) {
            in.type = CI_RETURN;
        } else if (strcmp(op, "l") == 0) {
            in.type = CI_LABEL;
            in.a = dupstr(next(&p));
        } else if (strcmp(op, "j") == 0) {
            in.type = CI_JUMP;
            in.a = dupstr(next(&p));
        } else if (strcmp(op, "jz") == 0) {
            in.type = CI_JUMP_ZERO;
            in.a = dupstr(next(&p));
        } else {
            fprintf(stderr, "caliber--: unknown instruction %s\n", op);
            exit(1);
        }

        add(program, in);
    }

    return program;
}

void caliber_free(caliber_program_t *program) {
    for (int i = 0; i < program->count; i++) {
        free(program->items[i].a);
        free(program->items[i].b);
        free(program->items[i].c);
    }

    free(program->items);
    free(program);
}
