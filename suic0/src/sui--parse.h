#ifndef SUI_PARSE_H
#define SUI_PARSE_H

#include <stdio.h>

typedef enum {
    SI_FUNC,
    SI_END,
    SI_INT,
    SI_STRING,
    SI_LOAD,
    SI_STORE,
    SI_BINOP,
    SI_CMP,
    SI_GET,
    SI_SET,
    SI_ALLOC,
    SI_CALL,
    SI_PRINT,
    SI_RETURN,
    SI_LABEL,
    SI_JUMP,
    SI_JUMP_ZERO
} sui_inst_type_t;

typedef struct {
    sui_inst_type_t type;
    char *a;
    char *b;
    char *c;
    long value;
    int n;
} sui_inst_t;

typedef struct {
    sui_inst_t *items;
    int count;
    int cap;
} sui_program_t;

sui_program_t *sui_parse(FILE *f);
void sui_free(sui_program_t *program);

#endif
