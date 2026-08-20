#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

/* metadata about a stake or reference */
typedef struct {
    char *name;
    char *type_name;
    int is_ref;              /* 1 if reference, 0 if stake */
    char *target_stake;      /* if ref, name of stake it references */
} stake_info_t;

void codegen_generate(node_t *program, const char *out_path);

#endif
