#ifndef TYPETAB_H
#define TYPETAB_H

#include "ast.h"

#define MAX_TYPES 64

typedef struct {
    char *field_names[MAX_FIELDS];
    int field_offsets[MAX_FIELDS];
    int field_count;
    int total_size;
} type_info_t;

typedef struct {
    char *name;
    type_info_t info;
} type_entry_t;

void typetab_reset(void);
void typetab_register(node_t *datadef);
type_info_t *typetab_lookup(const char *name);
int typetab_field_offset(const char *type_name, const char *field_name);

#endif
