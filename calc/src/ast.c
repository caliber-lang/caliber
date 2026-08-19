#include "ast.h"
#include <stdlib.h>

node_t *node_new(node_type_t type) {
    node_t *n = calloc(1, sizeof(node_t));
    n->type = type;
    return n;
}

void node_add_child(node_t *parent, node_t *child) {
    if (parent->child_count == parent->child_cap) {
        parent->child_cap = parent->child_cap ? parent->child_cap * 2 : 4;
        parent->children = realloc(parent->children, sizeof(node_t *) * parent->child_cap);
    }
    parent->children[parent->child_count++] = child;
}
