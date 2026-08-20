#ifndef CALIBER_PARSE_H
#define CALIBER_PARSE_H

#include <stdio.h>

typedef enum {
    CI_FUNC,
    CI_END,
    CI_INT,
    CI_STRING,
    CI_LOAD,
    CI_STORE,
    CI_BINOP,
    CI_CMP,
    CI_GET,
    CI_SET,
    CI_ALLOC,
    CI_CALL,
    CI_PRINT,
    CI_RETURN,
    CI_LABEL,
    CI_JUMP,
    CI_JUMP_ZERO
} caliber_inst_type_t;

typedef struct {
    caliber_inst_type_t type;
    char *a;
    char *b;
    char *c;
    long value;
    int n;
} caliber_inst_t;

typedef struct {
    caliber_inst_t *items;
    int count;
    int cap;
} caliber_program_t;

caliber_program_t *caliber_parse(FILE *f);
void caliber_free(caliber_program_t *program);

#endif
