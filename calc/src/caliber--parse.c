#define _POSIX_C_SOURCE 200809L
#include "caliber--parse.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char *dupstr(const char *s) {
    char *r = strdup(s);
    if (!r) {
        fprintf(stderr, "sui--: out of memory\n");
        exit(1);
    }
    return r;
}

static void add(sui_program_t *p, sui_inst_t in) {
    if (p->count == p->cap) {
        p->cap = p->cap ? p->cap * 2 : 64;
        p->items = realloc(p->items, sizeof(sui_inst_t) * p->cap);
        if (!p->items) {
            fprintf(stderr, "sui--: out of memory\n");
            exit(1);
        }
    }
    p->items[p->count++] = in;
}

static char *next(char **p) {
    char *s = strtok_r(*p, " \t\r\n", p);
    return s;
}

sui_program_t *sui_parse(FILE *f) {
    sui_program_t *program = calloc(1, sizeof(sui_program_t));
    char line[4096];

    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        char *op = next(&p);

        if (!op) continue;

        sui_inst_t in = {0};

        if (strcmp(op, "f") == 0) {
            in.type = SI_FUNC;
            in.a = dupstr(next(&p));
            in.n = atoi(next(&p));
            in.value = atol(next(&p));
        } else if (strcmp(op, "e") == 0) {
            in.type = SI_END;
        } else if (strcmp(op, "i") == 0) {
            in.type = SI_INT;
            in.a = dupstr(next(&p));
            in.b = dupstr(next(&p));
            in.value = atol(next(&p));
        } else if (strcmp(op, "s") == 0) {
            in.type = SI_STRING;
            in.value = atol(next(&p));
        } else if (strcmp(op, "r") == 0) {
            in.type = SI_LOAD;
            in.a = dupstr(next(&p));
        } else if (strcmp(op, "v") == 0) {
            in.type = SI_STORE;
            in.a = dupstr(next(&p));
        } else if (strcmp(op, "b") == 0) {
            in.type = SI_BINOP;
            in.a = dupstr(next(&p));
        } else if (strcmp(op, "c") == 0) {
            in.type = SI_CMP;
            in.a = dupstr(next(&p));
        } else if (strcmp(op, "g") == 0) {
            in.type = SI_GET;
            in.a = dupstr(next(&p));
        } else if (strcmp(op, "m") == 0) {
            in.type = SI_SET;
            in.a = dupstr(next(&p));
        } else if (strcmp(op, "a") == 0) {
            in.type = SI_ALLOC;
            in.a = dupstr(next(&p));
            in.b = dupstr(next(&p));
            in.n = atoi(next(&p));
        } else if (strcmp(op, "x") == 0) {
            in.type = SI_CALL;
            in.a = dupstr(next(&p));
            in.n = atoi(next(&p));
        } else if (strcmp(op, "p") == 0) {
            in.type = SI_PRINT;
        } else if (strcmp(op, "q") == 0) {
            in.type = SI_RETURN;
        } else if (strcmp(op, "l") == 0) {
            in.type = SI_LABEL;
            in.a = dupstr(next(&p));
        } else if (strcmp(op, "j") == 0) {
            in.type = SI_JUMP;
            in.a = dupstr(next(&p));
        } else if (strcmp(op, "jz") == 0) {
            in.type = SI_JUMP_ZERO;
            in.a = dupstr(next(&p));
        } else {
            fprintf(stderr, "sui--: unknown instruction %s\n", op);
            exit(1);
        }

        add(program, in);
    }

    return program;
}

void sui_free(sui_program_t *program) {
    for (int i = 0; i < program->count; i++) {
        free(program->items[i].a);
        free(program->items[i].b);
        free(program->items[i].c);
    }

    free(program->items);
    free(program);
}
