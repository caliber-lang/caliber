#include "typetab.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static type_entry_t types[MAX_TYPES];
static int type_count;

void typetab_reset(void) {
    type_count = 0;
}

void typetab_register(node_t *datadef) {
    if (type_count >= MAX_TYPES) {
        fprintf(stderr, "typetab error: too many types\n");
        exit(1);
    }

    type_entry_t *entry = &types[type_count++];
    entry->name = datadef->name;
    entry->info.field_count = datadef->field_count;

    int offset = 8;
    for (int i = 0; i < datadef->field_count; i++) {
        entry->info.field_names[i] = datadef->field_names[i];
        entry->info.field_offsets[i] = offset;
        offset += 8;
    }
    entry->info.total_size = offset;
}

type_info_t *typetab_lookup(const char *name) {
    for (int i = 0; i < type_count; i++) {
        if (strcmp(types[i].name, name) == 0) return &types[i].info;
    }
    return NULL;
}

int typetab_field_offset(const char *type_name, const char *field_name) {
    type_info_t *info = typetab_lookup(type_name);
    if (!info) {
        fprintf(stderr, "typetab error: unknown type %s\n", type_name);
        exit(1);
    }
    for (int i = 0; i < info->field_count; i++) {
        if (strcmp(info->field_names[i], field_name) == 0) {
            return info->field_offsets[i];
        }
    }
    fprintf(stderr, "typetab error: type %s has no field %s\n", type_name, field_name);
    exit(1);
    return -1;
}
