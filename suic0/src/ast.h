#ifndef AST_H
#define AST_H

typedef enum {
    NODE_PROGRAM,
    NODE_FUNCDEF,
    NODE_VARDECL,
    NODE_ANCHORDECL,
    NODE_MUTATION,
    NODE_PRINT,
    NODE_INT,
    NODE_STRING,
    NODE_IDENT,
    NODE_ANCHORREF,
    NODE_BINOP,
} node_type_t;

typedef struct node node_t;

struct node {
    node_type_t type;
    char *name;
    long ival;
    char *sval;
    char op;
    node_t *left;
    node_t *right;
    node_t **children;
    int child_count;
    int child_cap;
};

node_t *node_new(node_type_t type);
void node_add_child(node_t *parent, node_t *child);

#endif
