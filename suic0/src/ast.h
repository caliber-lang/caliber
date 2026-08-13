#ifndef AST_H
#define AST_H

typedef enum {
    NODE_PROGRAM,
    NODE_FUNCDEF,
    NODE_VARDECL,
    NODE_ANCHORDECL,
    NODE_MUTATION,
    NODE_PRINT,
    NODE_RETURN,
    NODE_CALL,
    NODE_INT,
    NODE_STRING,
    NODE_IDENT,
    NODE_ANCHORREF,
    NODE_BINOP,
} node_type_t;

typedef struct node node_t;

#define MAX_PARAMS 8

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
    char *param_names[MAX_PARAMS];
    char *param_types[MAX_PARAMS];
    int param_count;
    char *ret_type;
};

node_t *node_new(node_type_t type);
void node_add_child(node_t *parent, node_t *child);

#endif
